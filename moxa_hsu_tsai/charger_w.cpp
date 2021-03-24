#include "IOThread.h"
#include "CHAdeMO.h"

//// 主線程
int main()
{
    pthread_t tids[6];
    int ret;
    int do_tmp;
    int di_tmp;

    //時間變數宣告
    double start, end;
    double time_used;

    CAN_PARA AllPara;
    AllPara.CAN0_write_enable = true;
    AllPara.CAN1_write_enable = true;
    AllPara.CAN0_read_enable = true;
    AllPara.CAN1_read_enable = true;
    AllPara.DIO_moxa5112_enable = true;
    AllPara.io1242_enable = true;

    AllPara.CANPort_0 = CANPORT_DEF(0);
    AllPara.CANPort_1 = CANPORT_DEF(1);
    ret = MXEIO_Init();
    AllPara.iHandle[0] = ioLogic_set(0);
    AllPara.do_state[0] = 1;

    //	ret = mx_dio_init();
    pthread_create(&tids[0], NULL, canwrite0, &AllPara);
    pthread_create(&tids[1], NULL, canwrite1, &AllPara);
    pthread_create(&tids[2], NULL, canread0, &AllPara);
    pthread_create(&tids[3], NULL, canread1, &AllPara);
    pthread_create(&tids[4], NULL, moxa5112_dio, &AllPara);
    pthread_create(&tids[5], NULL, ioLogic1242, &AllPara);
    /* //擷取程式起始時間
    start = get_now_time();
    usleep(990000);*/

    /*for (int i = 0; i < 100; i++)
    {
        //擷取目前時間
        end = get_now_time();
        //計算時間差並列印
        time_used = end - start;
        AllPara.time_tmp = time_used;
        printf("Time %d = %f\n", i + 1, time_used);

        printf("AI: ");
        for (int j = 0; j < 4; j++)
            printf(" %05.3f", AllPara.io1242_AI[j]);
        printf(" DI: %2d", AllPara.io1242_di);
        printf("\n");

        do_tmp = AllPara.do_state[0];
        AllPara.do_state[0] = AllPara.do_state[1];
        AllPara.do_state[1] = AllPara.do_state[2];
        AllPara.do_state[2] = AllPara.do_state[3];
        AllPara.do_state[3] = do_tmp;

        AllPara.io1242_do = AllPara.do_state[0] + AllPara.do_state[1] * 2 + AllPara.do_state[2] * 4 + AllPara.do_state[3] * 8;

        usleep(990000);
    }*/
    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    //變數宣告
    bool abnormal = false;
    int fd1, fd2;
    //	int MAX_TIME;
    //	int I_SetValue;
    //	int waiting_counts,
    //	int counts_c1, counts_c2, counts_v;
    //	bool TimeOutCheck, TimeComplianceCheck;
    bool echo = true;
    DWORD reside_time;
    double V_command_tmp;
    unsigned int fdAI, fd232;

    //宣告systemtime變數用來取得系統時間
    bool main_thread_enable = true; //主執行緒啟動致能
    CAN_PARA CANParaC;
    DWORD dwrtn = 0;

    //485 & RS232control

    //開啟CAN執行緒
    CANParaC.next_step = 0x1000;
    CANParaC.tt.start_time = 0;

    Data_INI;
    Flag_reset;
    Timer_reset;
    Para_reset;

    CANParaC.ff.stop_is_pushed = false;
    CANParaC.ff.emeg_is_pushed = false;

    CANParaC.POWER_MX = 50;
    CANParaC.I_MX = 10;

    CANParaC.Chroma.C_TaskN = 0x0000;
    int tmp;
    CANParaC.ff.start_is_pushed = false;
    usleep(1000000); //Sleep(1000);
    //擷取程式起始時間
    start = get_now_time();
    usleep(990000);
    while (main_thread_enable)
    {

        //擷取目前時間
        end = get_now_time();
        //計算時間差並列印
        time_used = end - start;
        AllPara.time_tmp = time_used;
        //讀取時間
        /* CANParaC.tt.now_time = GetTickCount();
        CANParaC.tt.OP_time = (CANParaC.tt.now_time - CANParaC.tt.start_time) / 10;
        CANParaC.charging_step = CANParaC.next_step;*/
        DI_Read;
        get_check_time;

        CANParaC.Vdc = CANParaC.V_com;
        CANParaC.Idc = CANParaC.I_com;

        if (CANParaC.V_diff != 999)
            tmp = CANParaC.V_diff;
        else
            tmp = CANParaC.Vdc;

        if (tmp > 600)
            tmp = 600;
        if (tmp < 0)
            tmp = 0;
        CANParaC.PRE_V = tmp;

        if (CANParaC.I_diff != 999)
            tmp = CANParaC.I_diff;
        else
            tmp = CANParaC.Idc;

        if (CANParaC.ff.Enable_High_Current)
            CANParaC.PRE_I_Ext = tmp;
        else
            CANParaC.PRE_I_Ext = 0;
        if (tmp > 255)
            tmp = 255;
        if (tmp < 0)
            tmp = 0;

        CANParaC.PRE_I = tmp;

        CANParaC.ff.stop_is_pushed = ((CANParaC.DIO.DIdata[STOP_Button] == DI_ON) || CANParaC.ff.stop_is_pushed);
        CANParaC.ff.emeg_is_pushed = ((CANParaC.DIO.DIdata[Emeg_Stop] == DI_ON) || CANParaC.ff.emeg_is_pushed);

#pragma region Main flow
        switch (CANParaC.charging_step)
        {

#pragma region Before Charging
        case 0x1000:
            if (echo)
            {
                printf("%10d, %X, System start: Waiting for Switch(d1) = ON command.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
                CANParaC.Chroma.C_Control = false;
                CANParaC.Chroma.C_Relay = false;
                CANParaC.Idc = 0;
                CANParaC.Vdc = 0;
                CANParaC.V_Command = 500;
                CANParaC.I_Command = 10;
                CANParaC.CHG_I = 0;
                Timer_reset;
                Para_reset;
                Flag_reset;
                CANParaC.ff.chag_is_enable = true;
                CANParaC.SystemIsWorking = false;
                CANParaC.ff.result_sending = false;
                CANParaC.Chroma.C_Charger_Button = false;
                CANParaC.ff.CH_SUB_10_enable = true;
                reside_time = 0;
            }

            if ((CANParaC.DIO.DIdata[Start_Button] == DI_ON) || (CANParaC.ff.start_is_pushed))
            {
                CANParaC.can_0_on = false;
                CANParaC.can_1_on = false;
                CANParaC.can_2_on = false;
                Flag_reset;
                CANParaC.ff.chag_is_enable = false;
                CANParaC.SystemIsWorking = true;
                CANParaC.Chroma.C_Result = false;
                CANParaC.ff.stop_is_pushed = false;
                CANParaC.ff.emeg_is_pushed = false;

                CANParaC.ff.stop_is_pushed = false;
                //mxcan_purge_buffer(CANParaC.hPort1, 0);
                usleep(200000);
                CANParaC.DIO.DOdata[Relay_d1] = DO_ON;
                Result_reset;
                CANParaC.Chroma.C_IsFinish = false;
                CANParaC.Chroma.C_IsRunning = false;
                CANParaC.next_step = 0x1100;
                CANParaC.tt.start_time = CANParaC.tt.now_time;
                CANParaC.V_diff = 999;
                CANParaC.I_diff = 999;

                CANParaC.tt.Test_start = 0;
                CANParaC.tt.Test_Buff_Time = 0;
                CANParaC.tt.Test_Time1 = 0xffff;
                CANParaC.tt.Test_Time2 = 0xffff;
                CANParaC.tt.Test_Time3 = 0xffff;
                CANParaC.tt.Test_Time4 = 0xffff;

                printf("%10d, %X, Start button is pushed. set switch(d1)  = ON. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);

                echo = true;
            }
            break;

        case 0x1100:
            if (echo)
            {
                printf("%10d, %X, Waiting for Vehicle CAN Start.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.ff.TOT_01_enable = true;
                CANParaC.ff.TOT_04_enable = true;
                CANParaC.ff.start_is_pushed = false;
                echo = false;
            }

            if (CANParaC.ff.stop_is_pushed || CANParaC.ff.emeg_is_pushed)
            {
                echo = true;
                CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                CANParaC.next_step = 0x3500;
                CANParaC.tt.PT[18] = CANParaC.tt.OP_time;
                printf("%10d, %X, Stop button detected. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            if (CANParaC.can_0_on && CANParaC.can_1_on && CANParaC.can_2_on)
            {
                CANParaC.tt.PT[0] = CANParaC.tt.OP_time;
                CH_SUB_05(&CANParaC);
                CANParaC.next_step = 0x1200;
                echo = true;
                printf("%10d, %X, Vehicle CAN received. GOTO Next...%d\r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.tt.PT[0]);
            }
            break;

        case 0x1200:
            if (echo)
            {
                printf("%10d, %X, Prepare Charger CAN data.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }

            CH_SUB_01(&CANParaC);

            if (((CANParaC.tt.PT[1] == 0) && (CANParaC.tt.OP_time - CANParaC.tt.PT[0]) > CANParaC.tt.WT[0]))
            {

                CANParaC.tt.PT[1] = CANParaC.tt.OP_time;
                printf("%10d, %X, Set Pt[1] = %d.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.tt.PT[1]);
            }

            if ((CANParaC.tt.PT[1] > 0) && (CANParaC.tt.can1_output > CANParaC.tt.PT[1]))
            {
                echo = true;

                CANParaC.next_step = 0x1300;
                printf("%10d, %X, Charging CAN start sending... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            break;

        case 0x1300:
            if (echo)
            {
                printf("%10d, %X, Waiting for charging enable.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.eSend = true;
                echo = false;
            }
            CH_SUB_01(&CANParaC);

            if (CANParaC.tt.PT[2] == 0)
            {
                if (CANParaC.DIO.DIdata[Perm_j] == DI_ON)
                    CANParaC.tt.PT[2] = CANParaC.tt.OP_time;
            }

            if (CANParaC.tt.PT[3] == 0)
            {
                if (CANParaC.ff.f10250_Vehicle_charging_enable)
                    CANParaC.tt.PT[3] = CANParaC.tt.OP_time;
            }

            if ((CANParaC.tt.PT[3] > 0) && (CANParaC.tt.PT[2] > 0))
            {
                CANParaC.next_step = 0x1400;
                CANParaC.Chroma.C_Relay = true;
                echo = true;
                printf("%10d, %X, Charging enable received.. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            break;

        case 0x1400:
            if (echo)
            {
                printf("%10d, %X, Start insulation diagnosis.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                printf("%10d, %X, Checking for  DC BUS.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }

            if (CANParaC.Vdc > Vdc_LIMIT)
                CANParaC.ff.f10954_Charging_system_error = true;
            if (((CANParaC.tt.PT[4] == 0) && (CANParaC.tt.OP_time - CANParaC.tt.PT[2]) > CANParaC.tt.WT[1]) && ((CANParaC.tt.OP_time - CANParaC.tt.PT[3]) > CANParaC.tt.WT[1]))
            {

                CANParaC.DIO.DOdata[Connect_Lock] = DO_ON;
                CANParaC.tt.PT[4] = CANParaC.tt.OP_time;
                CANParaC.ff.f10952_Energizing_state = true;
            }

            if ((CANParaC.tt.PT[4] > 0) && (CANParaC.tt.can1_output > CANParaC.tt.PT[4]))
            {
                echo = true;
                CANParaC.next_step = 0x1500;
                printf("%10d, %X, GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            CH_SUB_01(&CANParaC);

            break;

        case 0x1500:
            if (echo)
            {
                printf("%10d, %X, Start insulation diagnosis.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
                CANParaC.Chroma.C_Control = true;
                CANParaC.V_Command = 500;
                CANParaC.I_Command = 10;
                CANParaC.tt.PT[5] = CANParaC.tt.OP_time;
            }
            if (CANParaC.Idc > 5)
            {
                echo = true;
                CANParaC.ff.f10951_Charger_error = true;
                echo = true;
                printf("%10d, %X, Warning!! Idc >5A detected.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            if ((CANParaC.DIO.DIdata[DDEA_1] == DI_OFF))
            {
                echo = true;
                CANParaC.ff.f10951_Charger_error = true;
                echo = true;
                printf("%10d, %X, Warning!! DDEA detected.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            if ((CANParaC.tt.OP_time - CANParaC.tt.PT[5]) > CANParaC.tt.WT[3])
            {
                CANParaC.tt.PT[6] = CANParaC.tt.OP_time;
                CANParaC.V_Command = 0;
                CANParaC.I_Command = 0;
                CANParaC.next_step = 0x1501;
                echo = true;
            }
            CH_SUB_01(&CANParaC);
            break;

        case 0x1501:
            if (echo)
            {
                printf("%10d, %X, Waiting for Vdc <=20V.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);

                echo = false;
            }

            if ((CANParaC.Vdc < 20) && ((CANParaC.tt.OP_time - CANParaC.tt.PT[1]) > CANParaC.tt.WT[4]))
            {
                if (CANParaC.tt.PT[7] == 0)
                {
                    CANParaC.DIO.DOdata[Relay_d2] = DO_ON;
                    CANParaC.tt.PT[7] = CANParaC.tt.OP_time;
                    CANParaC.ff.TOT_07_enable = true;
                    CANParaC.ff.TOT_08_enable = true;
                    printf("%10d, %X, Set Switch(d2) = ON... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                }
            }

            if (((CANParaC.tt.OP_time - CANParaC.tt.PT[7]) > 1) && (CANParaC.DIO.DIdata[D2_Status] == DI_OFF))
            {
                echo = true;
                CANParaC.next_step = 0x1600;
                printf("%10d, %X, GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            break;

        case 0x1600:
            if (echo)
            {
                printf("%10d, %X, Waiting for Vdc >=50V.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }
            if (CANParaC.Vdc > 50)
            {
                CANParaC.next_step = 0x1700;
                echo = true;
                CANParaC.tt.PT[8] = CANParaC.tt.OP_time;
                printf("%10d, %X, Vdc > 50 detected... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            CH_SUB_01(&CANParaC);
            break;

        case 0x1700:
            if (echo)
            {
                printf("%10d, %X, Waiting for I command > 0A.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }
            if (CANParaC.CHG_I > 0)
            {
                CANParaC.next_step = 0x1800;
                echo = true;
                CANParaC.tt.PT[9] = CANParaC.tt.OP_time;
                printf("%10d, %X, I command > 0 detected... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            CH_SUB_01(&CANParaC);
            break;

        case 0x1800:
            if (echo)
            {
                printf("%10d, %X, Waiting for charging enable.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);

                echo = false;
            }

            if (CANParaC.tt.PT[10] == 0)
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[9]) > CANParaC.tt.WT[6])
                {
                    CANParaC.ff.f10950_Charger_status = true;
                    CANParaC.tt.PT[10] = CANParaC.tt.OP_time;
                }
            }

            if (CANParaC.tt.PT[11] == 0)
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[9]) > CANParaC.tt.WT[7])
                {
                    CANParaC.ff.f10955_Charging_stop_control = false;
                    CANParaC.tt.PT[11] = CANParaC.tt.OP_time;
                }
            }

            if ((CANParaC.tt.PT[10] > 0) && (CANParaC.tt.PT[11] > 0))
            {
                CANParaC.next_step = 0x2100;
                CANParaC.Chroma.C_Relay = true;
                CANParaC.Chroma.C_Control = true;
                CANParaC.V_Command = CANParaC.Vdc;
                V_command_tmp = CANParaC.Vdc;
                CANParaC.I_Command = CANParaC.CHG_I;
                CANParaC.charging_stop_code = 0;
                echo = true;
                printf("%10d, %X, Start current output.. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
            break;

        case 0x2100:
            if (echo)
            {
                printf("%10d, %X, Start charging process....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.tt.begin_time = CANParaC.tt.OP_time;
                printf("%10d, %X, Start Begin Tiime =  %d ....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.tt.begin_time);

                CANParaC.Icommand_Tmp = 0;
                CANParaC.Chroma.C_Result = true;
                echo = false;
            }

            if (CANParaC.V_Command < CANParaC.LIMIT_VOLT)
            {
                V_command_tmp = V_command_tmp + 0.2;
                CANParaC.V_Command = V_command_tmp;
            }
            if (CANParaC.V_Command > CANParaC.LIMIT_VOLT)
                CANParaC.V_Command = CANParaC.LIMIT_VOLT;

            CANParaC.Icommand_Max = CANParaC.Icommand_Tmp + (CANParaC.slop_p * (CANParaC.tt.now_time - CANParaC.tt.finish_time) * 0.001);
            CANParaC.Icommand_Min = CANParaC.Icommand_Tmp - (CANParaC.slop_n * (CANParaC.tt.now_time - CANParaC.tt.finish_time) * 0.001);

            if ((double)CANParaC.CHG_I > CANParaC.Icommand_Tmp)
                CANParaC.Icommand_Tmp = min(CANParaC.Icommand_Max, (double)CANParaC.CHG_I);
            if ((double)CANParaC.CHG_I < CANParaC.Icommand_Tmp)
                CANParaC.Icommand_Tmp = max((CANParaC.Icommand_Min), (double)CANParaC.CHG_I);

            if (CANParaC.Icommand_Tmp > CANParaC.LIMIT_I)
                CANParaC.Icommand_Tmp = CANParaC.LIMIT_I;
            if (CANParaC.Icommand_Tmp < 0)
                CANParaC.Icommand_Tmp = 0;

            CANParaC.I_Command = CANParaC.Icommand_Tmp;
            printf("%10d, %X, %f %f %f ....\r", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.Icommand_Max, CANParaC.Icommand_Min, CANParaC.I_Command);

            reside_time = CANParaC.LIMIT_TIME - ((CANParaC.tt.OP_time - CANParaC.tt.begin_time) / 100);

            if ((CANParaC.tt.OP_time - CANParaC.tt.begin_time) > 50)
            {
                if (CANParaC.Vdc > CANParaC.LIMIT_VOLT)
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[25] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 1;
                    echo = true;
                    printf("%10d, %X, Reach to full charge voltage....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }
                if (CANParaC.ff.stop_is_pushed)
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[26] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 2;
                    echo = true;
                    printf("%10d, %X, Stop is Pushed.....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }

                if (reside_time <= 0)
                {
                    CANParaC.next_step = 0x3100;
                    reside_time = 0;
                    CANParaC.tt.Error[25] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 3;
                    echo = true;
                    printf("%10d, %X, Charging reside_time count to zero ....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }
                if (!CANParaC.ff.f10250_Vehicle_charging_enable)
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[24] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 4;
                    echo = true;
                    printf("%10d, %X, Vehicle side charging enable(software) disable....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }

                if (!(CANParaC.DIO.DIdata[Perm_j] == DI_ON))
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[24] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 5;
                    echo = true;
                    printf("%10d, %X, Vehicle side charging enable(hardware) disable....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }
                if ((CANParaC.DIO.DIdata[DDEA_1] == DI_OFF))
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.ff.f10951_Charger_error = true;
                    CANParaC.tt.Error[25] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 6;
                    echo = true;
                    printf("%10d, %X, Warning!! DDEA detected.\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                }

                if (CANParaC.CHG_I > CANParaC.LIMIT_I)
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[25] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 8;
                    echo = true;
                    printf("%10d, %X, I command illeage %d....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.CHG_I);
                    break;
                }
                if (CANParaC.CHG_I > CANParaC.CHG_I_old)
                {
                    CANParaC.input_slop_n = false;
                    if ((CANParaC.CHG_I - CANParaC.CHG_I_old) == 3)
                    {
                        if (CANParaC.input_slop_p)
                            CANParaC.input_slop_error = true;
                        CANParaC.input_slop_p = true;
                    }
                    else if ((CANParaC.CHG_I - CANParaC.CHG_I_old) > 3)
                    {
                        CANParaC.input_slop_error = true;
                        CANParaC.input_slop_p = true;
                    }
                    else
                        CANParaC.input_slop_p = false;
                }
                if (CANParaC.CHG_I_old > CANParaC.CHG_I)
                {
                    CANParaC.input_slop_p = false;
                    if ((CANParaC.CHG_I_old - CANParaC.CHG_I) == 3)
                    {
                        if (CANParaC.input_slop_n)
                            CANParaC.input_slop_error = true;
                        CANParaC.input_slop_n = true;
                    }
                    else if ((CANParaC.CHG_I_old - CANParaC.CHG_I) > 3)
                    {
                        CANParaC.input_slop_error = true;
                        CANParaC.input_slop_n = true;
                    }
                    else
                        CANParaC.input_slop_n = false;
                }
                if ((CANParaC.input_slop_error) && (CANParaC.Chroma.C_TaskN == 30))
                {
                    CANParaC.next_step = 0x3100;
                    CANParaC.tt.Error[23] = CANParaC.tt.OP_time;
                    CANParaC.tt.Error[25] = CANParaC.tt.OP_time;
                    CANParaC.charging_stop_code = 7;
                    echo = true;
                    printf("%10d, %X, I command slop too much....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                    break;
                }
            }
            CANParaC.CHG_I_old = CANParaC.CHG_I;
            if (reside_time > 2540)
            {
                CANParaC.REM_TM0 = 0xff;
                CANParaC.REM_TM1 = reside_time / 60;
            }
            else
            {
                CANParaC.REM_TM0 = reside_time / 10;
                CANParaC.REM_TM1 = 0x0;
            }

            break;

        case 0x3100:
            if (echo)
            {
                printf("%10d, %X, Stop process....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.REM_TM0 = 0;
                CANParaC.REM_TM1 = 0;
                if (CANParaC.charging_stop_code == 0)
                    CANParaC.charging_stop_code = 9;
                CANParaC.Icommand_Tmp = 0;
                CANParaC.tt.PT[12] = CANParaC.tt.OP_time - 1;
                echo = false;
            }

            if ((CANParaC.tt.PT[13] == 0) && ((CANParaC.tt.OP_time - CANParaC.tt.PT[12]) > CANParaC.tt.WT[8]))
            {
                CANParaC.ff.f10955_Charging_stop_control = true;
                CANParaC.tt.PT[13] = CANParaC.tt.OP_time;
                CANParaC.I_Command = CANParaC.Icommand_Tmp;
                if (CANParaC.ff.Charger_Malfunction)
                    CANParaC.ff.f10951_Charger_error = true;
                if (CANParaC.ff.Vehicle_Malfunction)
                    CANParaC.ff.f10954_Charging_system_error = true;
            }

            if ((CANParaC.tt.PT[13] > 0) && (CANParaC.tt.can1_output > CANParaC.tt.PT[13]))
            {
                echo = true;
                CANParaC.next_step = 0x3200;
                printf("%10d, %X, Set #109.5.5 = true... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            break;

        case 0x3200:
            if (echo)
            {
                printf("%10d, %X, Stop Charging  current....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }
            if (CANParaC.tt.PT[14] == 0)
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[13]) > CANParaC.tt.WT[9])
                {
                    CANParaC.tt.PT[14] = CANParaC.tt.OP_time;
                    CANParaC.I_Command = 0;
                    CANParaC.Chroma.C_Control = false;
                }
            }

            if (CANParaC.tt.PT[15] == 0)
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[13]) > CANParaC.tt.WT[10])
                {
                    if (CANParaC.Idc <= 5)
                    {
                        CANParaC.tt.PT[15] = CANParaC.tt.OP_time;
                        CANParaC.ff.f10950_Charger_status = false;
                        printf("%10d, %X, Get PT[15] = %d. \r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.tt.PT[15]);
                    }
                }
            }

            if ((CANParaC.tt.PT[14] > 0) && (CANParaC.tt.PT[15] > 0) && (CANParaC.tt.can1_output > CANParaC.tt.PT[15]))
            {
                CANParaC.next_step = 0x3300;
                echo = true;
                printf("%10d, %X, Set #109.5.0 = false... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            break;

        case 0x3300:
            if (echo)
            {
                printf("%10d, %X Tiime for Vehicle relay open....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }

            if ((CANParaC.Vdc < 10) || ((CANParaC.tt.OP_time - CANParaC.tt.PT[15]) > CANParaC.tt.WT[11]))
            {
                CANParaC.next_step = 0x3400;
                CANParaC.tt.PT[16] = CANParaC.tt.OP_time;
                echo = true;
                printf("%10d, %X, DC BUS Voltage < 10V. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            break;

        case 0x3400:

            if (echo)
            {
                printf("%10d, %X Set D1 D2 open....\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.ff.TOT_14_enable = true;

                echo = false;
            }

            if (CANParaC.ff.f10951_Charger_error)
            {
                CANParaC.DIO.DOdata[Relay_d2] = DO_OFF;

                if (CANParaC.tt.PT[19] == 0)
                {
                    CANParaC.tt.PT[19] = CANParaC.tt.OP_time;
                    printf("%10d, %X, Charger side malfunction. D2 open first...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                }
            }

            if (CANParaC.tt.PT[19] > 0)
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[19]) > CANParaC.tt.WT[12])
                {
                    CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                    if (CANParaC.tt.PT[18] == 0)
                        CANParaC.tt.PT[18] = CANParaC.tt.OP_time;
                }
            }
            else if ((CANParaC.tt.OP_time - CANParaC.tt.PT[15]) > CANParaC.tt.WT[11])
            {
                CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                if (CANParaC.tt.PT[18] == 0)
                    CANParaC.tt.PT[18] = CANParaC.tt.OP_time;
                CANParaC.DIO.DOdata[Relay_d2] = DO_OFF;
                if (CANParaC.tt.PT[19] == 0)
                    CANParaC.tt.PT[19] = CANParaC.tt.OP_time;
            }

            if (((CANParaC.tt.OP_time - CANParaC.tt.PT[18]) > 9) && (CANParaC.tt.PT[18] > 0))
            {

                CANParaC.next_step = 0x3500;
                echo = true;
                printf("%10d, %X, D1 D2 OPEN... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            break;

        case 0x3500:
            if (echo)
            {
                printf("%10d, %X, Connect lock release ...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.Chroma.C_Relay = false;
                CANParaC.Chroma.C_Control = false;

                echo = false;
            }
            CANParaC.Chroma.C_Relay = false;
            CANParaC.Chroma.C_Control = false;

            if (CANParaC.Vdc <= 10) // || ((CANParaC.tt.OP_time - CANParaC.tt.PT[18]) > 400))
            {
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[18]) > (CANParaC.tt.WT[13] - 30))
                {
                    CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                    CANParaC.DIO.DOdata[Relay_d2] = DO_OFF;
                    if (CANParaC.tt.PT[18] == 0)
                        CANParaC.tt.PT[18] = CANParaC.tt.OP_time;
                    if (CANParaC.tt.PT[19] == 0)
                        CANParaC.tt.PT[19] = CANParaC.tt.OP_time;
                }
                if ((CANParaC.tt.OP_time - CANParaC.tt.PT[18]) > CANParaC.tt.WT[13])
                {
                    CANParaC.DIO.DOdata[Connect_Lock] = DO_OFF;
                    CANParaC.ff.f10952_Energizing_state = false;

                    if (CANParaC.tt.PT[21] == 0)
                    {
                        CANParaC.tt.PT[20] = CANParaC.tt.OP_time;
                        CANParaC.tt.PT[21] = CANParaC.tt.OP_time;
                    }
                }
            }
            if ((CANParaC.tt.PT[21] > 0) && (CANParaC.tt.can1_output > CANParaC.tt.PT[21]))
            {
                echo = true;
                CANParaC.next_step = 0x3600;
                printf("%10d, %X, Connect lock released %d... GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step, CANParaC.Vdc);
            }
            break;

        case 0x3600:
            if (echo)
            {
                printf("%10d, %X, Waiting for vehicle CAN stop ...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.ff.TOT_15_enable = true;
                echo = false;
            }
            if ((CANParaC.tt.OP_time - CANParaC.tt.PT[21]) > 1200)
            {
                printf("%10d, %X, Check #101,102,103 stop out of time. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.tt.PT[22] = CANParaC.tt.OP_time;
                CANParaC.next_step = 0x3700;
                echo = true;
            }
            else if ((CANParaC.tt.OP_time - CANParaC.tt.last_can_time1 > 100) && (CANParaC.tt.OP_time - CANParaC.tt.last_can_time2 > 100) && (CANParaC.tt.OP_time - CANParaC.tt.last_can_time0 > 100) && ((CANParaC.tt.OP_time - CANParaC.tt.PT[21]) > CANParaC.tt.WT[14]))
            {
                printf("%10d, %X, Vehicle CAN stopped. GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.tt.PT[22] = CANParaC.tt.last_can_time1;
                CANParaC.next_step = 0x3700;
                echo = true;
            }
            break;

        case 0x3700:
            if (echo)
            {
                printf("%10d, %X, Charger side CAN STOP...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.Chroma.C_Control = false;
                echo = false;
            }
            if ((CANParaC.tt.OP_time - CANParaC.tt.PT[22]) > (CANParaC.tt.WT[15] + 200))
            {
                printf("%10d, %X, Charger side CAN stopped. \r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.tt.PT[23] = CANParaC.tt.OP_time;
                CANParaC.eSend = false;

                CANParaC.next_step = 0x3800;
                echo = true;
                printf("%10d, %X, GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }

            break;

        case 0x3800:
            if (echo)
            {
                printf("%10d, %X, Prepare to STOP...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
            }

            if ((CANParaC.tt.OP_time - CANParaC.tt.PT[23]) > CANParaC.tt.WT[5])
            {
                printf("%10d, %X, GOTO Next...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                CANParaC.Chroma.C_IsFinish = true;
                CANParaC.Chroma.C_IsRunning = false;
                CANParaC.ff.CH_SUB_10_enable = true;
                CANParaC.ff.f10955_Charging_stop_control = true;
                CANParaC.ff.f10950_Charger_status = false;
                CANParaC.DIO.D1_12V = false;
                CANParaC.DIO.D1_BRK = false;
                CANParaC.DIO.D2_BRK = false;
                CANParaC.DIO.D2_GND = false;

                CANParaC.waiting_send_result = CANParaC.tt.OP_time;

                CANParaC.Chroma.P_MeasureC = 0;
                CANParaC.Chroma.P_MeasureV = 0;
                CANParaC.next_step = 0x3900;
                echo = true;
            }
            break;

        case 0x3900:
            if (echo)
            {
                printf("%10d, %X, Charging process terminated...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                echo = false;
                CANParaC.tt.Test_start = CANParaC.tt.real_test_start;
                CANParaC.eSend = false;
                CANParaC.Chroma.C_Control = false;
                CANParaC.Chroma.C_Relay = false;
                CANParaC.DIO.DOdata[Connect_Lock] = DO_OFF;
                CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                CANParaC.DIO.DOdata[Relay_d2] = DO_OFF;
                CANParaC.SystemIsWorking = false;
                CANParaC.ff.CH_SUB_10_enable = true;
                CANParaC.ff.f10955_Charging_stop_control = true;
                CANParaC.ff.f10950_Charger_status = false;
                CANParaC.ff.start_is_pushed = false;

                CANParaC.DIO.DOdata[PI_Switch] = DO_OFF;
                CANParaC.DIO.DOdata[GND_Switch] = DO_OFF;
                CANParaC.DIO.D2_GND = false;

                //Get_Result(&CANParaC);
                CANParaC.ff.result_sending = true;
            }

            if ((CANParaC.tt.OP_time - CANParaC.waiting_send_result) > 300)
            {
                CANParaC.ff.result_sending = false;
                CANParaC.tt.Test_Buff_Time = 0;
                CANParaC.tt.Test_start = 0;
                CANParaC.Chroma.C_TaskN = 0;
                if (CANParaC.Chroma.C_Teststep == 1)
                    CANParaC.Chroma.C_Teststep = 2;
                echo = true;
                CANParaC.next_step = 0x4000;
            }
            break;

        case 0x4000:
            if (echo)
            {
                printf("%10d, %X, reset and return to start...\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
                reside_time = 0;
                echo = false;
            }

            if ((CANParaC.tt.OP_time - CANParaC.tt.PT[23]) > (CANParaC.tt.WT[5] + CANParaC.tt.WT[17] + CANParaC.tt.WT[16]) + 300)
            {
                echo = true;
                //mxcan_purge_buffer(CANParaC.hPort1, 0);

                CANParaC.next_step = 0x1000;
            }
            break;
        }

#pragma endregion

        CANParaC.Chroma.RESIDE_TIME = reside_time;
        CANParaC.Chroma.C_ChargingV = CANParaC.V_Command;
        CANParaC.Chroma.C_ChargingC = CANParaC.I_Command;
        CANParaC.POWER8000.SET_V = CANParaC.Chroma.C_ChargingV;
        CANParaC.POWER8000.SET_I = CANParaC.Chroma.C_ChargingC;

        if (CANParaC.Chroma.C_Relay)
        {
            CANParaC.DIO.DOdata[DCIN_1] = DO_ON;
            CANParaC.DIO.DOdata[DCIN_2] = DO_ON;
        }
        else
        {
            CANParaC.DIO.DOdata[DCIN_1] = DO_OFF;
            CANParaC.DIO.DOdata[DCIN_2] = DO_OFF;
        }
        CANParaC.POWER8000.POWER_ON_SET = CANParaC.Chroma.C_Control;

#pragma region TimeOutCheck

        if (CANParaC.ff.TOT_01_enable)
        {
            if (CANParaC.next_step >= 0x1200)
                CANParaC.ff.TOT_01_enable = false;

            else if ((CANParaC.tt.OP_time) > CANParaC.tt.OT[0])
            {
                CANParaC.ff.TOT_01_enable = false;
                CANParaC.ff.TOT_01_check = true;
                CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
                CANParaC.ff.f10954_Charging_system_error = true;
                echo = true;
                printf("%10d, %X, TimeOutTime 01 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }
        if (CANParaC.ff.TOT_04_enable)
        {
            if (CANParaC.next_step >= 0x1400)
                CANParaC.ff.TOT_04_enable = false;
            else if ((CANParaC.tt.OP_time) > CANParaC.tt.OT[1])
            {
                CANParaC.ff.TOT_04_enable = false;
                CANParaC.ff.TOT_04_check = true;
                CANParaC.DIO.DOdata[11] = DO_OFF;
                CANParaC.ff.f10954_Charging_system_error = true;
                echo = true;
                printf("%10d, %X, TimeOutTime 04 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }
        if (CANParaC.ff.TOT_07_enable)
        {
            if (CANParaC.next_step >= 0x1700)
                CANParaC.ff.TOT_07_enable = false;
            else if ((CANParaC.tt.OP_time - CANParaC.tt.PT[7]) > CANParaC.tt.OT[2])
            {
                CANParaC.ff.TOT_07_enable = false;
                CANParaC.ff.TOT_07_check = true;
                CANParaC.ff.f10954_Charging_system_error = true;
                echo = true;
                printf("%10d, %X, TimeOutTime 07 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }
        if (CANParaC.ff.TOT_08_enable)
        {
            if (CANParaC.next_step >= 0x1800)
                CANParaC.ff.TOT_08_enable = false;
            else if ((CANParaC.tt.OP_time - CANParaC.tt.PT[7]) > CANParaC.tt.OT[3])
            {
                CANParaC.ff.TOT_08_enable = false;
                CANParaC.ff.TOT_08_check = true;
                CANParaC.ff.f10954_Charging_system_error = true;
                echo = true;
                printf("%10d, %X, TimeOutTime 08 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }
        if (CANParaC.ff.TOT_14_enable)
        {
            if (CANParaC.next_step >= 0x3600)
                CANParaC.ff.TOT_14_enable = false;
            else if ((CANParaC.tt.OP_time - CANParaC.tt.PT[15]) > CANParaC.tt.OT[4])
            {
                CANParaC.ff.TOT_14_enable = false;
                CANParaC.ff.TOT_14_check = true;
                CANParaC.ff.f10954_Charging_system_error = true;
                echo = true;
                CANParaC.PRE_I = 0;
                CANParaC.PRE_V = 0;
                CANParaC.next_step = 0x3500;
                printf("%10d, %X, TimeOutTime 14 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }
        if (CANParaC.ff.TOT_15_enable)
        {
            if (CANParaC.next_step >= 0x3700)
                CANParaC.ff.TOT_15_enable = false;
            else if ((CANParaC.tt.OP_time - CANParaC.tt.PT[21]) > CANParaC.tt.OT[5])
            {
                CANParaC.ff.TOT_15_enable = false;
                CANParaC.ff.TOT_15_check = true;
                CANParaC.ff.f10954_Charging_system_error = true;
                CANParaC.next_step = 0x3700;
                echo = true;
                printf("%10d, %X, TimeOutTime 15 detected!!\r\n", CANParaC.tt.OP_time, CANParaC.charging_step);
            }
        }

#pragma endregion

        if ((CANParaC.charging_step >= 0x2100) && (CANParaC.charging_step <= 0x3800))
        {
            if ((CANParaC.tt.PT[17] == 0) && (!CANParaC.ff.f10250_Vehicle_charging_enable))
                CANParaC.tt.PT[17] = CANParaC.tt.OP_time;
        }

        if ((CANParaC.charging_step >= 0x1200) && (CANParaC.charging_step <= 0x1800))
        {
            CH_SUB_01(&CANParaC);
        }
        if ((CANParaC.charging_step >= 0x1300) && (CANParaC.charging_step <= 0x1800))
        {
            CH_SUB_01(&CANParaC);
            CH_SUB_02(&CANParaC);
        }
        if ((CANParaC.charging_step >= 0x1100) && (CANParaC.charging_step <= 0x3800))
        {
            CH_SUB_06(&CANParaC);
        }

        if (CANParaC.ff.chag_is_finish)
        {

            CANParaC.Chroma.C_Control = false;
            CANParaC.ff.chag_is_finish = false;
            CANParaC.Chroma.C_Relay = false;
            CANParaC.DIO.DOdata[Relay_d1] = DO_OFF;
            CANParaC.DIO.DOdata[Relay_d2] = DO_OFF;
            if (CANParaC.tt.PT[18] == 0)
                CANParaC.tt.PT[18] = CANParaC.tt.OP_time;
            if (CANParaC.tt.PT[19] == 0)
                CANParaC.tt.PT[19] = CANParaC.tt.OP_time;
            CANParaC.DIO.DOdata[Connect_Lock] = DO_OFF;
            CANParaC.next_step = 0x3500;

            echo = true;
        }

        //DO_write(&CANParaC);
        CANParaC.tt.finish_time = CANParaC.tt.now_time;
        usleep(9000); //Sleep(9);
    }

    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    //讓所有執行緒的執行旗標設定為false
    AllPara.CAN0_write_enable = false;
    AllPara.CAN1_write_enable = false;
    AllPara.CAN0_read_enable = false;
    AllPara.CAN1_read_enable = false;
    AllPara.DIO_moxa5112_enable = false;
    AllPara.io1242_enable = false;
    printf("Set CAN0 Write = %d from main thread\n", AllPara.CAN0_write_enable);
    printf("Set CAN1 Write = %d from main thread\n", AllPara.CAN1_write_enable);
    printf("Set CAN0 Read = %d from main thread\n", AllPara.CAN0_read_enable);
    printf("Set CAN1 Read = %d from main thread\n", AllPara.CAN1_read_enable);
    printf("Set CAN0 Read = %d from main thread\n", AllPara.CAN0_read_enable);
    printf("Set MOXA 5112LX DIO = %d from main thread\n", AllPara.DIO_moxa5112_enable);
    printf("Set io1242_module = %d from main thread\n", AllPara.io1242_enable);
    pthread_cancel(tids[2]);
    pthread_cancel(tids[3]);

    pthread_join(tids[0], NULL);
    pthread_join(tids[1], NULL);
    pthread_join(tids[2], NULL);
    pthread_join(tids[3], NULL);
    pthread_join(tids[4], NULL);
    pthread_join(tids[5], NULL);
    MXEIO_Exit();

    return 0;
}
