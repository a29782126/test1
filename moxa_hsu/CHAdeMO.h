#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "mxio.h"
#include <moxa/mx_dio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define PORT 502 //Modbus TCP port
#define DEVICE_COUNT 3

#pragma once
typedef	struct _CAN_PARA
{
    bool CAN0_read_enable;
    bool CAN1_read_enable;
    bool CAN0_write_enable;
    bool CAN1_write_enable; 
    bool DIO_moxa5112_enable;
    bool io1242_enable;
    int CANPort_0, CANPort_1;   
    int time_tmp;

    int di_state[4];
    int do_state[4] = {0};

    int iHandle[DEVICE_COUNT] = {0}; //ioLogicK module handle 
    double io1242_AI[4] = {0};
    DWORD io1242_di;
    DWORD io1242_do;
} CAN_PARA, *PCAN_PARA;
