/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
// ActiveTag.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdlib.h>
#ifdef WINDOWS_TYPE
	#include "mxio.h"
#else
	#include <mxio.h>
	#include <unistd.h>
	#include <string.h>
	#include <time.h>
	#include <sys/time.h>
#endif
#define ActCmdPort		                9500					//Modbus TCP port
#define ActTagPort		                9900					//Active Tag port
#define MAX_CH_VALUE_CHANGE_LIST		16                      //Array size for save channel value that changed of one channel type
#define MAX_MAC_MATCH_LIST		        8                       //Array size for save Source MAC mapping to Destination MAC

//=================================================================
#pragma pack(1)

typedef struct _CH_VALUE_CHANGE_DATA
{
	int iChIndex;
	_ANALOG_VAL pAnalogVal;
}CH_VALUE_CHANGE_DATA, *pCH_VALUE_CHANGE_DATA;

// MAC Match Table
typedef struct _MAC_MATCH_TABLE
{
	BYTE SrcMAC[6];
	BYTE DstMAC[6];
}MAC_MATCH_TABLE, *pMAC_MATCH_TABLE;

// Each Device Channel Type Start Index
typedef struct _CH_TYPE_START_INDEX{
        WORD wHWIDNum;
		//WORD wReserved;
		int StartIndex_AI;
		int StartIndex_DI;
		int StartIndex_DO;
		int StartIndex_RTD;
}CH_TYPE_START_INDEX, *pCH_TYPE_START_INDEX;

#pragma pack()

//=================================================================
void CheckErr( int iRet, char * szFunctionName );		//check function execution result

// Show channels information of each slot
void CheckMsgType( UINT8 nType, char * szMsgTypeName, int& Len );
void CheckModuleType( WORD wHWID, char * szModuleName, int& Len );
void CheckSubMsgType( UINT16 nType, char * szMsgTypeName, int& Len );
void CheckChType( UINT8 wHWID, char * szChTypeName, int& Len );
void CheckIpMacTime( IOLOGIKSTRUCT *data, char * szStr, int& Len );
void ShowChType( IOLOGIKSTRUCT *data, char * szStr, int& Len );
void ShowChEnabled( IOLOGIKSTRUCT *data, char * szStr, int& Len );
void ShowChLocked( IOLOGIKSTRUCT *data, char * szStr, int& Len );
void ShowChValue( IOLOGIKSTRUCT *data, char * szStr, int& Len );

//
bool SaveDev(IOLOGIKSTRUCT data[], int &iIndex);
bool FoundMatchMacDevice(ACTDEV_IO ActDevIo[],
						 IOLOGIKSTRUCT *data,
						 MAC_MATCH_TABLE MacMatchTable[],
						 int iMacMatchCount,
						 int iDstIndexArray[MAX_MAC_MATCH_LIST]);
void ParsingMsg( IOLOGIKSTRUCT data[], IOLOGIKSTRUCT Olddata[], ACTDEV_IO ActDevIo[]);
void GetSpecificIOTypeValueChangeCh(
                        IOLOGIKSTRUCT *data,
                        IOLOGIKSTRUCT *Olddata,
                        int iType,
                        CH_VALUE_CHANGE_DATA ChValueChangeData[],
                        int& iCount,
                        float *fValue); //set to NULL, if iType is not analog type.
void SetAllDeviceConnect(int wDeviceCount, char DeviceInfo[], ACTDEV_IO ActDevIo[]);
int SetDO(IOLOGIKSTRUCT *data, CH_VALUE_CHANGE_DATA ChValueChangeData[], int iCount);
int SetAO(IOLOGIKSTRUCT *data, CH_VALUE_CHANGE_DATA ChValueChangeData[], int iCount);
int GetModuleIOTypeStartIndex( WORD wHWID, int iChType);


char DbgStr[4096] = {'\0'};

//We need to save device info by myself.
const int MAX_SUPPORT_DEVICE = 256;
ACTDEV_IO ActDevIo[MAX_SUPPORT_DEVICE] = {'\0'};
IOLOGIKSTRUCT IoLogikStructure[MAX_SUPPORT_DEVICE] = {0};
IOLOGIKSTRUCT IoLogikOldStructure[MAX_SUPPORT_DEVICE] = {0};

const int IP_INDEX = 0;
const int MAC_INDEX = 4;
WORD wDeviceCount = 0;
BYTE bytCount = 4;
BYTE bytStartChannel = 0;
char Password[8] = {'\0'};


MAC_MATCH_TABLE MacMatchTable[2] =
{
	// When "Src MAC Address" device DI status changed and then set "Dst MAC Address" DO channels status.
    //E1242: Src MAC                     //E1242: Dst MAC
	{0x00, 0x90, 0xE8, 0x20, 0x8D, 0xD4, 0x00, 0x90, 0xE8, 0x20, 0x8D, 0xD4},
	// When "Src MAC Address" device RTD value changed and then set "Dst MAC Address" DO channels status.
	//E1260: Src MAC                     //E1242: Dst MAC
	{0x00, 0x90, 0xE8, 0x1F, 0xA4, 0x70, 0x00, 0x90, 0xE8, 0x20, 0x8D, 0xD4},
};
int iMacMatchTableCount = sizeof(MacMatchTable)/sizeof(MacMatchTable[0]);


//=================================================================
char szTimelog[64] = {'\0'};
char* GetTimeRightNow(void)
{
	UINT16 wYear; 			// tag timestamp
    UINT8 BytMonth;
    UINT8 BytDay;
    UINT8 BytHour;
    UINT8 BytMin;
    UINT8 BytSec;
    UINT16 wMSec;

    #ifdef WINDOWS_TYPE
        SYSTEMTIME time;
    	GetLocalTime(&time);
    	wYear = (UINT16)time.wYear;
        BytMonth = (BYTE)time.wMonth;
        BytDay = (BYTE)time.wDay;
        BytHour = (BYTE)time.wHour;
        BytMin = (BYTE)time.wMinute;
        BytSec = (BYTE)time.wSecond;
        wMSec = (UINT16)time.wMilliseconds;
    #else
    	timeval t1, t2;
    	time_t tt;
    	struct tm *tm_ptr;
    	gettimeofday( &t1, NULL );
		(void)time(&tt);
		tm_ptr = localtime(&tt);
        wYear = (UINT16)(tm_ptr->tm_year+1900);
        BytMonth = (BYTE)tm_ptr->tm_mon+1;//range 0-11
        BytDay = (BYTE)tm_ptr->tm_mday;
        BytHour = (BYTE)tm_ptr->tm_hour;
        BytMin = (BYTE)tm_ptr->tm_min;
        BytSec = (BYTE)tm_ptr->tm_sec;
        wMSec = (UINT16)(t1.tv_usec/1000);
    #endif

    sprintf( szTimelog, "%02d-%02d:%02d:%02d.%03d @ ", BytDay,BytHour,BytMin,BytSec,wMSec);
    return szTimelog;
}
//=================================================================
int GetDevIndex(BYTE szMAC[6])
{
	for(int i=0; i < MAX_SUPPORT_DEVICE; i++)
	{
		if(!memcmp(IoLogikStructure[i].BytSrcMAC, szMAC, sizeof(BYTE)*6))
		{
			return i;
		}
	}
	return -1;
}

//=================================================================
bool SaveDev(IOLOGIKSTRUCT data[], int &iIndex)
{
	bool bFound = false;
	if((-1) != (iIndex = GetDevIndex(data->BytSrcMAC)))
	{
	    //Copy Current to Old first
		memcpy(&IoLogikOldStructure[iIndex], &IoLogikStructure[iIndex], sizeof(IOLOGIKSTRUCT));
		//
		memcpy(&IoLogikStructure[iIndex], data, sizeof(IOLOGIKSTRUCT));
	    bFound = true;
	}

	if(false == bFound)
	{
		for(int i=0; i < MAX_SUPPORT_DEVICE; i++)
		{
			if(0 == IoLogikStructure[i].dwSrcIP)
			{
				memcpy(&IoLogikStructure[i], data, sizeof(IOLOGIKSTRUCT));
				//Copy Current to Old first
		        memcpy(&IoLogikOldStructure[i], &IoLogikStructure[i], sizeof(IOLOGIKSTRUCT));
		        //
				iIndex = i;
				bFound = true;
				break;
			}
		}
	}
	//
	return bFound;
}

