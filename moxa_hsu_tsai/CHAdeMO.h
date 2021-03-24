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

#define CHAdeMO_PORT 1 //CHAdeMO CAN Comm Port
#define CHROMA_PORT 2  //致茂電子通信埠
#define DO_ON 0        //定義DO ON狀態的傳回值
#define DO_OFF 1       //定義DO OFF狀態的傳回值
#define DI_ON 0        //定義DI ON狀態的傳回值
#define DI_OFF 1       //定義DI OFF狀態的傳回值

//定義DI通道
#define Emeg_Stop 0    //停止充電按鈕，平常為
#define Start_Button 1 //定義充電開始按鈕的DI通道
#define STOP_Button 2  //定義緊急停止的DI通道，平常為
#define DDEA_1 3       //直流漏電偵測，正常為ON
#define LEM_State 4    //LEM，正常為ON
#define POWER12 5      //12V電源供應，正常為ON
#define Perm_j 6       //定義Charge permision and prohibition 的DI通道，此信號接收載具端h狀態
#define GND_DisC 7     //接地線斷路狀態，斷路為OFF
#define PIh_DisC 8     //PIUG IN(h) 斷路狀態，斷路為OFF
#define D2_Status 9    //D2開關偵測
#define D1_Status 10   //D1開關偵測
#define CL_Status 11   //Connect lock 狀態偵測

//定義DO通道
#define DCIN_1 0       //
#define DCIN_2 1       //
#define DC_Switch 4    //12V電源供應切換
#define R2_Switch 5    //R2電阻切換
#define R1_Switch 6    //R1電阻切換
#define PI_Switch 7    //Charge permision and prohibition斷路開關
#define GND_Switch 8   //GND斷路開關
#define Connect_Lock 9 //充電接頭上鎖，電路標註n
#define Relay_d2 10    //充電繼電器d2之DO通道
#define Relay_d1 11    //充電繼電器d1之DO通道

//#define POWER_MX 50000
//#define I_MX    120
#define CHG_TM_QC 15300
#define Vdc_LIMIT 10
#define Vdc_TEST 500

#define CANMSG_MSG_COUNT 9
#define CANMSG_BUFFER_SIZE (CANMSG_MSG_COUNT * sizeof(CANMSG))

#define min(a, b) ({ \
    typeof(a) __min1__ = (a);  \
    typeof(b) __min2__ = (b);  \
    (void)(&__min1__ == &__min2__);  \
    __min1__ < __min2__ ? __min1__ : __min2__; })

int time_cal(int, int);

#define max(a, b) ({ \
    typeof(a) __max1__ = (a);  \
    typeof(b) __max2__ = (b);  \
    (void)(&__max1__ == &__max2__);  \
    __max1__ > __max2__ ? __max1__ : __max2__; })

int time_cal(int, int);

void *Data_INI;
void *Flag_reset;
void *Para_reset;
void *Timer_reset;
void *Result_reset;
//void *DI_Read;
void *DO_write;
void *get_check_time;
/*
void *CH_SUB_01;
void *CH_SUB_02;
void *CH_SUB_03;
void *CH_SUB_04;
void *CH_SUB_05;
void *CH_SUB_06;
*/
unsigned int canbus_ini(int port); //CAN BUS 初始化
int canbus_close(int port);        //CAN BUS 關閉

struct flags
{
    bool start_is_pushed;

    bool stop_is_pushed;
    bool emeg_is_pushed;
    bool chag_is_enable;
    bool chag_is_finish;
    bool result_sending;

    bool Charger_Malfunction;
    bool Vehicle_Malfunction;

    //lin
    bool f10240_Battery_overvoltage;
    bool f10241_Battery_undervoltage;
    bool f10242_Battery_current_deviation_error;
    bool f10243_High_battery_temperature;
    bool f10244_Battery_voltage_deviation_error;

    bool f10250_Vehicle_charging_enable;
    bool f10251_Vehicle_shift_position;
    bool f10252_Charging_system_error;
    bool f10253_vehicle_staus;
    bool f10254_Normal_stop_request_before_charging;

    bool f10800_Welding_detection;

