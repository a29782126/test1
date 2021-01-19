/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    DIO.cpp
    DIO program Example code for WinCE................
    2010-06-09	Freddy Liao
		new release
*/

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include "moxa\devices.h"
#include <moxa\\canbus.h>
#include <moxa\\mxcanbus.h>
#include "modbus.h"

#if 1
#define DIO_PORTS	12
#else
#define DIO_PORTS	12	// UC8418
#endif

typedef	struct _CAN_PARA
{
	bool enable, eSend;
	bool setDO[12];
	bool readDI[12];
	unsigned int hPort1, hPort2;
	HANDLE AIO_PORT;
	WORD AI_Value[6];

} CAN_PARA, *PCAN_PARA;

unsigned int canbus_ini(int port)
{
	unsigned int fd;
	fd =mxcan_open(port);
	if ( fd==0 )
	{
		return 0;
	}

	if (mxcan_purge_buffer( fd, 0)!=0)
	{
		return 0;
	}

	if (mxcan_set_bus_timing(fd, 500)!=0)
	{
		return 0;
	}

	if (mxcan_set_write_timeout(fd, 1000)!=0)
	{
		return 0;
	}

	if (mxcan_set_read_timeout(fd, 100)!=0)
	{
		return 0;
	}
	return fd;

}

void AIO_ini(LPVOID param)
{
		
	PCAN_PARA pstPara = (PCAN_PARA) param;
	unsigned int fd; 
	fd = mxsp_open(3);
	mxsp_set_interface( fd, 1);
	mxsp_close(fd);
	DCB dcb;    // setting structure of Serial port
	WCHAR szPort[10] = L"COM3:";          // define COM4: as the port of data received
	COMMTIMEOUTS commtimeouts;
	pstPara->AIO_PORT = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	// Get Port setting structure
	GetCommState(pstPara->AIO_PORT, &dcb);
	GetCommTimeouts(pstPara->AIO_PORT, &commtimeouts);
	dcb.BaudRate = 19200;                     // set baud rate to 384 bps
	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;   // set Hardware flow control
	dcb.ByteSize = 8;                          // data size = 8
	dcb.StopBits = ONESTOPBIT;                 // stop bit = 1
	dcb.Parity = NOPARITY;                     // no parity
	// Update Port setting structure
	SetCommState(pstPara->AIO_PORT, &dcb);
	commtimeouts.ReadIntervalTimeout = 1;
	commtimeouts.ReadTotalTimeoutConstant = 10;
	commtimeouts.ReadTotalTimeoutMultiplier = 10;
}

DWORD WINAPI CANWrite1(LPVOID param)
{
	PCAN_PARA pstPara = (PCAN_PARA) param;
	CANMSG *msg;
	int max1, max2, counts = 0;
	char *sendbuf;
	sendbuf = (char*)malloc(sizeof(CANMSG));


	while(1)
	{
		if (pstPara->eSend)
		{
			msg = (CANMSG*) sendbuf;
			msg->identifier = 0x102;
			msg->length = 8;

			msg->data[0] = (char) pstPara->readDI[0];
			msg->data[1] = (char) pstPara->readDI[1];
			msg->data[2] = (char) pstPara->readDI[2];
			msg->data[3] = (char) pstPara->readDI[3];
			msg->data[4] = (char) pstPara->readDI[4];
			msg->data[5] = (char) pstPara->readDI[5];
			msg->data[6] = (char) pstPara->readDI[6];
			msg->data[7] = (char) pstPara->readDI[7];

			msg++;
			msg->identifier = 0x109;
			msg->length = 8;

			msg->data[0] = (char) 1;
			msg->data[1] = (char) 2;
			msg->data[2] = (char) 3;
			msg->data[3] = (char) 4;
			msg->data[4] = (char) 5;
			msg->data[5] = (char) 6;
			msg->data[6] = (char) 7;
			msg->data[7] = (char) 8;

			max1 = mxcan_write(pstPara->hPort1, sendbuf, 2*sizeof(CANMSG), NULL);

			msg = (CANMSG*) sendbuf;
			msg->identifier = 0x200;
			msg->length = 8;
			msg->data[0] = (char) pstPara->AI_Value[0] & 0xff;
			msg->data[1] = (char) (pstPara->AI_Value[0] /256);
			msg->data[2] = (char) pstPara->AI_Value[1] & 0xff;
			msg->data[3] = (char) (pstPara->AI_Value[1] /256);
			msg->data[4] = (char) pstPara->AI_Value[2] & 0xff;
			msg->data[5] = (char) (pstPara->AI_Value[2] /256);
			msg->data[6] = (char) pstPara->AI_Value[3] & 0xff;
			msg->data[7] = (char) (pstPara->AI_Value[3] /256);

			msg++;
			msg->identifier = 0x201;
			msg->length = 8;
			msg->data[0] = (char) counts;
			msg->data[1] = (char) 2;
			msg->data[2] = (char) 3;
			msg->data[3] = (char) 4;
			msg->data[4] = (char) 5;
			msg->data[5] = (char) 6;
			msg->data[6] = (char) 7;
			msg->data[7] = (char) 8;

			max2 = mxcan_write(pstPara->hPort2, sendbuf, 2*sizeof(CANMSG), NULL);			
			}				
			
		counts++;
		Sleep(99);
	}

	free(sendbuf);

	return 0;
}