//=================================================================
bool FoundMatchMacDevice(ACTDEV_IO ActDevIo[],
						 IOLOGIKSTRUCT *data,
						 MAC_MATCH_TABLE MacMatchTable[],
						 int iMacMatchCount,
						 int iDstIndexArray[MAX_MAC_MATCH_LIST])
{
	bool bRet = false;
	int iDstIndex = 0;
	memset(iDstIndexArray, -1, sizeof(int)*MAX_MAC_MATCH_LIST);
	for(int i=0; i < iMacMatchCount; i++)
	{
		if(!memcmp(data->BytSrcMAC, MacMatchTable[i].SrcMAC, sizeof(char)*6))
		{
			for(int k=0; k < MAX_SUPPORT_DEVICE; k++)
			{
				if(ActDevIo[k].iHandle)
				{
					if(!memcmp(ActDevIo[k].szMAC, MacMatchTable[i].DstMAC, sizeof(char)*6))
					{
						iDstIndexArray[iDstIndex] = k;
						bRet = true;
						iDstIndex++;
						if(iDstIndex >= MAX_MAC_MATCH_LIST)
							return true;
						break;
					}
				}
			}
		}
	}
	return bRet;
}

//=================================================================
void ParsingMsg( IOLOGIKSTRUCT data[], IOLOGIKSTRUCT Olddata[], ACTDEV_IO ActDevIo[])
{
	if(NULL == data)
		return;

	int f = 0, iRet = 0;
	CheckModuleType( data->wHWID, &DbgStr[f], f);
	CheckMsgType( data->BytMsgType, &DbgStr[f], f);
	CheckIpMacTime(data, &DbgStr[f], f);
	printf(DbgStr);

	if(data->BytMsgType == ACTTAG_MSG_SYSTEM)
	{
		f=0;
		CheckSubMsgType( data->wMsgSubType, &DbgStr[f], f);
		printf(DbgStr);
	}
	else if(data->BytMsgType == ACTTAG_MSG_HEARTBEAT)
	{}
	else if(data->BytMsgType == ACTTAG_MSG_CONFIG || (data->BytMsgType == ACTTAG_MSG_IO_STATUS))
	{
		if((data->BytMsgType == ACTTAG_MSG_CONFIG))
		{
			printf( "HeartBeat Timeout Seconds=%d\n", data->wMsgSubType);
            //
			ShowChType(data, &DbgStr[0], f);
			printf(DbgStr);
		    //
			ShowChEnabled(data, &DbgStr[0], f);
			printf(DbgStr);
		    //
            ShowChLocked(data, &DbgStr[0], f);
			printf(DbgStr);
		}
		ShowChValue(data, &DbgStr[0], f);
		printf(DbgStr);
		//
		CH_VALUE_CHANGE_DATA ChValueChangeData[MAX_CH_VALUE_CHANGE_LIST] = {0};
		int iCount = 0;
		GetSpecificIOTypeValueChangeCh(data, Olddata, ACTTAG_DI_DI, ChValueChangeData, iCount, NULL);
		if(iCount) //DI Status Changed and then We have to set DO status the same as DI
		{
			printf( "Set DO status mapping to DI channels.\r\n");
		    iRet = SetDO(data, ChValueChangeData, iCount);
		}
		iCount = 0;
		float fValue = 200.0;
		GetSpecificIOTypeValueChangeCh(data, Olddata, ACTTAG_RTD_PT500_C, ChValueChangeData, iCount, &fValue);
		if(iCount) //DI Status Changed and then We have to set DO status the same as DI
		{
			printf( "Set DO status mapping to RTD channels.\r\n");
		    iRet = SetDO(data, ChValueChangeData, iCount);
		}
	}
	else
	{
		printf( "Undefined Message type: %d\n", data->BytMsgType);
	}
}

//=================================================================
void CALLBACK getActiveTagMessage( IOLOGIKSTRUCT data[], WORD* wMutex )
{
	printf("Recv Active Tag!***********************\n");
	int iIndex = -1;
	if(data != NULL)
	{
		SaveDev(data, iIndex);
		printf("SaveDev to [%d]\r\n", iIndex);
		//Release Mutex
	    *wMutex = 0;
		ParsingMsg(&IoLogikStructure[iIndex], &IoLogikOldStructure[iIndex], ActDevIo);
	}
	else
	{
		printf( "Data = NULL, bypass\n");
		//Release Mutex
	    *wMutex = 0;
	}
}

