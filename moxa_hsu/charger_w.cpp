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
    //擷取程式起始時間
    start = get_now_time();
    usleep(990000);

/* for (int i = 0; i < 100; i++)
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