    bool f10950_Charger_status;
    bool f10951_Charger_error;
    bool f10952_Energizing_state;
    bool f10953_Battery_incompatibility;
    bool f10954_Charging_system_error;
    bool f10955_Charging_stop_control;

    bool f11000_Dynamic_control;
    bool f11001_High_current_control;
    bool f11002_High_voltage_control;
    bool f11003_Reserved;

    bool f11800_Dynamic_control;
    bool f11801_High_current_control;
    bool f11802_High_voltage_control;
    bool f11803_Reserved;

    bool f11850_Operating_condition;
    bool f11851_Cooling_function_cable;
    bool f11852_Current_limiting_function_cable;
    bool f11853_Cooling_function_connector;
    bool f11854_Current_limiting_function_connector;
    bool f11855_Over_temperature_protection;
    bool f11856_Reliability_design;

    bool f11860_Reset_max_charging_time;

    bool TOT_01_enable;
    bool TOT_04_enable;
    bool TOT_07_enable;
    bool TOT_08_enable;
    bool TOT_14_enable;
    bool TOT_15_enable;

    bool TOT_01_check;
    bool TOT_04_check;
    bool TOT_07_check;
    bool TOT_08_check;
    bool TOT_14_check;
    bool TOT_15_check;

    bool Enable_Dynamic_Control; //From V1.1
    bool Enable_High_Current;    //From V1.2
    bool Enable_High_Voltage;    //From V2.0

    bool CH_SUB_10_enable;

    flags()
    {

        stop_is_pushed = false;
        emeg_is_pushed = false;
        chag_is_enable = false;
        chag_is_finish = false;
        result_sending = false;

        Vehicle_Malfunction = false;
        Charger_Malfunction = false;
        ////////////////////////lin
        f10240_Battery_overvoltage = false;
        f10241_Battery_undervoltage = false;
        f10242_Battery_current_deviation_error = false;
        f10243_High_battery_temperature = false;
        f10244_Battery_voltage_deviation_error = false;

        f10250_Vehicle_charging_enable = false;
        f10251_Vehicle_shift_position = false;
        f10252_Charging_system_error = false;
        f10253_vehicle_staus = false;
        f10254_Normal_stop_request_before_charging = false;

        f10800_Welding_detection = false;

        f10950_Charger_status = false;
        f10951_Charger_error = false;
        f10952_Energizing_state = false;
        f10953_Battery_incompatibility = false;
        f10954_Charging_system_error = false;
        f10955_Charging_stop_control = false;

        f11000_Dynamic_control = false;
        f11001_High_current_control = false;
        f11002_High_voltage_control = false;
        f11003_Reserved = false;

        f11800_Dynamic_control = false;
        f11801_High_current_control = false;
        f11802_High_voltage_control = false;
        f11803_Reserved = false;

        f11850_Operating_condition = false;
        f11851_Cooling_function_cable = false;
        f11852_Current_limiting_function_cable = false;
        f11853_Cooling_function_connector = false;
        f11854_Current_limiting_function_connector = false;
        f11855_Over_temperature_protection = false;
        f11856_Reliability_design = false;

        f11860_Reset_max_charging_time = false;

        /////////////////////////////

        TOT_01_enable = false;
        TOT_04_enable = false;
        TOT_07_enable = false;
        TOT_08_enable = false;
        TOT_14_enable = false;
        TOT_15_enable = false;

        TOT_01_check = false;
        TOT_04_check = false;
        TOT_07_check = false;
        TOT_08_check = false;
        TOT_14_check = false;
        TOT_15_check = false;

        CH_SUB_10_enable = true;
    }
    ~flags() {}
};

struct DIO_PARA
{
    //HANDLE Port;
    bool enable;
    bool D1_12V, D1_BRK, D2_GND, D2_BRK;
    int DOdata[12];
    int DIdata[12];
    int DO_read[12];
    BYTE DI_Status0;
    BYTE DI_Status1;
    BYTE DO_Status;
};

//定義時間
struct ToTime
{
    double now_time;    /////目前時間
    double OP_time;    //目前執行時間
    double start_time; //程序開始時間
    double begin_time; //充電程序部分開始時間
    double last_time;