int main(int argc, char *argv[])
{
	int iRet, i, g, f, k, iCheck, iLoopCount;			//stored return code
	DWORD dwCmdTimeOut=5000;	//ms
	DWORD dwTagTimeOut=5000;	//ms
	WORD wQueueSize = 256;//Range: 1~256
	//==========================
	iRet = MXIO_GetDllVersion();
	printf("MXIO_GetDllVersion DLL Version:%01d.%01d.%01d.%01d\r\n",(iRet>>12)&0xF, (iRet>>8)&0xF,(iRet>>4)&0xF,(iRet)&0xF);
	//==========================
	iRet = MXIO_GetDllBuildDate();
	printf("MXIO_GetDllBuildDate DLL release date:%04X/%02X/%02X\r\n",(iRet>>16)&0xFFFF, (iRet>>8)&0xFF, (iRet)&0xFF);
	//==========================
	iRet = MXEIO_Init();
	CheckErr( iRet, (char*)"MXEIO_Init" );
	if(iRet == MXIO_OK)
		printf("MXEIO_Init Initiate the socket succeed.\n");
	//=========================================================================
	iRet = MXIO_Init_ActiveTag( ActTagPort, ActCmdPort, dwTagTimeOut, dwCmdTimeOut, getActiveTagMessage, wQueueSize);
    CheckErr( iRet, (char*)"MXIO_Init_ActiveTag" );
    if(iRet == MXIO_OK)
    	printf("MXIO_Init_ActiveTag() succeed.\r\n");
	//=========================================================================

	//
	#define     LISTDEV_DELAY_SECS   1000
	int iListDevSec = 0;

	//Send Modbus command to device
	for( iLoopCount=0; iLoopCount < 3600; iLoopCount++) // Secs
	{
		#ifdef WINDOWS_TYPE
			Sleep(1000);
		#else
			usleep(1*1000*1000);
		#endif
		if(iListDevSec < LISTDEV_DELAY_SECS)
		{
		    iListDevSec++;
		    continue;
		}
		iListDevSec = 0;

		MXIO_ListDevs_ActiveTag( &wDeviceCount);
	    printf(" ========= MXIO_ListDevs_ActiveTag() get %d Device. =========\r\n", wDeviceCount );
	    //Device list that connected to server
	    char * DeviceInfo = new char[wDeviceCount*10];
	    //Get All Info (IP and MAC address ) that link to Active MXIO
		iRet =MXIO_GetDevsInfo_ActiveTag( wDeviceCount, DeviceInfo);
		CheckErr( iRet, (char*)"MXIO_GetDevsInfo_ActiveTag" );
		if(iRet == MXIO_OK)
		{
			printf("MXIO_GetDevsInfo_ActiveTag succeed. Device Info:\n");

    		for(int i=0; i < wDeviceCount; i++)
    		{
    			printf(  "Dev[%d]: IP=%d.%d.%d.%d, MAC Address=%02X-%02X-%02X-%02X-%02X-%02X\r\n", i,
    				(unsigned char)DeviceInfo[IP_INDEX+(i*10)], (unsigned char)DeviceInfo[IP_INDEX+1+(i*10)],    //IP
    				(unsigned char)DeviceInfo[IP_INDEX+2+(i*10)], (unsigned char)DeviceInfo[IP_INDEX+3+(i*10)],  //IP
    				(unsigned char)DeviceInfo[MAC_INDEX+(i*10)], (unsigned char)DeviceInfo[MAC_INDEX+1+(i*10)],  //MAC
    				(unsigned char)DeviceInfo[MAC_INDEX+2+(i*10)], (unsigned char)DeviceInfo[MAC_INDEX+3+(i*10)],//MAC
    				(unsigned char)DeviceInfo[MAC_INDEX+4+(i*10)],(unsigned char)DeviceInfo[MAC_INDEX+5+(i*10)]);//MAC
    		}
		}

		//Make all list device connected
    	for(k=0; k < wDeviceCount; k++)
		{
			//Check which MAC has already Connected
			for(f=0; f < MAX_SUPPORT_DEVICE; f++)
			{
				if(ActDevIo[f].iHandle != 0)
				{
					//Check MAC is not exist.
					for(g=0, iCheck=0; g < 6; g++)
                    {
                        if(ActDevIo[f].szMAC[g] != DeviceInfo[MAC_INDEX+(k*10)+g])
                            break;
                        else
                            iCheck++;
                    }//for
                    if(iCheck == 6)
                    	break;
                }
            }//for--(f < MAX_SUPPORT_DEVICE)

            //Check is not found
            if(f == MAX_SUPPORT_DEVICE)
            {
            	//Create Connection to new MAC address
            	for(i=0; i < MAX_SUPPORT_DEVICE; i++)
            	{
            		if(ActDevIo[i].iHandle == 0)
            		{
    					memcpy( ActDevIo[i].szMAC, &DeviceInfo[MAC_INDEX+(k*10)], 6);
    					iRet = MXIO_Connect_ActiveTag( 5000, &ActDevIo[i].iHandle, ActDevIo[i].szMAC, 0, Password);//port and password don't care
    					CheckErr( iRet, (char*)"MXIO_Connect_ActiveTag" );
        				if(iRet == MXIO_OK)
        				{
        					printf("MXIO_Connect_ActiveTag() [%03d]=%02X-%02X-%02X-%02X-%02X-%02X succeed.\r\n",k,
        						(unsigned char)ActDevIo[i].szMAC[0],(unsigned char)ActDevIo[i].szMAC[1],
								(unsigned char)ActDevIo[i].szMAC[2],(unsigned char)ActDevIo[i].szMAC[3],
								(unsigned char)ActDevIo[i].szMAC[4],(unsigned char)ActDevIo[i].szMAC[5]);
							break;
        				}
            		}
            	}//for--(i < MAX_SUPPORT_DEVICE)
            }
        }//for--(k < wDeviceCount)
		//==============================
		//Send Modbus Cmd
		WORD wType = 0;
		for(k=0; k <wDeviceCount; k++)
		{
			//Initial
			wType = 0;
			//==========================
			if(ActDevIo[k].iHandle != 0)
			{
				iRet = MXIO_GetModuleType( ActDevIo[k].iHandle, 0, &wType );
    			CheckErr( iRet, (char*)"MXIO_GetModuleType" );
				if(iRet == MXIO_OK)
				{
					printf( "[%d] Module Type : %X ======================\n", k, wType );
				}
				else
				{
					MXEIO_Disconnect(ActDevIo[k].iHandle);
					ActDevIo[k].iHandle = 0;
				}
			}
			//==========================
			if(ActDevIo[k].iHandle != 0)
			{
				BYTE bytRevision[5] = {'\0'};
            	iRet = MXIO_ReadFirmwareRevision( ActDevIo[k].iHandle, bytRevision);
            	CheckErr( iRet, (char*)"MXIO_ReadFirmwareRevision" );
            	if(iRet == MXIO_OK)
            	{
            		printf("Firmware revision :V%01d.%01d, Release:%01d, build:%01d\r\n",bytRevision[0], bytRevision[1], bytRevision[2], bytRevision[3]);
            	}
            	else
				{
					MXEIO_Disconnect(ActDevIo[k].iHandle);
					ActDevIo[k].iHandle = 0;
				}
            }
            //==========================
            if(ActDevIo[k].iHandle != 0)
            {
            	WORD wGetFirmwareDate[2] = {'\0'};
            	iRet = MXIO_ReadFirmwareDate( ActDevIo[k].iHandle, wGetFirmwareDate);
            	CheckErr( iRet, (char*)"MXIO_ReadFirmwareDate" );
				if(iRet == MXIO_OK)
            	{
            		printf("Firmware Release Date:%04X/%02X/%02X\r\n",wGetFirmwareDate[1], (wGetFirmwareDate[0]>>8)&0xFF, (wGetFirmwareDate[0])&0xFF);
				}
				else
				{
					MXEIO_Disconnect(ActDevIo[k].iHandle);
					ActDevIo[k].iHandle = 0;
				}
			}
			//==========================
			if(ActDevIo[k].iHandle != 0)
			{
			    WORD wValue[16] = {0};
			    DWORD dwValue[16] = {0};
			    double dValue[16] = {0};
				//=========================================================================
    	        // DI Channel
    	        //==========================
    	        if(wType == 0x1242)
				{
	      		    bytStartChannel = 0;
            		bytCount = 4;
            		//==========================
            		iRet = E1K_DI_Reads(ActDevIo[k].iHandle, bytStartChannel, bytCount, dwValue);
                	CheckErr( iRet, (char*)"E1K_DI_Reads" );
                	if(iRet == MXIO_OK)
                	{
                		printf( "E1K_DI_Reads succeed.\r\n");
                		for(int i=bytStartChannel; i < bytCount; i++)
                		{
            				printf( "DI%02d = %s\r\n", i, (dwValue[0]&(1 << (i-bytStartChannel)))?"ON":"OFF");
            			}
                	}
                }
                //=========================================================================
    	        // DO Channel
    	        //==========================
            	if(wType == 0x1242)
				{
				    bytStartChannel = 0;
            		bytCount = 4;
            		//==========================
				    printf( "Set DO status mapping to DI channels.\r\n");
            		iRet = E1K_DO_Writes( ActDevIo[k].iHandle, bytStartChannel, bytCount, dwValue[0]);
            		CheckErr( iRet, (char*)"E1K_DO_Writes" );
            		if(iRet == MXIO_OK)
            		{
            		    for(int i=bytStartChannel; i < bytCount; i++)
            			    printf( "Set DO%02d = %s\r\n", i, ((dwValue[0] & (1 << i)) > 0)?"ON":"OFF");
                	}
            		//==========================
            		printf( "Get DO status (0=OFF or 1=ON):\r\n");
            		iRet = E1K_DO_Reads( ActDevIo[k].iHandle, bytStartChannel, bytCount, &dwValue[0]);
            		CheckErr( iRet, (char*)"E1K_DO_Reads" );
            		if(iRet == MXIO_OK)
            		{
            			printf( "E1K_DO_Reads succeed,\r\n");
            			for(int i=bytStartChannel; i < bytCount; i++)
            				printf( "Get DO%02d = %s\r\n", i, ((dwValue[0] & (1 << i)) > 0)?"ON":"OFF");
                	}
            	}
            	//=========================================================================
    	        // AI Channel
    	        //==========================
    	        if(wType == 0x1242)
				{
				    bytStartChannel = 0;
            		bytCount = 4;
            		//==========================
                	iRet = E1K_AI_Reads(ActDevIo[k].iHandle, bytStartChannel, bytCount, dValue);
                	CheckErr( iRet, (char*)"E1K_AI_Reads" );
                	if(iRet == MXIO_OK)
                	{
                		printf( "E1K_AI_Reads succeed.\r\n");
                		for(int i=bytStartChannel; i < bytCount; i++)
                		{
                			printf( "Get AI%02d = %.3f\r\n", i, dValue[i-bytStartChannel]);
                		}
                	}
                	//==========================
                	iRet = E1K_AI_ReadRaws(ActDevIo[k].iHandle, bytStartChannel, bytCount, wValue);
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
				if(wType == 0x1260)
				{
        		    bytStartChannel = 0;
            		bytCount = 6;
            		//==========================
            		iRet = E1K_RTD_Reads(ActDevIo[k].iHandle, bytStartChannel, bytCount, dValue);
                   	CheckErr( iRet, (char*)"E1K_RTD_Reads" );
                   	if(iRet == MXIO_OK)
                   	{
                   		printf( "E1K_RTD_Reads succeed.\r\n");
                   		for(int i=bytStartChannel; i < bytCount; i++)
                   		{
                   			printf( "Get RTD%02d = %.3f\r\n", i, dValue[i-bytStartChannel]);
                   		}
                   	}
                	//==========================
                	iRet = E1K_RTD_ReadRaws(ActDevIo[k].iHandle, bytStartChannel, bytCount, wValue);
                	CheckErr( iRet, (char*)"E1K_RTD_ReadRaws" );
                	if(iRet == MXIO_OK)
                	{
                		printf( "E1K_RTD_ReadRaws succeed.\r\n");
                		for(int i=bytStartChannel; i < bytCount; i++)
                    	{
                    		printf( "Get RTD%02d = %d\r\n", i, wValue[i-bytStartChannel]);
                    	}
                	}
				}
              	//
    			if(iRet != MXIO_OK)
    			{
					MXEIO_Disconnect(ActDevIo[k].iHandle);
					ActDevIo[k].iHandle = 0;
				}
			}
        	//==========================
		}//for--(k < wDeviceCount)
		//
		delete [] DeviceInfo;
	}
	//=========================================================================
	iRet = MXIO_Close_ActiveTag( );
	CheckErr( iRet, (char*)"MXIO_Close_ActiveTag" );
	if(iRet == MXIO_OK)
		printf("MXIO_Close_ActiveTag() succeed.\r\n");
	getchar();
	//=========================================================================
}
//  After each MXIO function call, the application checks whether the call succeed.
//  If a MXIO function call fails, return an error code.
//  If the call failed, this procedure prints an error message and exits.
void CheckErr( int iRet, char * szFunctionName )
{
	const char * szErrMsg = NULL;

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
		case SLOT_NOT_EXIST:
			szErrMsg = "SLOT_NOT_EXIST";
			break;
		case FIRMWARE_NOT_SUPPORT:
			szErrMsg = "FIRMWARE_NOT_SUPPORT";
			break;
		case CREATE_MUTEX_FAIL:
			szErrMsg = "CREATE_MUTEX_FAIL";
			break;
		case ENUM_NET_INTERFACE_FAIL:
			szErrMsg = "ENUM_NET_INTERFACE_FAIL";
			break;
		case ADD_INFO_TABLE_FAIL:
			szErrMsg = "ADD_INFO_TABLE_FAIL";
			break;
		case UNKNOWN_NET_INTERFACE_FAIL:
			szErrMsg = "UNKNOWN_NET_INTERFACE_FAIL";
			break;
		case TABLE_NET_INTERFACE_FAIL:
			szErrMsg = "TABLE_NET_INTERFACE_FAIL";
			break;
		default:
			szErrMsg = "Unknown Error Value";
			break;
		}
		printf( "Function \"%s\" execution Fail. Error Message : %s\n", szFunctionName, szErrMsg );
	}
}

