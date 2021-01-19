/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
// test.cpp : Defines the entry point for the console application.
//
//#include <stdafx.h>
#include "stdio.h"
#include "stdlib.h"
#include "mxio.h"
#include <sys/time.h>

//=========================================================================
#define PORT            502                         //Modbus TCP port
#define DEVICE_COUNT    2

//=========================================================================
void PressAnyKey();
void CheckErr( int iRet, char * szFunctionName );   //check function execution result

//=========================================================================
int main(int argc, char *argv[])
{
    int iRet;                           //stored return code
    int iHandle[DEVICE_COUNT] = {0};    //stored handle
    char RetString[64];
    char IPAddress[DEVICE_COUNT][16] = {'\0'};
    DWORD dwTimeOut=5000;
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
   /* if(argc < 3)
    {
        printf("e12xx inet_Addr1 inet_addr2 [Timeout(ms)] [Password]\n");
        PressAnyKey();
        return 0;
    }
    //==========================
    if(argc > 1)
    {
        sprintf(IPAddress[0], "%s", argv[1]);
    }
    if(argc > 2)
    {
        sprintf(IPAddress[1], "%s", argv[2]);
    }
    if(argc > 3)
    {
        sprintf(RetString, "%s", argv[3]);
        dwTimeOut = atoi(RetString);
    }
    if(argc > 4)
    {
        sprintf(Password, "%s", argv[4]);
    }*/
    sprintf(IPAddress[0], "%s", "192.168.4.254");
    sprintf(IPAddress[1], "%s", "0");
  
    printf( "inet_Addr1=%s\n", IPAddress[0]);
    printf( "inet_Addr2=%s\n", IPAddress[1]);
    printf( "TimeOut=%d\n", dwTimeOut);
    printf( "Password=%s\n", Password);
    //=========================================================================
    iRet = MXIO_GetDllVersion();
    printf( "MXIO_GetDllVersion DLL Version:%01d.%01d.%01d.%01d\r\n",(iRet>>12)&0xF, (iRet>>8)&0xF,(iRet>>4)&0xF,(iRet)&0xF);
    //==========================
    iRet = MXIO_GetDllBuildDate();
    printf( "MXIO_GetDllBuildDate DLL release date:%04X/%02X/%02X\r\n",(iRet>>16)&0xFFFF, (iRet>>8)&0xFF, (iRet)&0xFF);
    //=========================================================================
    iRet = MXEIO_Init();
    CheckErr( iRet, (char*)"MXEIO_Init" );
    if(iRet == MXIO_OK)
    {
        printf( "MXEIO_Init Initiate the socket succeed.\n");
    }

    //==========================
    // Connect Devices
    for(int i=0; i < DEVICE_COUNT; i++)
    {
        iRet = MXEIO_E1K_Connect( IPAddress[i],     //IP address
                              PORT,                 //TCP port number
                              dwTimeOut,            //timeout
                              &iHandle[i],          //connection handle
                              Password);            //ligin password
        CheckErr( iRet, (char*)"MXEIO_E1K_Connect" );
        if(iRet == MXIO_OK)
        {
            printf(  "MXEIO_E1K_Connect Creates Adapter IP=%s connection succeed. TimeOut=%d, Password=%s\n",IPAddress, dwTimeOut, Password);
        }
        else
        {
            printf(  "***** MXEIO_E1K_Connect Creates Adapter IP=%s connection failed. TimeOut=%d, Password=%s\n",IPAddress, dwTimeOut, Password);
            continue;
        }

        BYTE bytRevision[5] = {'\0'};
        iRet = MXIO_ReadFirmwareRevision( iHandle[i], bytRevision);
        CheckErr( iRet, (char*)"MXIO_ReadFirmwareRevision" );
        if(iRet == MXIO_OK)
        {
            printf( "MXIO_ReadFirmwareRevision firmware revision :V%01d.%01d, Release:%01d, build:%01d\r\n",
                bytRevision[0], bytRevision[1], bytRevision[2], bytRevision[3]);
        }
        //==========================
        WORD wGetFirmwareDate[2] = {'\0'};
        iRet = MXIO_ReadFirmwareDate( iHandle[i], wGetFirmwareDate);
        CheckErr( iRet, (char*)"MXIO_ReadFirmwareDate" );
        printf( "MXIO_ReadFirmwareDate firmware Release Date:%04X/%02X/%02X\r\n",
            wGetFirmwareDate[1], (wGetFirmwareDate[0]>>8)&0xFF, (wGetFirmwareDate[0])&0xFF);
    }

    //=========================================================================
    WORD wType[DEVICE_COUNT]={0};
    for(int i=0; i < DEVICE_COUNT; i++)
    {
        if(0 == iHandle[i])
            continue;

        iRet = MXIO_GetModuleType( iHandle[i],  //the handle for a connection
                               0,               //unused
                               &wType[i] );     //module type
        CheckErr( iRet, (char*)"MXIO_GetModuleType" );
        printf("Module Type : E%X ***************************************\r\n", wType[i]);

        //==========================
        if(wType[i] == 0x1242)
        {
            // Get/Clear Safe status
            WORD wSafeStatus;                       // [Safe status] 0 : Normal, 1 : Safe Mode
            //Get Safe status
            iRet = E1K_GetSafeStatus( iHandle[i],   //the handle for a connection
                &wSafeStatus ); //Safe status
            CheckErr( iRet, (char*)"E1K_GetSafeStatus" );
            if(iRet == MXIO_OK)
                printf(  "Get SafeStatus succeeded. Status : %d\r\n", wSafeStatus );

            if( wSafeStatus == 1 )
            {
                //Clear Safe status
                iRet = E1K_ClearSafeStatus( iHandle[i] );
                CheckErr( iRet, (char*)"E1K_ClearSafeStatus" );
                if(iRet == MXIO_OK)
                    printf(  "Clear SafeStatus succeeded.\r\n" );
            }
        }
        //=========================================================================
        // DI Channel
        //==========================
        if(wType[i] == 0x1242)
        {
            bytStartChannel = 0;
            bytCount = 4;
            //==========================
            iRet = E1K_DI_Reads(iHandle[i], bytStartChannel, bytCount, dwValue);
            CheckErr( iRet, (char*)"E1K_DI_Reads" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_DI_Reads succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++ )
                {
                    printf( "DI%02d = %s\r\n", k, (dwValue[0]&(1 << (k-bytStartChannel)))?"ON":"OFF");
                }
            }
        }
        //=========================================================================
        // DO Channel
        //==========================
        if(wType[i] == 0x1242)
        {
            bytStartChannel = 0;
            bytCount = 4;
            //==========================
            printf( "Set DO status mapping to DI channels.\r\n");
            iRet = E1K_DO_Writes( iHandle[i], bytStartChannel, bytCount, dwValue[0]);
            CheckErr( iRet, (char*)"E1K_DO_Writes" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_DO_Writes succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                    printf( "Set DO%02d = %s\r\n", k, ((dwValue[0] & (1 << (k-bytStartChannel))) > 0)?"ON":"OFF");
            }
            //==========================
            printf( "Get DO status (0=OFF or 1=ON):\r\n");
            iRet = E1K_DO_Reads( iHandle[i], bytStartChannel, bytCount, &dwValue[0]);
            CheckErr( iRet, (char*)"E1K_DO_Reads" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_DO_Reads succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                    printf( "Get DO%02d = %s\r\n", k, ((dwValue[0] & (1 << (k-bytStartChannel))) > 0)?"ON":"OFF");
   
            }
        }
        //=========================================================================
        // AI Channel
        //==========================
        if(wType[i] == 0x1242)
        {
            bytStartChannel = 0;
            bytCount = 4;
            //==========================
            iRet = E1K_AI_Reads(iHandle[i], bytStartChannel, bytCount, dValue);
            CheckErr( iRet, (char*)"E1K_AI_Reads" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_AI_Reads succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                {
                    printf( "Get AI%02d = %.3f\r\n", k, dValue[k-bytStartChannel]);
                }
            }
            //==========================
            iRet = E1K_AI_ReadRaws(iHandle[i], bytStartChannel, bytCount, wValue);
            CheckErr( iRet, (char*)"E1K_AI_ReadRaws" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_AI_ReadRaws succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                {
                    printf( "Get AI%02d = %d\r\n", k, wValue[k-bytStartChannel]);
                }
            }
        }
        //=========================================================================
        // RTD Channel
        //==========================
        if(wType[i] == 0x1260)
        {
            bytStartChannel = 0;
            bytCount = 6;
            //==========================
            iRet = E1K_RTD_Reads(iHandle[i], bytStartChannel, bytCount, dValue);
            CheckErr( iRet, (char*)"E1K_RTD_Reads" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_RTD_Reads succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                {
                    printf( "Get RTD%02d = %.3f\r\n", k, dValue[k-bytStartChannel]);
                }
            }
            //==========================
            iRet = E1K_RTD_ReadRaws(iHandle[i], bytStartChannel, bytCount, wValue);
            CheckErr( iRet, (char*)"E1K_RTD_ReadRaws" );
            if(iRet == MXIO_OK)
            {
                printf( "E1K_RTD_ReadRaws succeed.\r\n");
                for(k=bytStartChannel; k < bytCount; k++)
                {
                    printf( "Get RTD%02d = %d\r\n", k, wValue[k-bytStartChannel]);
                }
            }
        }
    }
    //=========================================================================
    //Disconnect I/O module
    for(int i=0; i < DEVICE_COUNT; i++)
    {
        if(0 == iHandle[i])
            continue;

        iRet = MXEIO_Disconnect( iHandle[i] );
        CheckErr( iRet, (char*)"MXEIO_Disconnect" );
        if(iRet == MXIO_OK)
        {
            printf( "\nDisconnect module %d connection succeeded.\r\n", iHandle[i]);
        }
    }
    PressAnyKey();
}

