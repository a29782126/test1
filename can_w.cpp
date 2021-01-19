#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
#include "mxio.h"
#include <sys/time.h>
#include <pthread.h>
//=========================================================================
#define PORT 502 //Modbus TCP port
#define DEVICE_COUNT 2
#define NUM_THREADS 3
//=========================================================================
int s, iRet;
int iHandle[DEVICE_COUNT] = {0}; //stored handle
struct sockaddr_can addr;
struct can_frame frame;
struct ifreq ifr;
char const *ifname = "can0";
char IPAddress[DEVICE_COUNT][16] = {'\0'};
char Password[16] = {'\0'};
BYTE bytStartChannel;
BYTE bytCount;
double dValue[16] = {'\0'};
WORD wValue[16] = {'\0'};
DWORD dwTimeOut = 5000;
DWORD dwValue[16] = {'\0'};
int y; 

//// 線程 1
void *AIread(void *args)
{
    printf("AIread ready！\n");
    char hex[16];
    WORD wType[DEVICE_COUNT] = {0};
    // Connect Devices
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");
    MXEIO_E1K_Connect(IPAddress[0], PORT, dwTimeOut, &iHandle[0], Password); //IP address/TCP port number/timeout/connection handle/ligin password
    MXIO_GetModuleType(iHandle[0], 0, &wType[0]);                            //the handle for a connection/unused/module type
    //==========================
    // AI Channel
    //==========================
    if (wType[0] == 0x1242)
    {
        while (1)
        {

            bytStartChannel = 0;
            bytCount = 1;

            iRet = E1K_AI_Reads(iHandle[0], bytStartChannel, bytCount, dValue);
            for (int k = bytStartChannel; k < bytCount; k++)
            {
                //printf("AI0 = %.3f\r\n", k, dValue[k - bytStartChannel]);
                y = int(dValue[k - bytStartChannel] * 1000);
                sprintf(hex, "%x", y);
                //printf("y = %d\r\n", y);
                //printf("hex= %s\r\n", hex);
            }
        }
        MXEIO_Disconnect(iHandle[0]);
    }
}
//// 線程 2
void *canwrite(void *args)
{
    printf("canwrite ready！\n");

    int nbytes;
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW); //建立 SocketCAN 套接字
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr); //指定 can0 裝置
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr)); //將套接字與 can0 繫結

    while (1)
    {
        int high_y = y >> 8;
        int low_y = y & 0xff;
        frame.can_id = 0x500;
        frame.can_dlc = 8;
        frame.data[0] = 0x00;
        frame.data[1] = 0x00;
        frame.data[2] = low_y;  //low_y;
        frame.data[3] = high_y; //high_y;
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;

        nbytes = write(s, &frame, sizeof(struct can_frame));
        usleep(10000);
    }
}
//// 線程 3
void *say_hello2(void *args)
{
    printf("Hello 2！\n");
    return 0;
}

//// 主線程
int main()
{
    // 定義線程的 id 變量，多個變量使用數組
    pthread_t tids[NUM_THREADS];

    //参数依次是：创建的線程id，線程参数，调用的函数，传入的函数参数
    pthread_create(&tids[0], NULL, AIread, NULL);

    pthread_create(&tids[1], NULL, canwrite, NULL);

    pthread_create(&tids[2], NULL, say_hello2, NULL);

    int iRet;                        //stored return code
    int iHandle[DEVICE_COUNT] = {0}; //stored handle
    char IPAddress[DEVICE_COUNT][16] = {'\0'};
    DWORD dwTimeOut = 5000;
    char Password[16] = {'\0'};
    //==========================
    BYTE bytStartChannel = 0;
    BYTE bytCount = 4;
    double dValue[16] = {'\0'};
    WORD wValue[16] = {'\0'};
    DWORD dwValue[16] = {'\0'};
    DWORD SValue[16] = {'\0'};
    WORD wType[DEVICE_COUNT] = {0};
    int count = 0;
    //=========================================================================
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");
    //=========================================================================
    iRet = MXEIO_Init();
    iRet = MXEIO_E1K_Connect(IPAddress[0], PORT, dwTimeOut, &iHandle[0], Password);
    iRet = MXIO_GetModuleType(iHandle[0], 0, &wType[0]); //module type
    // DI Channel
    if (wType[0] == 0x1242)
    {
        for (int j = 0; j < 16; j++)
        {
            dwValue[j] = j;
            E1K_DO_Writes(iHandle[0], bytStartChannel, bytCount, dwValue[j]);
            if (j == 15)
            {
                j = 0;
            }
            else
            {
                j = j;
            }

            while (1)
            {
                E1K_DI_Reads(iHandle[0], bytStartChannel, bytCount, SValue);
                if (SValue[0] == 1)
                {
                    count += 1;
                    if (count % 2 == 1)
                    {
                        printf("DO state = OFF\r\n");

                        usleep(500000);
                    }
                    else if (count % 2 == 0)
                    {
                        printf("DO state = ON\r\n");
                    }

                    //printf("count = %d\r\n", count);
                }
                else if (SValue[0] == 0)
                {
                    count = count;
                }

                if (count % 2 == 1)
                {
                    usleep(500000);
                }
                else if (count % 2 == 0)
                {
                    break;
                }
            }
            usleep(1000);
        }
    }

    //等各个线程退出后，进程结束
    pthread_exit(NULL);
    return 0;
}