
#include "stdio.h"
#include "stdlib.h"
#include "mxio.h"
#include <sys/time.h>
//=========================================================================
#define PORT 502 //Modbus TCP port
#define DEVICE_COUNT 2
//=========================================================================
int main(void)
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
    WORD wHiValue[16] = {'\0'};
    WORD wLoValue[16] = {'\0'};
    BYTE bytValue[16] = {'\0'};
    int k;
    //=========================================================================
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");
    //==========================
    // Connect Devices
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
            bytStartChannel = 0;
            bytCount = 4;
            //==========================
            iRet = E1K_AI_Reads(iHandle[i], bytStartChannel, bytCount, dValue);
            printf("E1K_AI_Reads succeed.\r\n");
            for (k = bytStartChannel; k < bytCount; k++)
            {
                printf("Get AI%02d = %.3f\r\n", k, dValue[k - bytStartChannel]);
            }
        }
    }
    //=========================================================================
    //Disconnect I/O module
    for (int i = 0; i < DEVICE_COUNT; i++)
    {
        if (0 == iHandle[i])
            continue;
        iRet = MXEIO_Disconnect(iHandle[i]);
    }
}