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
#include <string.h>
#include <pthread.h>

#pragma once
typedef	struct _CAN_PARA
{
    bool CAN0_read_enable;
    bool CAN1_read_enable;
    bool CAN0_write_enable;
    bool CAN1_write_enable; 
    int CANPort_0, CANPort_1;   
    int time_tmp;
} CAN_PARA, *PCAN_PARA;