    double case_time;   //進入該CASE的時間
    double end_time;    //充電程序部分結束時間
    double finish_time; //完整程序結束時間
    double last_can_time1;
    double last_can_time2;
    double last_can_time0;
    double can1_output;
    double dio_output;
    double real_test_start;
    double OT[6];
    double WT[18];
    double PT[24], Error[28];

    double D1_ON1, D1_ON2, D1_OFF;
    double D2_ON1, D2_ON2, D2_OFF;
    double JK_ON1, JK_OFF, JK_ON2;
    double VE_ON1, VE_OFF, VE_ON2;
    double CUT_h, CUT_GND;

    double Test_start;
    double Test_Time1;
    double Test_Time2;
    double Test_Time3;
    double Test_Time4;
    double Test_Buff_Time;
};

struct CHROMA
{
    bool C_Control, C_Relay, C_IsRunning, C_IsFinish, C_SystemError, C_Result, C_Charger_Button, P_SystemError, P_VoltageError, P_CurrentError, P_IsEnable, P_Communication;

    DWORD C_TaskN, C_TestN, C_Teststep, C_TestPara1, C_TestPara2, C_ChargingV, C_ChargingC, P_MeasureC, P_MeasureV;
    WORD RESIDE_TIME;
};

struct POWER_CTL
{
    bool POWER_ON;
    bool POWER_ON_SET;
    double SET_I;
    double SET_V;
    double READ_I;
    double READ_V;
    double POWER_MAX;
    double I_MAX;
    double V_MAX;
};

typedef struct _CAN_PARA
{
    bool can_0_on, can_1_on, can_2_on, eSend, input_slop_n, input_slop_p, input_slop_error;

    bool CAN0_read_enable;
    bool CAN1_read_enable;
    bool CAN0_write_enable;
    bool CAN1_write_enable;
    bool DIO_moxa5112_enable;
    bool io1242_enable;
    int CANPort_0, CANPort_1;
    int time_tmp;

    int LIMIT_TIME, Vdc, Idc, POWER_MX, I_MX, PassVoltage[4];
    //--------------------
    int V_diff, I_diff;
    int V_Command;
    double I_Command;
    bool SystemIsWorking;
    int charging_stop_code;
    int slop_p, slop_n;
    double Icommand_Tmp, Icommand_Max, Icommand_Min, Icommand_Buff;
    int CHG_I_old;

    //--------------------
    flags ff;
    DIO_PARA DIO;
    ToTime tt;

    int di_state[4];
    int do_state[4] = {0};

    int iHandle[DEVICE_COUNT] = {0}; //ioLogicK module handle
    double io1242_AI[4] = {0};
    DWORD io1242_di;
    DWORD io1242_do;

    //#100
    BYTE MIN_I;   //Minimum charge current
    WORD MIN_V;   //Minimum battery voltage
    WORD BAT_MX;  //Maximum battery voltage
    BYTE TOT_CAP; //Charged rate reference constant

    //#101
    BYTE CHG_TM0;      //Maximum charging time (10s unit)
    BYTE CHG_TM1;      //Maximum charging time (1min unit)
    BYTE CHG_EST;      //Estimated charging time
    double TOT_CAP_KW; //Total capacity of traction battery

    //#102
    BYTE V_PROTOCOL; //CHAdeMO protocol number
    WORD CHG_MX;     //Target battery voltage
    BYTE CHG_I;      //Charging current request
    BYTE REM_CAP;    //State of charge

    //#108
    BYTE Welding_Detect; //Welding detection
    WORD LIMIT_V;        //Available output voltage
    WORD LIMIT_I;        //Available output current
    WORD LIMIT_VOLT;     //Threshold voltage

    //#109
    BYTE C_PROTOCOL; //CHAdeMO protocol number
    WORD PRE_V;      //Present output voltage
    WORD PRE_I;      //Present charging current
    BYTE REM_TM0;    //Remaining charging time(In the unit of 10 s)
    BYTE REM_TM1;    //Remaining charging time(In the unit of 1 min)

    //#110
    WORD CHG_I_Ext; //Charging current request

    //#118
    WORD LIMIT_I_Ext; //Available output current
    WORD PRE_I_Ext;   //Present charging current

    CHROMA Chroma;
	POWER_CTL POWER8000;
    short V_com, I_com;
    DWORD waiting_send_result;
    DWORD charging_step;
    DWORD next_step;

} CAN_PARA, *PCAN_PARA;

