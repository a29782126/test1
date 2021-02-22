#include "IOThread.h"
#include "CHAdeMO.h"

//// 主線程
int main()
{
    pthread_t tids[4];  

    //時間變數宣告
    double start, end;
    double time_used;

    CAN_PARA AllPara;
    AllPara.CAN0_write_enable = true;
    AllPara.CAN1_write_enable = true;   
    AllPara.CAN0_read_enable = true;
    AllPara.CAN1_read_enable = true;   

    AllPara.CANPort_0 = CANPORT_DEF(0);
    AllPara.CANPort_1 = CANPORT_DEF(1);

    pthread_create(&tids[0], NULL, canwrite0, &AllPara);   
    pthread_create(&tids[1], NULL, canwrite1, &AllPara);        
    pthread_create(&tids[2], NULL, canread0, &AllPara);   
    pthread_create(&tids[3], NULL, canread1, &AllPara);        
    //擷取程式起始時間
    start = get_now_time();
  
    for(int i = 0; i<100; i++)
    {
        usleep(99000);        

        //擷取目前時間       
        end = get_now_time();

        //計算時間差並列印
        time_used = end -start;
        AllPara.time_tmp = time_used;
        printf("Time %d = %f\n", i+1, time_used);     
    
    }


    //讓所有執行緒的執行旗標設定為false
    AllPara.CAN0_write_enable = false;
    AllPara.CAN1_write_enable = false;    
    AllPara.CAN0_read_enable = false;
    AllPara.CAN1_read_enable = true;      
    printf("Set CAN0 Write = %d from main thread\n", AllPara.CAN0_write_enable);   
    printf("Set CAN1 Write = %d from main thread\n", AllPara.CAN1_write_enable);  
    printf("Set CAN0 Read = %d from main thread\n", AllPara.CAN0_read_enable);   
    printf("Set CAN1 Read = %d from main thread\n", AllPara.CAN1_read_enable);      

    pthread_cancel(tids[2]);
    pthread_cancel(tids[3]);
    
    pthread_join(tids[0], NULL);  
    pthread_join(tids[1], NULL);
    pthread_join(tids[2], NULL);  
    pthread_join(tids[3], NULL); 

    return 0;
}