
#include "stdio.h"
#include "stdlib.h"
#include "mxio.h"
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
//=========================================================================
#define PORT 502 //Modbus TCP port
#define DEVICE_COUNT 2

//=========================================================================
void PressAnyKey();
void CheckErr(int iRet, char *szFunctionName); //check function execution result

//=========================================================================
#define NUM_THREADS 3

// 线程的运行函数
void *say_hello0(void *args)
{
    printf("Hello 0！\n");
    return 0;
}
void *say_hello1(void *args)
{
    printf("Hello 1！\n");
    return 0;
}
void *say_hello2(void *args)
{
    printf("Hello 2！\n");
    return 0;
}

int main(void)
{ // 定义线程的 id 变量，多个变量使用数组
    pthread_t tids[NUM_THREADS];

    //参数依次是：创建的线程id，线程参数，调用的函数，传入的函数参数
    pthread_create(&tids[0], NULL, say_hello0, NULL);

    pthread_create(&tids[1], NULL, say_hello1, NULL);

    pthread_create(&tids[2], NULL, say_hello2, NULL);
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
            printf("---------------------\r\n");
            printf("DI = %d\r\n", SValue[0]);
            printf("---------------------\r\n");

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
                printf("count = %d\r\n", count);
                printf("j = %d\r\n", j);
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

    //等各个线程退出后，进程才结束，否则进程强制结束了，线程可能还没反应过来；
    pthread_exit(NULL);
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
void CheckErr(int iRet, char *szFunctionName)
{
    const char *szErrMsg;
    if (iRet != MXIO_OK)
    {
        switch (iRet)
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
        printf("Function \"%s\" execution Fail. Error Message : %s\r\n", szFunctionName, szErrMsg);
    }
}