//=================================================================
void CheckMsgType( UINT8 nType, char * szMsgTypeName, int& Len )
{
	switch(nType)
	{
		case ACTTAG_MSG_POWER_ON:
			Len += sprintf( szMsgTypeName,"nType=%s\n", "MSG_POWER_ON");
			break;
		case ACTTAG_MSG_HEARTBEAT:
			Len += sprintf( szMsgTypeName,"nType=%s\n", "MSG_HEARTBEAT");
			break;
		case ACTTAG_MSG_CONFIG:
			Len += sprintf( szMsgTypeName,"nType=%s\n", "MSG_CONFIG");
			break;
		case ACTTAG_MSG_IO_STATUS:
			Len += sprintf( szMsgTypeName,"nType=%s\n", "MSG_IO_STATUS");
			break;
		case ACTTAG_MSG_SYSTEM:
			Len += sprintf( szMsgTypeName,"nType=%s\n", "MSG_SYSTEM");
			break;
		default:
			Len += sprintf( szMsgTypeName,"nType=%d, Unknown Value\n", nType);
	}
}

//=================================================================
void CheckSubMsgType( UINT16 nType, char * szMsgTypeName, int& Len )
{
	switch(nType)
	{
		case ACTTAG_MSG_SUB_HEARTBEAT_TIMEOUT:
			Len += sprintf( szMsgTypeName,"SubType=%s\n", "ACTTAG_MSG_SUB_HEARTBEAT_TIMEOUT");
			break;
		case ACTTAG_MSG_SUB_READWRITE_TIMEOUT:
			Len += sprintf( szMsgTypeName,"SubType=%s\n", "ACTTAG_MSG_SUB_READWRITE_TIMEOUT");
			break;
		case ACTTAG_MSG_SUB_CLIENT_DISCONNECT:
			Len += sprintf( szMsgTypeName,"SubType=%s\n", "ACTTAG_MSG_SUB_CLIENT_DISCONNECT");
			break;
		case ACTTAG_MSG_SUB_SERVER_DISCONNECT:
			Len += sprintf( szMsgTypeName,"SubType=%s\n", "ACTTAG_MSG_SUB_SERVER_DISCONNECT");
			break;
		default:
			Len += sprintf( szMsgTypeName,"SubType=%d, Unknown Value\n", nType);
	}
}