void *CH_SUB_01(void *param)
{
    PCAN_PARA pstPara = (PCAN_PARA)param;

    pstPara->LIMIT_V = 500;
    printf("CH_SUB_01\r\n");
    if (pstPara->CHG_MX > 0)
    {

        if (pstPara->CHG_MX <= pstPara->BAT_MX)
        {
            if (pstPara->CHG_MX <= pstPara->LIMIT_V)
            {
                pstPara->LIMIT_VOLT = min(pstPara->CHG_MX, pstPara->LIMIT_V); //( pstPara->CHG_MX, pstPara->LIMIT_V)
                printf("CHG_MX = %d.\r\n", pstPara->CHG_MX);
                printf("LIMIT_VOLT = %d.\r\n", pstPara->LIMIT_VOLT);

                //Available output current calculation
                if (pstPara->ff.Enable_High_Current)
                {
                    pstPara->LIMIT_I_Ext = pstPara->POWER_MX * 1000 / pstPara->LIMIT_VOLT;
                    if (!(pstPara->LIMIT_I_Ext < pstPara->I_MX))
                        pstPara->LIMIT_I_Ext = pstPara->I_MX;
                    pstPara->LIMIT_I = pstPara->LIMIT_I_Ext;
                    if (pstPara->LIMIT_I_Ext >= 255)
                        pstPara->LIMIT_I = 255;
                }
                else
                {
                    pstPara->LIMIT_I = pstPara->POWER_MX * 1000 / pstPara->LIMIT_VOLT;
                    printf("LIMIT_I = %d.\r\n", pstPara->LIMIT_I);
                    if (!(pstPara->LIMIT_I < pstPara->I_MX))
                        pstPara->LIMIT_I = pstPara->I_MX;
                    pstPara->LIMIT_I_Ext = 0;
                }
            }

            //Battery Incompatibility #109.5.3=1
            else
            {
                pstPara->ff.f10953_Battery_incompatibility = true;
                pstPara->tt.Error[22] = pstPara->tt.OP_time;
            }
        }
        //Battery malfunction #109.5.4=1
        else
        {
            pstPara->ff.f10954_Charging_system_error = true;
            pstPara->tt.Error[21] = pstPara->tt.OP_time;
        }
    }
    //return;
}

void *CH_SUB_02(void *param)
{
    PCAN_PARA pstPara = (PCAN_PARA)param;
    int CHG_TM;

    //Maximum charging time calculation
    if (pstPara->CHG_TM0 == 0xff)
    {
        CHG_TM = pstPara->CHG_TM1 * 60;
    }
    else
    {
        CHG_TM = pstPara->CHG_TM0 * 10;
    }
    if (CHG_TM <= CHG_TM_QC)
    {
        pstPara->LIMIT_TIME = CHG_TM;
    }
    else
    {
        pstPara->LIMIT_TIME = CHG_TM_QC;
    }

    //Set Calculation result as CAN data
    if (pstPara->LIMIT_TIME > 2540)
    {
        pstPara->REM_TM0 = 0xff;
        pstPara->REM_TM1 = pstPara->LIMIT_TIME / 60;
    }
    else
    {
        pstPara->REM_TM0 = pstPara->LIMIT_TIME / 10;
        pstPara->REM_TM1 = 0x0;
    }
}

void CH_SUB_05(void *param)
{
    PCAN_PARA pstPara = (PCAN_PARA)param;

    pstPara->ff.Enable_High_Current = (pstPara->ff.f11001_High_current_control && pstPara->ff.f11801_High_current_control);
    pstPara->ff.Enable_Dynamic_Control = (pstPara->ff.f11000_Dynamic_control && pstPara->ff.f11800_Dynamic_control);
    pstPara->ff.Enable_High_Voltage = (pstPara->ff.f11802_High_voltage_control && pstPara->ff.f11802_High_voltage_control);
}