void PressAnyKey()
{
#ifdef _WIN32
    system("pause");
#endif
}
//  After each MXIO function call, the application checks whether the call succeed.
//  If a MXIO function call fails, return an error code.
//  If the call failed, this procedure prints an error message and exits.
void CheckErr( int iRet, char * szFunctionName )
{
    const char * szErrMsg;
    if( iRet != MXIO_OK )
    {
        switch( iRet )
        {
        case ILLEGAL_FUNCTION:
            szErrMsg = "ILLEGAL_FUNCTION";
            break;
        case ILLEGAL_DATA_ADDRESS:
            szErrMsg = "ILLEGAL_DATA_ADDRESS";
            break;
        case ILLEGAL_DATA_VALUE:
            szErrMsg = "ILLEGAL_DATA_VALUE";
            break;
        case SLAVE_DEVICE_FAILURE:
            szErrMsg = "SLAVE_DEVICE_FAILURE";
            break;
        case SLAVE_DEVICE_BUSY:
            szErrMsg = "SLAVE_DEVICE_BUSY";
            break;
        case EIO_TIME_OUT:
            szErrMsg = "EIO_TIME_OUT";
            break;
        case EIO_INIT_SOCKETS_FAIL:
            szErrMsg = "EIO_INIT_SOCKETS_FAIL";
            break;
        case EIO_CREATING_SOCKET_ERROR:
            szErrMsg = "EIO_CREATING_SOCKET_ERROR";
            break;
        case EIO_RESPONSE_BAD:
            szErrMsg = "EIO_RESPONSE_BAD";
            break;
        case EIO_SOCKET_DISCONNECT:
            szErrMsg = "EIO_SOCKET_DISCONNECT";
            break;
        case PROTOCOL_TYPE_ERROR:
            szErrMsg = "PROTOCOL_TYPE_ERROR";
            break;
        case SIO_OPEN_FAIL:
            szErrMsg = "SIO_OPEN_FAIL";
            break;
        case SIO_TIME_OUT:
            szErrMsg = "SIO_TIME_OUT";
            break;
        case SIO_CLOSE_FAIL:
            szErrMsg = "SIO_CLOSE_FAIL";
            break;
        case SIO_PURGE_COMM_FAIL:
            szErrMsg = "SIO_PURGE_COMM_FAIL";
            break;
        case SIO_FLUSH_FILE_BUFFERS_FAIL:
            szErrMsg = "SIO_FLUSH_FILE_BUFFERS_FAIL";
            break;
        case SIO_GET_COMM_STATE_FAIL:
            szErrMsg = "SIO_GET_COMM_STATE_FAIL";
            break;
        case SIO_SET_COMM_STATE_FAIL:
            szErrMsg = "SIO_SET_COMM_STATE_FAIL";
            break;
        case SIO_SETUP_COMM_FAIL:
            szErrMsg = "SIO_SETUP_COMM_FAIL";
            break;
        case SIO_SET_COMM_TIME_OUT_FAIL:
            szErrMsg = "SIO_SET_COMM_TIME_OUT_FAIL";
            break;
        case SIO_CLEAR_COMM_FAIL:
            szErrMsg = "SIO_CLEAR_COMM_FAIL";
            break;
        case SIO_RESPONSE_BAD:
            szErrMsg = "SIO_RESPONSE_BAD";
            break;
        case SIO_TRANSMISSION_MODE_ERROR:
            szErrMsg = "SIO_TRANSMISSION_MODE_ERROR";
            break;
        case PRODUCT_NOT_SUPPORT:
            szErrMsg = "PRODUCT_NOT_SUPPORT";
            break;
        case HANDLE_ERROR:
            szErrMsg = "HANDLE_ERROR";
            break;
        case SLOT_OUT_OF_RANGE:
            szErrMsg = "SLOT_OUT_OF_RANGE";
            break;
        case CHANNEL_OUT_OF_RANGE:
            szErrMsg = "CHANNEL_OUT_OF_RANGE";
            break;
        case COIL_TYPE_ERROR:
            szErrMsg = "COIL_TYPE_ERROR";
            break;
        case REGISTER_TYPE_ERROR:
            szErrMsg = "REGISTER_TYPE_ERROR";
            break;
        case FUNCTION_NOT_SUPPORT:
            szErrMsg = "FUNCTION_NOT_SUPPORT";
            break;
        case OUTPUT_VALUE_OUT_OF_RANGE:
            szErrMsg = "OUTPUT_VALUE_OUT_OF_RANGE";
            break;
        case INPUT_VALUE_OUT_OF_RANGE:
            szErrMsg = "INPUT_VALUE_OUT_OF_RANGE";
            break;
        case EIO_PASSWORD_INCORRECT:
            szErrMsg = "EIO_PASSWORD_INCORRECT";
            break;
        }
        printf(  "Function \"%s\" execution Fail. Error Message : %s\r\n", szFunctionName, szErrMsg );
    }
}