//=================================================================
void CheckModuleType( WORD wHWID, char * szModuleName, int& Len )
{
	int i=0;
	typedef struct _HWID_NAME{
		WORD wHWIDNum;
		char* szMName;
	}HWID_NAME, *pHWID_NAME;

	HWID_NAME wHWIDArray[] = {
		{ACTTAG_E1210_ID,			(char*)"E1210"},
		{ACTTAG_E1210T_ID,			(char*)"E1210-T"},
		{ACTTAG_E1211_ID,			(char*)"E1211"},
		{ACTTAG_E1211T_ID,			(char*)"E1211-T"},
		{ACTTAG_E1212_ID,			(char*)"E1212"},
		{ACTTAG_E1212T_ID,			(char*)"E1212-T"},
		{ACTTAG_E1214_ID,			(char*)"E1214"},
		{ACTTAG_E1214T_ID,			(char*)"E1214-T"},
		{ACTTAG_E1240_ID,			(char*)"E1240"},
		{ACTTAG_E1240T_ID,			(char*)"E1240-T"},
		{ACTTAG_E1241_ID,			(char*)"E1241"},
		{ACTTAG_E1241T_ID,			(char*)"E1241-T"},
		{ACTTAG_E1242_ID,			(char*)"E1242"},
		{ACTTAG_E1242T_ID,			(char*)"E1242-T"},
		{ACTTAG_E1260_ID,			(char*)"E1260"},
		{ACTTAG_E1260T_ID,			(char*)"E1260-T"},
		{ACTTAG_E1262_ID,			(char*)"E1262"},
		{ACTTAG_E1262T_ID,			(char*)"E1262-T"},
		{ACTTAG_E1261_ID,			(char*)"E1261-WP"},
		{ACTTAG_E1261T_ID,			(char*)"E1261-WP-T"},
		{ACTTAG_E1213_ID,			(char*)"E1213"},
		{ACTTAG_E1213T_ID,			(char*)"E1213-T"},
		//
		{ACTTAG_E2210_ID,			(char*)"E2210"},
		{ACTTAG_E2210_V2_ID,		(char*)"E2210"},
		{ACTTAG_E2210T_ID,			(char*)"E2210-T"},
		{ACTTAG_E2212_ID,			(char*)"E2212"},
		{ACTTAG_E2212T_ID,			(char*)"E2212-T"},
		{ACTTAG_E2214_ID,			(char*)"E2214"},
		{ACTTAG_E2214T_ID,			(char*)"E2214-T"},
		{ACTTAG_E2240_ID,			(char*)"E2240"},
		{ACTTAG_E2240_V2_ID,		(char*)"E2240"},
		{ACTTAG_E2240T_ID,			(char*)"E2240-T"},
		{ACTTAG_E2242_ID,			(char*)"E2242"},
		{ACTTAG_E2242T_ID,			(char*)"E2242-T"},
		{ACTTAG_E2260_ID,			(char*)"E2260"},
		{ACTTAG_E2260T_ID,			(char*)"E2260-T"},
		{ACTTAG_E2262_ID,			(char*)"E2262"},
		{ACTTAG_E2262T_ID,			(char*)"E2262-T"},
		//
		{ACTTAG_W5340_ID,			(char*)"W5340"},
		{ACTTAG_W5312_ID,			(char*)"W5312"},
		{ACTTAG_W5340SLOT_ID,		(char*)"W5340"},
		{ACTTAG_W5340TSLOT_ID,		(char*)"W5340-T"},
		{ACTTAG_W5312SLOT_ID,		(char*)"W5312"},
		{ACTTAG_W5312TSLOT_ID,		(char*)"W5312-T"},
		{ACTTAG_W5341SLOT_ID,		(char*)"W5341"},
		{ACTTAG_W5341TSLOT_ID,		(char*)"W5341-T"},
		{ACTTAG_W5340_HSDPA_ID,		(char*)"W5340-HSDPA"},
		{ACTTAG_W5340T_HSDPA_ID,	(char*)"W5340-HSDPA-T"},
		{ACTTAG_W5312_HSDPA_ID,		(char*)"W5312-HSDPA"},
		{ACTTAG_W5312T_HSDPA_ID,	(char*)"W5312-HSDPA-T"},
		{ACTTAG_W5341_HSDPA_ID,		(char*)"W5341-HSDPA"},
		{ACTTAG_W5341T_HSDPA_ID,	(char*)"W5341-HSDPA-T"},
		{ACTTAG_W5340_HSDPA_JPN_ID,	(char*)"W5340-JPN-HSDPA"},
		{ACTTAG_W5340T_HSDPA_JPN_ID,(char*)"W5340-JPN-HSDPA-T"},
		{ACTTAG_W5340_HSDPA_JPS_ID,	(char*)"W5340-JPS-HSDPA"},
		{ACTTAG_W5340T_HSDPA_JPS_ID,(char*)"W5340-JPS-HSDPA-T"},
		{ACTTAG_W5312_HSDPA_JPN_ID,	(char*)"W5312-JPN-HSDPA"},
		{ACTTAG_W5312T_HSDPA_JPN_ID,(char*)"W5312-JPN-HSDPA-T"},
		{ACTTAG_W5312_HSDPA_JPS_ID,	(char*)"W5312-JPS-HSDPA"},
		{ACTTAG_W5312T_HSDPA_JPS_ID,(char*)"W5312-JPS-HSDPA-T"},
		//
		{ACTTAG_E4200_ID,			(char*)"E4200"},
		//
		{ACTTAG_E1510_ID,			(char*)"E1510"},
		{ACTTAG_E1510_T_ID,			(char*)"E1510-T"},
		{ACTTAG_E1512_ID,			(char*)"E1512"},
		{ACTTAG_E1512_T_ID,			(char*)"E1512-T"},
		{ACTTAG_E1261H_ID,			(char*)"E1261H"},
		{ACTTAG_E1261H_T_ID,		(char*)"E1261H-T"},
		{ACTTAG_E1263H_ID,			(char*)"E1263H"},
		{ACTTAG_E1263H_T_ID,		(char*)"E1263H-T"},
		//
		{ACTTAG_IOPAC_8020_T_ID					,	(char*)"ioPAC-8020-T"},
		//PGM
		{ACTTAG_IOPAC_8020_5_RJ45_C_T_ID 	    ,	(char*)"ioPAC-8020-5-RJ45-C-T"},
		{ACTTAG_IOPAC_8020_5_M12_C_T_ID		    ,	(char*)"ioPAC-8020-5-M12-C-T"},
		{ACTTAG_IOPAC_8020_9_RJ45_C_T_ID 	    ,	(char*)"ioPAC-8020-9-RJ45-C-T"},
		{ACTTAG_IOPAC_8020_9_M12_C_T_ID		    ,	(char*)"ioPAC-8020-9-M12_C_T"},
		{ACTTAG_IOPAC_8020_5_RJ45_IEC_T_ID 	    ,	(char*)"ioPAC-8020-5-RJ45-IEC-T"},
		{ACTTAG_IOPAC_8020_5_M12_IEC_T_ID	    ,	(char*)"ioPAC-8020-5-M12-IEC-T"},
		{ACTTAG_IOPAC_8020_9_RJ45_IEC_T_ID 	    ,	(char*)"ioPAC-8020-9-RJ45-IEC-T"},
		{ACTTAG_IOPAC_8020_9_M12_IEC_T_ID	    ,	(char*)"ioPAC-8020-9-M12-IEC-T"},
		{ACTTAG_IOPAC_8020_5_RJ45_C_ID		    ,	(char*)"ioPAC-8020-5-RJ45-C"},
		{ACTTAG_IOPAC_8020_5_M12_C_ID		    ,	(char*)"ioPAC-8020-5-M12-C"},
		{ACTTAG_IOPAC_8020_9_RJ45_C_ID		    ,	(char*)"ioPAC-8020-9-RJ45-C"},
		{ACTTAG_IOPAC_8020_9_M12_C_ID		    ,	(char*)"ioPAC-8020-9-M12-C"},
		{ACTTAG_IOPAC_8020_5_RJ45_IEC_ID		,	(char*)"ioPAC-8020-5-RJ45-IEC"},
		{ACTTAG_IOPAC_8020_5_M12_IEC_ID		    ,	(char*)"ioPAC-8020-5-M12-IEC"},
		{ACTTAG_IOPAC_8020_9_RJ45_IEC_ID		,	(char*)"ioPAC-8020-9-RJ45-IEC"},
		{ACTTAG_IOPAC_8020_9_M12_IEC_ID		    ,	(char*)"ioPAC-8020-9-M12-IEC"},
		//
		{ACTTAG_W5348_GPRS_C_ID					,   (char*)"W5348-GPRS-C"},
		{ACTTAG_W5348_HSDPA_C_ID				,   (char*)"W5348-HSDPA-C"},
		{ACTTAG_W5348_GPRS_IEC_ID				,   (char*)"W5348-GPRS-IEC"},
		{ACTTAG_W5348_HSDPA_IEC_ID				,   (char*)"W5348-HSDPA-IEC"},
		{ACTTAG_W5348_GPRS_C_T_ID				,   (char*)"W5348-GPRS-C-T"},
		{ACTTAG_W5348_HSDPA_C_T_ID				,   (char*)"W5348-HSDPA-C-T"},
		{ACTTAG_W5348_GPRS_IEC_T_ID				,   (char*)"W5348-GPRS-IEC-T"},
		{ACTTAG_W5348_HSDPA_IEC_T_ID			,	(char*)"W5348-HSDPA-IEC-T"},
	};

	for(i=0; i < (sizeof(wHWIDArray)/sizeof(wHWIDArray[0])); i++)
	{
		if(wHWID == wHWIDArray[i].wHWIDNum)
		{
			Len += sprintf( szModuleName,"%s\n", wHWIDArray[i].szMName);
			break;
		}
	}
	if(i >= (sizeof(wHWIDArray)/sizeof(wHWIDArray[0])))
		Len += sprintf( szModuleName,":Unknown ID(0x%X)\n", wHWID);
}

