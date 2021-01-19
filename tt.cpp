#include <stdio.h>
#include <pthread.h>

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

int main()
{
   // 定义线程的 id 变量，多个变量使用数组
   pthread_t tids[NUM_THREADS];

   //参数依次是：创建的线程id，线程参数，调用的函数，传入的函数参数
   pthread_create(&tids[0], NULL, say_hello0, NULL);

   pthread_create(&tids[1], NULL, say_hello1, NULL);

   pthread_create(&tids[2], NULL, say_hello2, NULL);

   //等各个线程退出后，进程才结束，否则进程强制结束了，线程可能还没反应过来；
   pthread_exit(NULL);
}