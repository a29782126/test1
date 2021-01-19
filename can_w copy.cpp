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

//=========================================================================
#define PORT 502 //Modbus TCP port
#define DEVICE_COUNT 2
//=========================================================================
//------------------------------------------

int main(void)
{
    int s;
    int nbytes;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;
    char const *ifname = "can0";
    time_t start,end;  

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Error while opening socket");
        return -1;
    }
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    printf("%s at index %d\n", ifname, ifr.ifr_ifindex);
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error in socket bind");
        return -2;
    }

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
    
   
   
   
   
   
   
   while (1)
    {
        int high_y = y >> 8;
        int low_y = y & 0xff;
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
                bytStartChannel = 0;
                bytCount = 1;
                //==========================
                iRet = E1K_AI_Reads(iHandle[i], bytStartChannel, bytCount, dValue);
                for (k = bytStartChannel; k < bytCount; k++)
                {
                    //printf("Get AI0 = %.3f\r\n", k, dValue[k - bytStartChannel]);
                    y = int(dValue[k - bytStartChannel] * 1000);
                    sprintf(hex, "%x", y);
                    printf("y = %d\r\n", y);
                    printf("hex= %s\r\n", hex);
                }
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
    return 0;
}