//=================================================================
void CheckChType( UINT8 wHWID, char * szChTypeName, int& Len )
{
	switch(wHWID)
	{
		case ACTTAG_DI_DI:
			Len += sprintf( szChTypeName,"%s,", "DI_DI                 ");
			break;
		case ACTTAG_DO_DO:
			Len += sprintf( szChTypeName,"%s,", "DO_DO                 ");
			break;
		case ACTTAG_DI_CNT:
			Len += sprintf( szChTypeName,"%s,", "DI_CNT                ");
			break;
		case ACTTAG_DO_PULSE:
			Len += sprintf( szChTypeName,"%s,", "DO_PULSE              ");
			break;
		case ACTTAG_DI_DI_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "DI_DI_DISABLE         ");
			break;
		case ACTTAG_DO_DO_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "DO_DO_DISABLE         ");
			break;
		case ACTTAG_AI_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "AI_DISABLE            ");
			break;
		case ACTTAG_AI_150_150MV:
			Len += sprintf( szChTypeName,"%s,", "AI_150_150MV          ");
			break;
		case ACTTAG_AI_500_500MV:
			Len += sprintf( szChTypeName,"%s,", "AI_500_500MV          ");
			break;
		case ACTTAG_AI_5_5V:
			Len += sprintf( szChTypeName,"%s,", "AI_5_5V               ");
			break;
		case ACTTAG_AI_10_10V:
			Len += sprintf( szChTypeName,"%s,", "AI_10_10V             ");
			break;
		case ACTTAG_AI_0_20MA:
			Len += sprintf( szChTypeName,"%s,", "AI_0_20MA             ");
			break;
		case ACTTAG_AI_4_20MA:
			Len += sprintf( szChTypeName,"%s,", "AI_4_20MA             ");
			break;
		case ACTTAG_AI_0_150MV:
			Len += sprintf( szChTypeName,"%s,", "AI_0_150MV            ");
			break;
		case ACTTAG_AI_0_500MV:
			Len += sprintf( szChTypeName,"%s,", "AI_0_500MV            ");
			break;
		case ACTTAG_AI_0_5V:
			Len += sprintf( szChTypeName,"%s,", "AI_0_5V               ");
			break;
		case ACTTAG_AI_0_10V:
			Len += sprintf( szChTypeName,"%s,", "AI_0_10V              ");
			break;
		case ACTTAG_AO_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "AO_DISABLE            ");
			break;
		case ACTTAG_AO_0_10V:
			Len += sprintf( szChTypeName,"%s,", "AO_0_10V              ");
			break;
		case ACTTAG_AO_4_20MA:
			Len += sprintf( szChTypeName,"%s,", "AO_4_20MA             ");
			break;
		case ACTTAG_AO_0_20MA:
			Len += sprintf( szChTypeName,"%s,", "AO_0_20MA             ");
			break;
		case ACTTAG_AO_10_10V:
			Len += sprintf( szChTypeName,"%s,", "AO_10_10V             ");
			break;
		case ACTTAG_AO_0_5V:
			Len += sprintf( szChTypeName,"%s,", "AO_0_5V               ");
			break;
		case ACTTAG_TC_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "TC_DISABLE            ");
			break;
		case ACTTAG_TC_Type_K_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_K_C           ");
			break;
		case ACTTAG_TC_Type_J_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_J_C           ");
			break;
		case ACTTAG_TC_Type_T_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_T_C           ");
			break;
		case ACTTAG_TC_Type_B_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_B_C           ");
			break;
		case ACTTAG_TC_Type_R_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_R_C           ");
			break;
		case ACTTAG_TC_Type_S_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_S_C           ");
			break;
		case ACTTAG_TC_Type_E_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_E_C           ");
			break;
		case ACTTAG_TC_Type_N_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_N_C           ");
			break;
		case ACTTAG_TC_Type_L_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_L_C           ");
			break;
		case ACTTAG_TC_Type_U_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_U_C           ");
			break;
		case ACTTAG_TC_Type_C_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_C_C           ");
			break;
		case ACTTAG_TC_Type_D_C:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_D_C           ");
			break;
		case ACTTAG_TC_Type_K_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_K_F           ");
			break;
		case ACTTAG_TC_Type_J_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_J_F           ");
			break;
		case ACTTAG_TC_Type_T_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_T_F           ");
			break;
		case ACTTAG_TC_Type_B_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_B_F           ");
			break;
		case ACTTAG_TC_Type_R_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_R_F           ");
			break;
		case ACTTAG_TC_Type_S_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_S_F           ");
			break;
		case ACTTAG_TC_Type_E_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_E_F           ");
			break;
		case ACTTAG_TC_Type_N_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_N_F           ");
			break;
		case ACTTAG_TC_Type_L_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_L_F           ");
			break;
		case ACTTAG_TC_Type_U_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_U_F           ");
			break;
		case ACTTAG_TC_Type_C_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_C_F           ");
			break;
		case ACTTAG_TC_Type_D_F:
			Len += sprintf( szChTypeName,"%s,", "TC_Type_D_F           ");
			break;
		case ACTTAG_TC_VOLTAGE_78_126MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_78_126MV   ");
			break;
		case ACTTAG_TC_VOLTAGE_39_062MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_39_062MV   ");
			break;
		case ACTTAG_TC_VOLTAGE_19_532MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_19_532MV   ");
			break;
		case ACTTAG_TC_VOLTAGE_78MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_78MV       ");
			break;
		case ACTTAG_TC_VOLTAGE_32_7MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_32_7MV     ");
			break;
		case ACTTAG_TC_VOLTAGE_65_5MV:
			Len += sprintf( szChTypeName,"%s,", "TC_VOLTAGE_65_5MV     ");
			break;
		case ACTTAG_RTD_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "RTD_DISABLE           ");
			break;
		case ACTTAG_RTD_PT50_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT50_C            ");
			break;
		case ACTTAG_RTD_PT100_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT100_C           ");
			break;
		case ACTTAG_RTD_PT200_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT200_C           ");
			break;
		case ACTTAG_RTD_PT500_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT500_C           ");
			break;
		case ACTTAG_RTD_PT1000_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT1000_C          ");
			break;
		case ACTTAG_RTD_JPT100_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT100_C          ");
			break;
		case ACTTAG_RTD_JPT200_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT200_C          ");
			break;
		case ACTTAG_RTD_JPT500_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT500_C          ");
			break;
		case ACTTAG_RTD_JPT1000_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT1000_C         ");
			break;
		case ACTTAG_RTD_NI100_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI100_C           ");
			break;
		case ACTTAG_RTD_NI200_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI200_C           ");
			break;
		case ACTTAG_RTD_NI500_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI500_C           ");
			break;
		case ACTTAG_RTD_NI1000_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI1000_C          ");
			break;
		case ACTTAG_RTD_NI120_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI120_C           ");
			break;
		case ACTTAG_RTD_CU10_C:
			Len += sprintf( szChTypeName,"%s,", "RTD_CU10_C            ");
			break;
		case ACTTAG_RTD_PT50_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT50_F            ");
			break;
		case ACTTAG_RTD_PT100_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT100_F           ");
			break;
		case ACTTAG_RTD_PT200_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT200_F           ");
			break;
		case ACTTAG_RTD_PT500_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT500_F           ");
			break;
		case ACTTAG_RTD_PT1000_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_PT1000_F          ");
			break;
		case ACTTAG_RTD_JPT100_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT100_F          ");
			break;
		case ACTTAG_RTD_JPT200_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT200_F          ");
			break;
		case ACTTAG_RTD_JPT500_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT500_F          ");
			break;
		case ACTTAG_RTD_JPT1000_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_JPT1000_F         ");
			break;
		case ACTTAG_RTD_NI100_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI100_F           ");
			break;
		case ACTTAG_RTD_NI200_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI200_F           ");
			break;
		case ACTTAG_RTD_NI500_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI500_F           ");
			break;
		case ACTTAG_RTD_NI1000_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI1000_F          ");
			break;
		case ACTTAG_RTD_NI120_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_NI120_F           ");
			break;
		case ACTTAG_RTD_CU10_F:
			Len += sprintf( szChTypeName,"%s,", "RTD_CU10_F            ");
			break;
		case ACTTAG_RTD_RESISTANCE_1_310_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_310_ ");
			break;
		case ACTTAG_RTD_RESISTANCE_1_620_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_620_ ");
			break;
		case ACTTAG_RTD_RESISTANCE_1_1250_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_1250_");
			break;
		case ACTTAG_RTD_RESISTANCE_1_2200_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_2200_");
			break;
		case ACTTAG_RTD_RESISTANCE_1_2000_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_2000_");
			break;
		case ACTTAG_RTD_RESISTANCE_1_327_:
			Len += sprintf( szChTypeName,"%s,", "RTD_RESISTANCE_1_327_ ");
			break;
		case ACTTAG_VIRTUAL_CH_AVG_C:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_AVG_C      ");
			break;
		case ACTTAG_VIRTUAL_CH_DIF_C:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_DIF_C      ");
			break;
		case ACTTAG_VIRTUAL_CH_AVG_F:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_AVG_F      ");
			break;
		case ACTTAG_VIRTUAL_CH_DIF_F:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_DIF_F      ");
			break;
		case ACTTAG_VIRTUAL_CH_MCONNECT:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_MCONNECT   ");
			break;
		case ACTTAG_VIRTUAL_CH_DISABLE:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_DISABLE    ");
			break;
		case ACTTAG_VIRTUAL_CH_VALUE:
			Len += sprintf( szChTypeName,"%s,", "VIRTUAL_CH_VALUE      ");
			break;
		case ACTTAG_SYSTEM_CONNECTION:
			Len += sprintf( szChTypeName,"%s,", "SYSTEM_CONNECTION     ");
			break;
		default:
			Len += sprintf( szChTypeName,"%s,", "Unknown               ");
			break;
	}
}