void CH_SUB_06(void *param)
{
    PCAN_PARA pstPara = (PCAN_PARA)param;

    if ((pstPara->ff.TOT_01_check) && (pstPara->tt.Error[0] == 0xffff))
    {
        pstPara->tt.Error[0] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 01 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.TOT_04_check) && (pstPara->tt.Error[1] == 0xffff))
    {
        pstPara->tt.Error[1] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 04 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.TOT_07_check) && (pstPara->tt.Error[2] == 0xffff))
    {
        pstPara->tt.Error[2] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 07 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.TOT_08_check) && (pstPara->tt.Error[3] == 0xffff))
    {
        pstPara->tt.Error[3] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 08 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.TOT_14_check) && (pstPara->tt.Error[4] == 0xffff))
    {
        pstPara->tt.Error[4] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 14 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.TOT_15_check) && (pstPara->tt.Error[5] == 0xffff))
    {
        pstPara->tt.Error[5] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! TOT 15 detected...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }

    if ((pstPara->ff.f10240_Battery_overvoltage) && (pstPara->tt.Error[6] == 0xffff))
    {
        pstPara->tt.Error[6] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10240 Battery_overvoltage...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10241_Battery_undervoltage) && (pstPara->tt.Error[7] == 0xffff))
    {
        pstPara->tt.Error[7] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10241 Battery_undervoltage...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10242_Battery_current_deviation_error) && (pstPara->tt.Error[8] == 0xffff))
    {
        pstPara->tt.Error[8] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10242 Battery_current_deviation_error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10243_High_battery_temperature) && (pstPara->tt.Error[9] == 0xffff))
    {
        pstPara->tt.Error[9] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10243_High_battery_temperature...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10244_Battery_voltage_deviation_error) && (pstPara->tt.Error[10] == 0xffff))
    {
        pstPara->tt.Error[10] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10244_Battery_voltage_deviation_error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }

    if (pstPara->tt.Error[11] == 0xffff)
    {

        if (((pstPara->charging_step == 0x1200) && (pstPara->ff.f10250_Vehicle_charging_enable) && ((pstPara->tt.OP_time - pstPara->tt.PT[0]) >= 50)) ||
            ((pstPara->charging_step >= 0x1400) && (pstPara->charging_step <= 0x1800) && (!pstPara->ff.f10250_Vehicle_charging_enable)) ||
            ((pstPara->charging_step >= 0x3400) && (pstPara->charging_step <= 0x3700) && (pstPara->ff.f10250_Vehicle_charging_enable)))
        {
            pstPara->tt.Error[11] = pstPara->tt.OP_time;
            pstPara->ff.Vehicle_Malfunction = true;
            printf("%10d, %X, Warning!!! f10250_Vehicle_charging_enable state error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }

    if ((pstPara->ff.f10251_Vehicle_shift_position) && (pstPara->tt.Error[12] == 0xffff))
    {
        pstPara->tt.Error[12] = pstPara->tt.OP_time;
        pstPara->ff.stop_is_pushed = true;
        printf("%10d, %X, Warning!!! f10251_Vehicle_shift_position...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10252_Charging_system_error) && (pstPara->tt.Error[13] == 0xffff))
    {
        pstPara->tt.Error[13] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! f10252_Other_Vehicle_faults...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }

    if ((pstPara->ff.f10951_Charger_error) && (pstPara->tt.Error[14] == 0xffff) && (pstPara->charging_step > 0x1300))
    {
        pstPara->tt.Error[14] = pstPara->tt.OP_time;
        pstPara->ff.Charger_Malfunction = true;
        printf("%10d, %X, Warning!!! f10951_Charger_malfunction...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10953_Battery_incompatibility) && (pstPara->tt.Error[15] == 0xffff) && (pstPara->charging_step > 0x1300))
    {
        pstPara->tt.Error[15] = pstPara->tt.OP_time;
        //pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!!f10953_Battery_incompatibility...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }
    if ((pstPara->ff.f10954_Charging_system_error) && (pstPara->tt.Error[16] == 0xffff) && (pstPara->charging_step > 0x1300))
    {
        pstPara->tt.Error[16] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!!f10954_Battery_malfunction...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }

    if (pstPara->tt.Error[17] == 0xffff)
    {

        if (((pstPara->charging_step == 0x1200) && (pstPara->DIO.DIdata[Perm_j] == DI_ON) && ((pstPara->tt.OP_time - pstPara->tt.PT[0]) >= 50)) ||
            ((pstPara->charging_step >= 0x1400) && (pstPara->charging_step <= 0x1900) && (pstPara->DIO.DIdata[Perm_j] == DI_OFF)) ||
            ((pstPara->charging_step >= 0x3400) && (pstPara->charging_step <= 0x3700) && (pstPara->DIO.DIdata[Perm_j] == DI_ON)))
        {
            pstPara->tt.Error[17] = pstPara->tt.OP_time;
            pstPara->ff.Vehicle_Malfunction = true;
            printf("%10d, %X, Warning!!! Charging enable switch K state error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }

    if (pstPara->tt.Error[18] == 0xffff)
    {

        if (((pstPara->charging_step >= 0x1300) && (pstPara->charging_step <= 0x1500) && (pstPara->CHG_I > 0)) ||
            ((pstPara->charging_step >= 0x3400) && (pstPara->charging_step <= 0x3800) && (pstPara->CHG_I > 0)))
        {
            pstPara->tt.Error[18] = pstPara->tt.OP_time;
            pstPara->ff.Vehicle_Malfunction = true;
            printf("%10d, %X, Warning!!! Charging Current error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }

    if ((pstPara->charging_step == 0x2100) && (pstPara->tt.Error[19] == 0xffff) && (pstPara->CHG_I > pstPara->LIMIT_I))
    {

        pstPara->tt.Error[19] = pstPara->tt.OP_time;
        pstPara->ff.Vehicle_Malfunction = true;
        printf("%10d, %X, Warning!!! Charging Current error...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
    }

    if ((pstPara->tt.Error[20] == 0xffff) && (pstPara->charging_step < 0x3500) && (pstPara->charging_step >= 0x1400))
    {
        if ((pstPara->tt.OP_time - pstPara->tt.last_can_time0 > 100) || (pstPara->tt.OP_time - pstPara->tt.last_can_time1 > 100) || (pstPara->tt.OP_time - pstPara->tt.last_can_time2 > 100))
        {
            pstPara->tt.Error[20] = pstPara->tt.OP_time;
            pstPara->ff.Vehicle_Malfunction = true;
            printf("%10d, %X, Warning!!! CAN missiing over 1 sec...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }
    if ((pstPara->tt.Error[26] == 0xffff) && (pstPara->charging_step <= 0x1800))
    {
        if (pstPara->ff.stop_is_pushed)
        {
            pstPara->tt.Error[26] = pstPara->tt.OP_time;
            printf("%10d, %X, Warning!!! Stop button is pushed...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }
    if (pstPara->tt.Error[27] == 0xffff)
    {
        if (pstPara->ff.emeg_is_pushed)
        {
            pstPara->tt.Error[14] = pstPara->tt.OP_time;
            pstPara->tt.Error[27] = pstPara->tt.OP_time;
            pstPara->ff.f10951_Charger_error = true;
            pstPara->ff.Charger_Malfunction = true;
            printf("%10d, %X, Warning!!! Emeg STOP pushed...\r\n", pstPara->tt.OP_time, pstPara->charging_step);
        }
    }

    if (pstPara->ff.CH_SUB_10_enable)
    {

        if ((pstPara->charging_step >= 0x1100) && (pstPara->charging_step <= 0x1800))
        {
            if ((pstPara->ff.Charger_Malfunction) || (pstPara->ff.Vehicle_Malfunction) || (pstPara->ff.stop_is_pushed) || pstPara->ff.f10953_Battery_incompatibility)
            {

                pstPara->next_step = 0x3500;
                pstPara->tt.PT[18] = pstPara->tt.OP_time;
                pstPara->DIO.DOdata[Relay_d1] = DO_OFF;
                printf("%10d, %X, set relay d1 off %d...\r\n", pstPara->tt.OP_time, pstPara->charging_step, pstPara->tt.PT[18]);
            }
        }
        else if (pstPara->charging_step == 0x2100)
        {
            if (pstPara->ff.Charger_Malfunction || pstPara->ff.Vehicle_Malfunction || pstPara->ff.f10953_Battery_incompatibility)
            {
                pstPara->next_step = 0x3100;
            }
        }
    }
}