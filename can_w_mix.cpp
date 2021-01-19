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
//------------------------------------------
// 線程的函数
void *say_hello0(void *args)
{
    int s;
    int nbytes;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;
    char const *ifname = "can0";
    time_t start, end;
    //------------------------------------------
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    /* if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Error while opening socket");
        return -1;
    }*/
    //------------------------------------------
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    printf("%s at index %d\n", ifname, ifr.ifr_ifindex);
    //------------------------------------------
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    /*if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error in socket bind");
        return -2;
    }*/
    //------------------------------------------
    char hex[16];
    int iRet;                        //stored return code
    int iHandle[DEVICE_COUNT] = {0}; //stored handle
    char RetString[64];
    char IPAddress[DEVICE_COUNT][16] = {'\0'};
    DWORD dwTimeOut = 5000;
    char Password[16] = {'\0'};
    //char Password[8] = {0x31, 0x32, 0x33, 0x34, 0, 0, 0, 0};
    BYTE bytStartChannel = 0;
    BYTE bytCount = 8;
    double dValue[16] = {'\0'};
    WORD wValue[16] = {'\0'};
    DWORD dwValue[16] = {'\0'};
    WORD wHiValue[16] = {'\0'};
    WORD wLoValue[16] = {'\0'};
    BYTE bytValue[16] = {'\0'};
    int k;
    int y;

    // Connect Devices
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");

    for (int i = 0; i < DEVICE_COUNT; i++)
    {
        iRet = MXEIO_E1K_Connect(IPAddress[i], //IP address
                                 PORT,         //TCP port number
                                 dwTimeOut,    //timeout
                                 &iHandle[i],  //connection handle
                                 Password);    //ligin password
        BYTE bytRevision[5] = {'\0'};
    }

    //=========================================================================
    WORD wType[DEVICE_COUNT] = {0};
    for (int i = 0; i < DEVICE_COUNT; i++)
    {
        if (0 == iHandle[i])
            continue;
        iRet = MXIO_GetModuleType(iHandle[i], //the handle for a connection
                                  0,          //unused
                                  &wType[i]); //module type
        //=========================================================================
        // AI Channel
        //==========================
        if (wType[i] == 0x1242)
        {
            while (1)
            {
                int high_y = y >> 8;
                int low_y = y & 0xff;
                bytStartChannel = 0;
                bytCount = 1;
                //==========================
                iRet = E1K_AI_Reads(iHandle[i], bytStartChannel, bytCount, dValue);
                for (k = bytStartChannel; k < bytCount; k++)
                {
                    // printf("Get AI0 = %.3f\r\n", k, dValue[k - bytStartChannel]);
                    y = int(dValue[k - bytStartChannel] * 1000);
                    sprintf(hex, "%x", y);
                    printf("y = %d\r\n", y);
                    printf("hex= %s\r\n", hex);
                }
                frame.can_id = 0x500;
                frame.can_dlc = 8;
                frame.data[0] = 0x00;
                frame.data[1] = 0x00;
                frame.data[2] = low_y;
                frame.data[3] = high_y;
                frame.data[4] = 0xFF;
                frame.data[5] = 0xFF;
                frame.data[6] = 0xFF;
                frame.data[7] = 0xFF;

                nbytes = write(s, &frame, sizeof(struct can_frame));
                usleep(100000);
            }
            MXEIO_Disconnect(iHandle[i]);
        }
    }
    //=========================================================================
    /* //Disconnect I/O module
        for (int i = 0; i < DEVICE_COUNT; i++)
        {
            if (0 == iHandle[i])
                continue;
            iRet = MXEIO_Disconnect(iHandle[i]);
        }*/
}
//------------------------------------------


void *say_hello1(void *args)
{
    int iRet;                        //stored return code
    int iHandle[DEVICE_COUNT] = {0}; //stored handle
    char RetString[64];
    char IPAddress[DEVICE_COUNT][16] = {'\0'};
    DWORD dwTimeOut = 5000;
    char Password[16] = {'\0'};
    //char Password[8] = {0x31, 0x32, 0x33, 0x34, 0, 0, 0, 0};
    //==========================
    BYTE bytStartChannel = 0;
    BYTE bytCount = 8;
    double dValue[16] = {'\0'};
    WORD wValue[16] = {'\0'};
    DWORD dwValue[16] = {'\0'};
    DWORD SValue[16] = {'\0'};
    WORD wHiValue[16] = {'\0'};
    WORD wLoValue[16] = {'\0'};
    BYTE bytValue[16] = {'\0'};
    int k, count = 0;
    //=========================================================================
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");
    //=========================================================================
    iRet = MXEIO_Init();
    //==========================
    // Connect Devices
    for (int i = 0; i < DEVICE_COUNT; i++)
    {
        iRet = MXEIO_E1K_Connect(IPAddress[i], //IP address
                                 PORT,         //TCP port number
                                 dwTimeOut,    //timeout
                                 &iHandle[i],  //connection handle
                                 Password);    //ligin password
    }
    //=========================================================================
    WORD wType[DEVICE_COUNT] = {0};
    for (int i = 0; i < DEVICE_COUNT; i++)
    {
        if (0 == iHandle[i])
            continue;

        iRet = MXIO_GetModuleType(iHandle[i], //the handle for a connection
                                  0,          //unused
                                  &wType[i]); //module type
      /*  CheckErr(iRet, (char *)"MXIO_GetModuleType");
        printf("Module Type : E%X ***********************\r\n", wType[i]);*/

        //=========================================================================
        // DI Channel
        //==========================
        if (wType[i] == 0x1242)
        {
            bytStartChannel = 0;
            bytCount = 4;
            //==========================
            iRet = E1K_DI_Reads(iHandle[i], bytStartChannel, bytCount, SValue);
            /*printf("---------------------\r\n");
            printf("DI = %d\r\n", SValue[0]);
            printf("---------------------\r\n");*/

            for (int j = 0; j < 16; j++)
            {

                dwValue[j] = j;
                E1K_DI_Reads(iHandle[i], bytStartChannel, bytCount, SValue);
                if (SValue[0] == 1)
                {
                    count += 1;
                }

                E1K_DO_Writes(iHandle[i], bytStartChannel, bytCount, dwValue[j]);
                //printf("E1K_DO_Writes succeed.\r\n");
               // printf("count = %d\r\n", count);
                //printf("j = %d\r\n", j);
                if (j == 15)
                {
                    j = 0;
                }
                else
                {
                    j = j;
                }

                usleep(500000);
            }
        }
    }
    return 0;
}

void *say_hello2(void *args)
{
    printf("Hello 2！\n");
    return 0;
}
//------------------------------------------

int main()
{
    // 定义线程的 id 变量，多个变量使用数组
    pthread_t tids[NUM_THREADS];

    //参数依次是：创建的线程id，线程参数，调用的函数，传入的函数参数
    pthread_create(&tids[0], NULL, say_hello0, NULL);

    pthread_create(&tids[1], NULL, say_hello1, NULL);

    pthread_create(&tids[2], NULL, say_hello2, NULL);

    //等各个线程退出后，进程才结束，否则进程强制结束了，线程可能还没反应过来；
    pthread_exit(NULL);
}