//=================================================================
void CheckIpMacTime( IOLOGIKSTRUCT *data, char * szStr, int& Len )
{
	Len += sprintf( &szStr[Len],"IP:%d.%d.%d.%d\n",
		(data->dwSrcIP&0x000000FF),((data->dwSrcIP&0x0000FF00)>>8),
		((data->dwSrcIP&0x00FF0000)>>16),((data->dwSrcIP&0xFF000000)>>24));
	Len += sprintf( &szStr[Len],"MAC:%02X-%02X-%02X-%02X-%02X-%02X\n",
		data->BytSrcMAC[0],data->BytSrcMAC[1],data->BytSrcMAC[2],
		data->BytSrcMAC[3],data->BytSrcMAC[4],data->BytSrcMAC[5]);
	Len += sprintf( &szStr[Len],"Time:%d/%02d/%02d %02d:%02d:%02d.%03d\n",
		data->wYear,data->BytMonth,data->BytDay,
		data->BytHour,data->BytMin,data->BytSec,data->wMSec);
}

//=================================================================
void ShowChType( IOLOGIKSTRUCT *data, char * szStr, int& Len )
{
	Len = sprintf(&szStr[0],"Channel Type:\n    ");
	for(int iSlot=0; iSlot < (data->BytLastSlot+1); iSlot++)
	{
		for(int g=0; g < data->BytLastCh[iSlot]; g++)
		{
			if((g!=0) && ((g%2)==0))
				Len += sprintf( &szStr[Len],"\n    ");
			Len += sprintf( &szStr[Len],"Ch%02d = ", g );
			CheckChType( data->BytChType[iSlot][g], &szStr[Len], Len);
		}
		Len += sprintf( &DbgStr[Len],"\n");
	}
}

//=================================================================
void ShowChEnabled( IOLOGIKSTRUCT *data, char * szStr, int& Len )
{
	Len = sprintf( &szStr[0],"Channel Update Active Tag: 0=> Disable, 1=> Enable\n    ");
	for(int iSlot=0; iSlot < (data->BytLastSlot+1); iSlot++)
	{
		for(int g=0; g < data->BytLastCh[iSlot]; g++)
		{
			if((g!=0) && ((g%4)==0))
				Len += sprintf( &szStr[Len],"\n    ");
			Len += sprintf( &szStr[Len],"Ch%02d = %s,  ", g, (data->BytChNumber[iSlot][g/8]&(1<<(g%8)))?" Enable":"Disable");
		}
		Len += sprintf( &szStr[Len],"\n");
	}
}

//=================================================================
void ShowChLocked( IOLOGIKSTRUCT *data, char * szStr, int& Len )
{
	Len = sprintf( &szStr[0], "Channel Lock Mode(Click&GO used): 0=> Unlocked, 1=> Locked\n    ");
	for(int iSlot=0; iSlot < (data->BytLastSlot+1); iSlot++)
	{
		for(int g=0; g < data->BytLastCh[iSlot]; g++)
		{
			if((g!=0) && ((g%4)==0))
				Len += sprintf( &szStr[Len],"\n    ");
			Len += sprintf( &szStr[Len],"Ch%02d = %s,  ", g, (data->BytChLocked[iSlot][g/8]&(1<<(g%8)))?"Locked  ":"Unlocked");
		}
		Len += sprintf( &szStr[Len],"\n");
	}
}

//=================================================================
void ShowChValue( IOLOGIKSTRUCT *data, char * szStr, int& Len )
{
	Len = sprintf( &DbgStr[0],"Channel Value: 0=> Off, 1=> On\n    ");
	for(int iSlot=0; iSlot < (data->BytLastSlot+1); iSlot++)
	{
		for(int g=0; g < data->BytLastCh[iSlot]; g++)
		{
			if((g!=0) && ((g%4)==0))
				Len += sprintf( &DbgStr[Len],"\n    ");
			if( (data->BytChType[iSlot][g] == ACTTAG_DI_DI_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_DO_DO_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_AI_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_AO_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_TC_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_RTD_DISABLE) ||
				(data->BytChType[iSlot][g] == ACTTAG_VIRTUAL_CH_MCONNECT) ||
				(data->BytChType[iSlot][g] == ACTTAG_VIRTUAL_CH_DISABLE))
				Len += sprintf( &DbgStr[Len],"Ch%02d = None ,\t", g);
			else if( data->BytChType[iSlot][g] < ACTTAG_AI_DISABLE)
				Len += sprintf( &DbgStr[Len],"Ch%02d = %5d,\t", g, data->dwChValue[iSlot][g].iVal);
			else
				Len += sprintf( &DbgStr[Len],"Ch%02d = %.3f,\t", g, data->dwChValue[iSlot][g].fVal);
		}
		Len += sprintf( &DbgStr[Len],"\n");
	}
}

//=================================================================
void GetSpecificIOTypeValueChangeCh(
    IOLOGIKSTRUCT *data,
    IOLOGIKSTRUCT *Olddata,
    int iType,
    CH_VALUE_CHANGE_DATA ChValueChangeData[],
    int& iCount,
    float *fValue)
{
    //Get Channel Type Start Index
	int iCurrentTypeStart = GetModuleIOTypeStartIndex(data->wHWID, iType);
    iCount = 0;
	for(int iSlot=0; iSlot < (data->BytLastSlot+1); iSlot++)
	{
		for(int g=0; g < data->BytLastCh[iSlot]; g++)
		{
		    if(false == (data->BytChLocked[iSlot][g/8]&(1<<(g%8))))
			{
			    if(data->BytChType[iSlot][g] == iType)
			    {
					if(iType < ACTTAG_AI_DISABLE) //Digital Type
					{
					    //Status Changed
    			        if((data->dwChValue[iSlot][g]).iVal != (Olddata->dwChValue[iSlot][g]).iVal)
    			        {
    			            //ChValueChangeData[iCount].pAnalogVal = data->dwChValue[iSlot][g];
    			            if((data->dwChValue[iSlot][g]).iVal >= 1)
    			                ChValueChangeData[iCount].pAnalogVal.iVal = 1;
    			            else
    			                ChValueChangeData[iCount].pAnalogVal.iVal = 0;
    						ChValueChangeData[iCount].iChIndex = g - iCurrentTypeStart;
        			        iCount++;
        			        if(iCount >= MAX_CH_VALUE_CHANGE_LIST)
        			            return;
        			    }
        			}
        			else //analog Type
        		    {
        		        //Save all channels
        		        //ChValueChangeData[iCount].pAnalogVal = data->dwChValue[iSlot][g];
    			        if((data->dwChValue[iSlot][g]).fVal > *fValue)
    			            ChValueChangeData[iCount].pAnalogVal.iVal = 1;
    			        else
    			            ChValueChangeData[iCount].pAnalogVal.iVal = 0;
        			    ChValueChangeData[iCount].iChIndex = g - iCurrentTypeStart;
    			        iCount++;
        			    if(iCount >= MAX_CH_VALUE_CHANGE_LIST)
        			        return;
        			}
			    }
			}
        }
    }
}
//=================================================================
void SetAllDeviceConnect(int wDeviceCount, char DeviceInfo[], ACTDEV_IO ActDevIo[])
{
    for(int k=0; k < wDeviceCount; k++)
    {
		bool bFound = false;
        //Check which MAC has already Connected
        for(int f=0; f < MAX_SUPPORT_DEVICE; f++)
        {
            if(ActDevIo[f].iHandle != 0)
            {
    		    //Check MAC is not exist.
    			if(!memcmp(ActDevIo[f].szMAC, &DeviceInfo[MAC_INDEX+(k*10)], sizeof(char)*6))
    			{
					bFound = true;
    			    break;
    			}
    	    }
        }
        //Check is not found
        if(false == bFound)
        {
        	//Create Connection to new MAC address
        	for(int i=0; i < MAX_SUPPORT_DEVICE; i++)
        	{
        		if(ActDevIo[i].iHandle == 0)
        		{
    				memcpy( ActDevIo[i].szMAC, &DeviceInfo[MAC_INDEX+(k*10)], 6);
    				int iRet = MXIO_Connect_ActiveTag( 5000, &ActDevIo[i].iHandle, ActDevIo[i].szMAC, 0, Password);//port and password don't care
    				CheckErr( iRet, (char*)"MXIO_Connect_ActiveTag" );
    				if(iRet == MXIO_OK)
    				{
    				    break;
    				}
    		    }
    	    }
    	}
    }
}