DWORD WINAPI DIOThread(LPVOID param )
{
	PCAN_PARA pstPara = (PCAN_PARA) param;
	HANDLE	hDIO = NULL;       // DIO handle
	int		i;
	int		dataDI[12], dataDO[12];
	int     count = 0;

	// Open the DIO driver
	hDIO = mxdgio_open();

	// Display all of the DI/DO
	for ( i=0; i<DIO_PORTS; i++)
	{
		// set DO to low(0)
		mxdgio_set_output_signal_low(hDIO, i);

		dataDI[i] = mxdgio_get_input_signal(hDIO, i);
		dataDO[i] = mxdgio_get_output_signal(hDIO, i);
	}

	while (pstPara->enable)
	{
		// Display all of the DI/DO
		for ( i=0; i<DIO_PORTS; i++)
		{
			if (pstPara->setDO[i]) 	mxdgio_set_output_signal_high(hDIO, i);
		    else mxdgio_set_output_signal_low(hDIO, i);
			dataDI[i] = mxdgio_get_input_signal(hDIO, i);
			dataDO[i] = mxdgio_get_output_signal(hDIO, i);
		}
		printf("\x1b[2J");
		printf("===============================\n");
		printf("count = %d\n",count);
		printf("   0 1 2 3 4 5 6 7 8 9 A B \n");
		printf("DO %d %d %d %d %d %d %d %d %d %d %d %d\n",  dataDO[0], dataDO[1], dataDO[2], dataDO[3], dataDO[4], dataDO[5], dataDO[6], dataDO[7], dataDO[8], dataDO[9], dataDO[10], dataDO[11]);
		printf("DI %d %d %d %d %d %d %d %d %d %d %d %d\n",  dataDI[0], dataDI[1], dataDI[2], dataDI[3], dataDI[4], dataDI[5], dataDI[6], dataDI[7], dataDI[8], dataDI[9], dataDI[10], dataDI[11]);
		printf("VI0 = %10.3f\n", pstPara->AI_Value[0]*0.002);
		printf("VI1 = %10.3f\n", pstPara->AI_Value[1]*0.002);
		printf("VI2 = %10.3f\n", pstPara->AI_Value[2]*0.05);
		printf("VI3 = %10.3f\n", pstPara->AI_Value[3]*0.04515);

		count++;
		Sleep(1000);
	}
	// Close DO;
	mxdgio_close(hDIO);
	return 0;
}

