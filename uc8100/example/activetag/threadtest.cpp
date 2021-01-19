#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define ONE_SECOND  1000000

//----------------------------------------------------------
class CMoxaIO
{
    public:
	    CMoxaIO(void);
	    virtual ~CMoxaIO();
	    void increment_count();
	    long long get_count();
	    static void* ACTTAG_ModbusTcp( void * lParam );
        //
        pthread_mutex_t m_count_mutex;
        long long m_count;
};
//----------------------------------------------------------
CMoxaIO::CMoxaIO(void)
    :m_count(100)
{
    printf("CMoxaIO() Enter m_count_mutex Addr=%p, m_count_mutex=%d\r\n", &m_count_mutex, m_count_mutex);
    pthread_mutex_init(&m_count_mutex, NULL);   
    printf("CMoxaIO() Exit m_count_mutex Addr=%p, m_count_mutex=%d\r\n", &m_count_mutex, m_count_mutex);
}
//----------------------------------------------------------
CMoxaIO::~CMoxaIO(void)
{
}
//----------------------------------------------------------
void CMoxaIO::increment_count()
{
    printf("increment_count() Enter\r\n");
	pthread_mutex_lock(&m_count_mutex);
    m_count += 1;
	pthread_mutex_unlock(&m_count_mutex);
}
//----------------------------------------------------------
long long CMoxaIO::get_count()
{
    long long c = 0;
    printf("get_count() Enter\r\n");
    pthread_mutex_lock(&m_count_mutex);
	printf("pthread_mutex_lock() m_count_mutex Addr=%p, m_count_mutex=%d\r\n", &m_count_mutex, m_count_mutex);
	c = m_count;
    pthread_mutex_unlock(&m_count_mutex);
    printf("pthread_mutex_unlock() m_count_mutex Addr=%p, m_count_mutex=%d\r\n", &m_count_mutex, m_count_mutex);
    return (c);
}
//----------------------------------------------------------
void* CMoxaIO::ACTTAG_ModbusTcp( void * lParam )
{
    printf("ACTTAG_ModbusTcp() Enter\r\n");
    CMoxaIO * io = (CMoxaIO *)lParam;
    printf("m_count = %d\r\n", io->get_count());            
    int i=0;
    for(i=0; i < 10; i++)
    {        
        io->increment_count();
        printf("loop [%d] m_count = %d\r\n", i, io->get_count());        
        usleep(ONE_SECOND);
    }
    //return((void *)0);
}


//----------------------------------------------------------
static void* TestThread( void * lParam )
{
    printf("TestThread() Enter\r\n");
    //return((void *)0);
}

int main()
{
    printf("Main() Enter\r\n");
    CMoxaIO MoxaIO;     
    /*printf("m_count = %d\r\n", get_count());            
    int i=0;
    for(i=0; i < 50; i++)
    {        
        increment_count();
        printf("loop [%d] m_count = %d\r\n", i, get_count());        
        usleep(ONE_SECOND);
    }*/
    printf("m_count = %d\r\n", MoxaIO.get_count());            
    int i=0;
    for(i=0; i < 3; i++)
    {        
        MoxaIO.increment_count();
        printf("loop [%d] m_count = %d\r\n", i, MoxaIO.get_count());        
        usleep(ONE_SECOND);
    }
     
    pthread_t tid_ModbusTcp = 0;// = pthread_self();
    printf("Call pthread_create(), MoxaIO=%p, CMoxaIO::ACTTAG_ModbusTcp=%p\r\n", &MoxaIO, CMoxaIO::ACTTAG_ModbusTcp);
    int iRet = pthread_create( &tid_ModbusTcp, NULL, CMoxaIO::ACTTAG_ModbusTcp, (void *)&MoxaIO );
    //int iRet = pthread_create( &tid_ModbusTcp, NULL, TestThread, NULL );
    printf("Call pthread_create() return %d\r\n", iRet);
    //
    void *res;
    iRet = pthread_join(tid_ModbusTcp, &res);
    if (iRet != 0)
    {
        printf("Joined with thread %d; returned value was %s\n", tid_ModbusTcp, (char *) res);
        free(res);
    }
    else
        printf("Joined with thread %d; Success.\n", tid_ModbusTcp);
    return 0;
}
  