//=================================================================
int SetDO(IOLOGIKSTRUCT *data, CH_VALUE_CHANGE_DATA ChValueChangeData[], int iCount)
{
	int iRet = MXIO_OK;
	WORD wDeviceCount = 0;
	printf(" ========= SetDO() Enter =========\r\n" );
    MXIO_ListDevs_ActiveTag( &wDeviceCount);
    printf(" ========= MXIO_ListDevs_ActiveTag() get %d Device. =========\r\n", wDeviceCount );

    //Device list that connected to server
    char * DeviceInfo = new char[wDeviceCount*10];
    //Get All Info (IP and MAC address ) that link to Active MXIO
	iRet =MXIO_GetDevsInfo_ActiveTag( wDeviceCount, DeviceInfo);
	CheckErr( iRet, (char*)"MXIO_GetDevsInfo_ActiveTag" );
	if(iRet == MXIO_OK)
	{
	    SetAllDeviceConnect(wDeviceCount, DeviceInfo, ActDevIo);
		//Search specific device to set DO
		int iDstIndexArray[MAX_MAC_MATCH_LIST] = {-1};
		memset(iDstIndexArray, -1, sizeof(iDstIndexArray));
		if(FoundMatchMacDevice(ActDevIo, data, MacMatchTable, iMacMatchTableCount, iDstIndexArray))
		{
			//Send Modbus Cmd
			BYTE bytStartChannel = 0;
			BYTE bytCount = 4;
			//==========================
			for(int k=0;  k < MAX_MAC_MATCH_LIST; k++)
			{
				if((-1) == iDstIndexArray[k])
					break;

				DWORD dwMixValue = 0;
				for(int g=0; g < iCount; g++)
				{
                    dwMixValue |= (ChValueChangeData[g].pAnalogVal.iVal << ChValueChangeData[g].iChIndex);
				}

				iRet = E1K_DO_Writes( ActDevIo[iDstIndexArray[k]].iHandle, bytStartChannel, bytCount, dwMixValue);
				CheckErr( iRet, (char*)"E1K_DO_Writes" );
				if(iRet == MXIO_OK)
				{
					for(int i=bytStartChannel; i < bytCount; i++)
						printf( "Set DO%02d = %s\r\n", i, ((dwMixValue & (1 << (i-bytStartChannel))) > 0)?"ON":"OFF");
				}
				else
				{
                    MXEIO_Disconnect(ActDevIo[iDstIndexArray[k]].iHandle);
					ActDevIo[iDstIndexArray[k]].iHandle = 0;
				}
			}
		}
		else
		{
			printf("Can not found match output device.\r\n");
		}
	}
	delete [] DeviceInfo;
	return iRet;
}
//=================================================================
int SetAO(IOLOGIKSTRUCT *data, CH_VALUE_CHANGE_DATA ChValueChangeData[], int iCount)
{
	int iRet = MXIO_OK;
	WORD wDeviceCount = 0;
    MXIO_ListDevs_ActiveTag( &wDeviceCount);
    printf(" ========= MXIO_ListDevs_ActiveTag() get %d Device. =========\r\n", wDeviceCount );

    //Device list that connected to server
    char * DeviceInfo = new char[wDeviceCount*10];
    //Get All Info (IP and MAC address ) that link to Active MXIO
	iRet =MXIO_GetDevsInfo_ActiveTag( wDeviceCount, DeviceInfo);
	CheckErr( iRet, (char*)"MXIO_GetDevsInfo_ActiveTag" );
	if(iRet == MXIO_OK)
	{
	    SetAllDeviceConnect(wDeviceCount, DeviceInfo, ActDevIo);
		//Search specific device to set DO
		int iDstIndexArray[MAX_MAC_MATCH_LIST] = {-1};
		memset(iDstIndexArray, -1, sizeof(iDstIndexArray));
		if(FoundMatchMacDevice(ActDevIo, data, MacMatchTable, iMacMatchTableCount, iDstIndexArray))
		{
			//Send Modbus Cmd
			BYTE bytStartChannel = 0;
			BYTE bytCount = 4;
			//==========================
			for(int k=0;  k < MAX_MAC_MATCH_LIST; k++)
			{
				if((-1) == iDstIndexArray[k])
					break;

				double dValue = 0;
				for(int g=0; g < iCount; g++)
				{
                   dValue = ChValueChangeData[g].pAnalogVal.fVal;
					iRet = E1K_AO_Writes( ActDevIo[iDstIndexArray[k]].iHandle, (BYTE)ChValueChangeData[g].iChIndex, 1, &dValue);
					CheckErr( iRet, (char*)"E1K_AO_Writes" );
					if(iRet == MXIO_OK)
					{
						printf( "Set AO%02d = %.3f\r\n", ChValueChangeData[g].iChIndex, dValue);
					}
					else
    				{
                        MXEIO_Disconnect(ActDevIo[iDstIndexArray[k]].iHandle);
    					ActDevIo[iDstIndexArray[k]].iHandle = 0;
    					break;
    				}
				}
			}
		}
		else
		{
			printf("Can not found match output device.\r\n");
		}
	}
	delete [] DeviceInfo;
	return iRet;
}
//=================================================================
// return Channel Type Start Index
int GetModuleIOTypeStartIndex( WORD wHWID, int iChType)
{
	int i=0;
	CH_TYPE_START_INDEX wHWIDArray[] =
	{
		{ACTTAG_E1242_ID, 0, 4, 12, -1},
		{ACTTAG_E1242T_ID, 0, 4, 12, -1},
		{ACTTAG_E1260_ID, -1, -1, -1, 0},
		{ACTTAG_E1260T_ID, -1, -1, -1, 0},
	};

	for(i=0; i < (sizeof(wHWIDArray)/sizeof(wHWIDArray[0])); i++)
	{
		if(wHWID == wHWIDArray[i].wHWIDNum)
		{
			if((iChType == ACTTAG_DI_DI) || (iChType == ACTTAG_DI_CNT))
			    return wHWIDArray[i].StartIndex_DI;
			if((iChType == ACTTAG_DO_DO) || (iChType == ACTTAG_DO_PULSE))
				return wHWIDArray[i].StartIndex_DO;
			if((iChType >= ACTTAG_AI_DISABLE) && (iChType <= ACTTAG_AI_0_10V))
				return wHWIDArray[i].StartIndex_AI;
			if((iChType >= ACTTAG_RTD_DISABLE) && (iChType <= ACTTAG_RTD_RESISTANCE_1_327_))
				return wHWIDArray[i].StartIndex_RTD;
			break;
		}
	}
	if(i >= (sizeof(wHWIDArray)/sizeof(wHWIDArray[0])))
	{
	    printf( ":Unknown ID(0x%X)\n", wHWID);
	    return -1;
	}
}
