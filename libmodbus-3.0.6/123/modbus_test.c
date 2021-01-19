#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "/usr/local/include/modbus/modbus.h"

//#define MODBUS_DEV_NAME    "/dev/ttymxc1"    ///< 串口设备
//#define MODBUS_DEV_NAME    "/dev/ttyS0"
#define MODBUS_DEV_NAME    "/dev/ttyM0"
int main(void)
{
    modbus_t *ctx =NULL;

    // 以串口的方式创建libmobus实例,并设置参数
    //ctx = modbus_new_rtu(MODBUS_DEV_NAME, 115200, 'N', 8, 1);
    ctx = modbus_new_rtu(MODBUS_DEV_NAME, 9600, 'N', 8, 1);
    if (ctx == NULL)                //使用UART4,对应的设备描述符为ttymxc3
    {
        fprintf(stderr, "Unable to allocate libmodbus contex\n");
        return -1;
    }
    // 使用RS485时需考虑设置串口模式、RTS引脚等
   // modbus_rtu_set_serial_mode(MODBUS_RTU_RS485);    //设置串口模式

    modbus_set_debug(ctx, 1);      //设置1可看到调试信息
    modbus_set_slave(ctx, 1);      //设置slave ID

    if (modbus_connect(ctx) == -1) //等待连接设备
    {
        fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
        return -1;
    }

    int i,rc;
    uint16_t tab_reg[64] = {0}; //定义存放数据的数组
    while (1)
    {
        printf("\n----------------\n");
        //读取保持寄存器的值，可读取多个连续输入保持寄存器
        rc = modbus_read_registers(ctx, 0, 10, tab_reg);
        if (rc == -1)
        {
            fprintf(stderr,"%s\n", modbus_strerror(errno));
            return -1;
        }
        for (i=0; i<10; i++)
        {
            printf("reg[%d] = %d(0x%x)\n", i, tab_reg[i], tab_reg[i]);
        }

        usleep(5000000);
    }
    modbus_close(ctx);  //关闭modbus连接
    modbus_free(ctx);   //释放modbus资源，使用完libmodbus需要释放掉

    return 0;
}