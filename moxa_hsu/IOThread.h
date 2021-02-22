#include "CHAdeMO.h"

//計算兩個時間差的函數，回傳回秒數，但必須注意傳過來的時間格式
double get_now_time(void) {
  struct timespec temp;
  clock_gettime(CLOCK_MONOTONIC, &temp); 
  double time_result;
  time_result = temp.tv_sec + (double) temp.tv_nsec / 1000000000.0; 
  return time_result;
}

int CANPORT_DEF(int can_port)
{  
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s;
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW); //建立 SocketCAN 端口

    //選擇can0或can1
    char const *ifname0 = "can0";    
    char const *ifname1 = "can1"; 
    if(can_port == 0)
    {
      strcpy(ifr.ifr_name, ifname0);
    }
    else if(can_port == 1) 
    {  
      strcpy(ifr.ifr_name, ifname1); 
    }
 
    ioctl(s, SIOCGIFINDEX, &ifr); //指定 can 裝置
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr)); //將端口與 can0 繫結    
    return s;      
}


void *canwrite0(void *args)
{
    printf("CAN0 write ready！\n");
  	PCAN_PARA pstPara = (PCAN_PARA) args;
    struct can_frame frame;
    int ret;
    while (pstPara->CAN0_write_enable) //要送的資料程式寫在這回圈裡面
    {
      //無法開一長串空間連續傳送，故每一筆分開傳送，會造成每筆時間間隔約0.0004秒
        frame.can_id = 0x500;
        frame.can_dlc = 8;
        frame.data[0] = pstPara->time_tmp;
        frame.data[1] = 0x00;
        frame.data[2] = 0x00;  
        frame.data[3] = 0x00; 
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;
        ret = write(pstPara->CANPort_0, &frame, sizeof(struct can_frame));

        frame.can_id = 0x501;
        frame.can_dlc = 8;
        frame.data[0] = 0x00;
        frame.data[1] = pstPara->time_tmp;
        frame.data[2] = 0x00;  
        frame.data[3] = 0x00; 
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;
        ret = write(pstPara->CANPort_0, &frame, sizeof(struct can_frame));        

        usleep(99900);
    }
    pthread_exit(NULL); 
}

void *canwrite1(void *args)
{
    printf("CAN1 write ready！\n");
  	PCAN_PARA pstPara = (PCAN_PARA) args;
    struct can_frame frame;
    int ret;
    while (pstPara->CAN1_write_enable) //要送的資料程式寫在這回圈裡面
    {
      //無法開一長串空間連續傳送，故每一筆分開傳送，會造成每筆時間間隔約0.0004秒

        frame.can_id = 0x100;
        frame.can_dlc = 8;
        frame.data[0] = 0x00;
        frame.data[1] = 0x00;
        frame.data[2] = pstPara->time_tmp;  
        frame.data[3] = 0x00; 
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;
        ret = write(pstPara->CANPort_1, &frame, sizeof(struct can_frame));

        frame.can_id = 0x109;
        frame.can_dlc = 8;
        frame.data[0] = 0x00;
        frame.data[1] = 0x11;
        frame.data[2] = 0x00;  
        frame.data[3] = pstPara->time_tmp; 
        frame.data[4] = 0xFF;
        frame.data[5] = 0xFF;
        frame.data[6] = 0xFF;
        frame.data[7] = 0xFF;
        ret = write(pstPara->CANPort_1, &frame, sizeof(struct can_frame));        


        usleep(99900);
    }
    pthread_exit(NULL); 
}

void *canread0(void *args)
{
    printf("CAN0 read ready！\n");
  	PCAN_PARA pstPara = (PCAN_PARA) args;
    struct can_frame frame;
    int ret, count_temp;
    double now_time;

    while (pstPara->CAN0_read_enable)
    {
      ret = read(pstPara->CANPort_0, &frame, sizeof(struct can_frame));
      now_time = get_now_time();
    //===========================================================
    //這個區域撰寫每次讀到的CAN資料轉換成要使用的變數與旗標

        printf("CAN0 received: %010.4f %03x [%d] ", now_time, frame.can_id, frame.can_dlc);
        for (int i = 0; i < frame.can_dlc; i++) printf(" %02x", frame.data[i]);
        printf("\n");     
        
    //============================================================

    //每讀10筆資料休息0.1秒
      count_temp++;
      if(count_temp >10)
      {
        count_temp = 0;
        usleep(9900); 
      }

    }
    pthread_exit(NULL); 
}

void *canread1(void *args)
{
    printf("CAN1 read ready！\n");
  
  	PCAN_PARA pstPara = (PCAN_PARA) args;
    struct can_frame frame;
    int ret, count_temp;
    double now_time;
 
    while (pstPara->CAN1_read_enable)
    {
      ret = read(pstPara->CANPort_1, &frame, sizeof(struct can_frame));
      now_time = get_now_time();

    //===========================================================
    //這個區域撰寫每次讀到的CAN資料轉換成要使用的變數與旗標

        printf("CAN0 received: %010.4f %03x [%d] ", now_time, frame.can_id, frame.can_dlc);
        for (int i = 0; i < frame.can_dlc; i++) printf(" %02x", frame.data[i]);
        printf("\n");     
        
    //============================================================
    //每讀10筆資料休息0.1秒
      count_temp++;
      if(count_temp >10)
      {
        count_temp = 0;
        usleep(9900); 
      }

    }
    pthread_exit(NULL); 
}