DWORD WINAPI AIO_7026(LPVOID param)
{
	PCAN_PARA pstPara = (PCAN_PARA) param;
	BYTE buffer[13] = {0};   // data receiving buffer
	BYTE AI_Set[6];
	BYTE AI_R[8];
	BYTE AO_W[8];
	BYTE AO_R[8];
	int value;

	short d;
	BYTE mbrtu_buf[13];
	int len;	
	DWORD dwReturned = 0;
	AI_Set[0] = 0x01;
	AI_Set[1] = 0x04;
	AI_Set[2] = 0x00;
	AI_Set[3] = 0x00;
	AI_Set[4] = 0x00;
	AI_Set[5] = 0x04;

	AO_W[0] = 0x01;
	AO_W[1] = 0x06;
	AO_W[2] = 0x00;
	AO_W[3] = 0x20;

	AO_R[0] = 0x01;
	AO_R[1] = 0x03;
	AO_R[2] = 0x00;
	AO_R[3] = 0x20;
	AO_R[4] = 0x00;
	AO_R[5] = 0x01;
	AO_R[6] = 0x85;
	AO_R[7] = 0xC0;

	len = mbrtu_packet_format(AI_Set, 6, AI_R);	
	WriteFile( pstPara->AIO_PORT, AI_R, 8 , &dwReturned, NULL);
	Sleep(100);

	while(true)
	{
		ReadFile( pstPara->AIO_PORT, buffer, 13, &dwReturned, NULL);
		if ( dwReturned > 0 ) 
		{

			len = mbrtu_packet_format(buffer, 11, mbrtu_buf);
		
			if((mbrtu_buf[11] == buffer[11]) && (mbrtu_buf[12] == buffer[12]))
			{
				if( buffer[1] == 0x04) 
				{
					memcpy(&d, &buffer[3], 2);
					d = BSWAP16(d);
					if(d<0) d = 0;
					pstPara->AI_Value[0] = d;

					memcpy(&d, &buffer[5], 2);
					d = BSWAP16(d);
					if(d<0) d = 0;
					pstPara->AI_Value[1] = d;

					memcpy(&d, &buffer[7], 2);
					d = BSWAP16(d);
					if(d<0) d = 0;
					pstPara->AI_Value[2] = d;

					memcpy(&d, &buffer[9], 2);
					d = BSWAP16(d);
					if(d<0) d = 0;
					pstPara->AI_Value[3] = d;

			/*		memcpy(&d, &buffer[11], 2);
					d = BSWAP16(d);
					pstPara->AI_Value[4] = d;
					memcpy(&d, &buffer[13], 2);
					d = BSWAP16(d);
					pstPara->AI_Value[5] = d;*/
					
				}
			}
		}
     
			value= 6000;

			d = BSWAP16(value);

			memcpy(&AO_W[4], &d, 2);
			len = mbrtu_packet_format(AO_W, 6, mbrtu_buf);
			WriteFile( pstPara->AIO_PORT, mbrtu_buf, len , &dwReturned, NULL);
			Sleep(50);
			ReadFile( pstPara->AIO_PORT, buffer, 8, &dwReturned, NULL);
			Sleep(50);


		WriteFile( pstPara->AIO_PORT, AI_R, 8 , &dwReturned, NULL);	
		Sleep(48);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CAN_PARA CANParaC;
	DWORD dwrtn=0;
	bool IsEnable = true;
	int count = 0;
	int i;
	int     fd1,fd2;
	fd1 = canbus_ini(1);
	fd2 = canbus_ini(2);
	AIO_ini(&CANParaC);
	CANParaC.hPort1 = fd1;
	CANParaC.hPort2 = fd2;
    CANParaC.eSend = true;
	HANDLE AIOControl = CreateThread( NULL, 0, AIO_7026, &CANParaC, 0, NULL); 
	HANDLE htCANW1 = CreateThread( NULL, 0, CANWrite1, &CANParaC, 0, NULL);
	CANParaC.enable = 1;
    HANDLE DIOControl = CreateThread( NULL, 0, DIOThread, &CANParaC, 0, NULL); 
	for ( i=0; i<DIO_PORTS; i++)
	{
		CANParaC.setDO[i] = 1;
	}
	
	while (IsEnable)
	{
		WCHAR ch = getwchar();
		if (ch == _T('0')) CANParaC.setDO[0] = !CANParaC.setDO[0];
		if (ch == _T('1')) CANParaC.setDO[1] = !CANParaC.setDO[1];
		if (ch == _T('2')) CANParaC.setDO[2] = !CANParaC.setDO[2];
		if (ch == _T('3')) CANParaC.setDO[3] = !CANParaC.setDO[3];
		if (ch == _T('4')) CANParaC.setDO[4] = !CANParaC.setDO[4];
		if (ch == _T('5')) CANParaC.setDO[5] = !CANParaC.setDO[5];
		if (ch == _T('6')) CANParaC.setDO[6] = !CANParaC.setDO[6];
		if (ch == _T('7')) CANParaC.setDO[7] = !CANParaC.setDO[7];
		if (ch == _T('8')) CANParaC.setDO[8] = !CANParaC.setDO[8];
		if (ch == _T('9')) CANParaC.setDO[9] = !CANParaC.setDO[9];
		if (ch == _T('a')) CANParaC.setDO[10] = !CANParaC.setDO[10];
		if (ch == _T('b')) CANParaC.setDO[11] = !CANParaC.setDO[11];
		if (ch == _T('q')) IsEnable = false;
	}

    CANParaC.eSend = false;
	dwrtn = 0;
	GetExitCodeThread(DIOControl, &dwrtn);
	if ( dwrtn == STILL_ACTIVE )
		TerminateThread(DIOControl, -1);
	CloseHandle(DIOControl);

	dwrtn = 0;
	GetExitCodeThread(htCANW1, &dwrtn);
	if ( dwrtn == STILL_ACTIVE )
		TerminateThread(htCANW1, -1);
	CloseHandle(htCANW1);

	dwrtn = 0;
	GetExitCodeThread(AIOControl, &dwrtn);
	if ( dwrtn == STILL_ACTIVE )
		TerminateThread(AIOControl, -1);
	CloseHandle(AIOControl);	
	return 0;
}