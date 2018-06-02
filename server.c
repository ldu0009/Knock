#include "stems.h"
#include "request.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

// 
// To run:
// 1. Edit config-ws.txt with following contents
//    <port number>
// 2. Run by typing executable file
//    ./server
// Most of the work is done within routines written in request.c
#define TRUE 1
#define FALSE 0

struct timeval startTime;


void initwatch(void)
{
  gettimeofday(&startTime, NULL);
}


typedef struct _Data
{
   int fd;
   int time;

}Data;



 
typedef struct _cQueue
{
    int front; // 큐의 시작점 F
    int rear; // 큐의 끝점 R
    Data *queArr;
} CQueue;
 
typedef CQueue Queue; // Cqueue 구조체를 Queue로 선언한다.

int thread_num=0;
int queue_num=0;

Queue q;

Queue queue;
sem_t s;
sem_t full;
sem_t empty;
sem_t k;


void QueueInit(Queue *pq) // 텅 빈 경우 front와  rear은 동일위치를 가리킨다.
{
    pq->front = 0;  // 초기화 과정
    pq->rear = 0;
    pq->queArr=(Data*)malloc(sizeof(Data)*queue_num);


}
 
int QIsEmpty(Queue *pq)
{
    if (pq->front == pq->rear) // 큐가 텅 비었다면,
        return TRUE;
 
    else
        return FALSE;
}
 
int NextPosIdx(int pos) // 큐의 다음 위치에 해당하는 인덱스 값 반환
{
    if (pos == queue_num - 1) //  만약 현재위치가 큐길이 - 1이라면 // 즉 배열의 마지막 요소의 인덱스 값이라면
        return 0; // 0을 반환(큐의 끝에 도달했으므로 회전을 돕는 함수이다)
 
    else
        return pos + 1; // 그외에는 다음 큐를 가리키도록
}
 
void Enqueue(Queue *pq, Data data)
{
    if (NextPosIdx(pq->rear) == pq->front) // 큐가 꽉 찼다면,
    {
        printf("Queue Memory full!\n"); // 여기 접근을 할 수 없는 코드인데 접근한다면 에러구문 표시 후 종료
        exit(-1); // 즉, 원형 큐에서는 전체크기 - 1을 기준으로 F,R이 만나지 않게되는 알고리즘인데 F==Q가 될 수 없다.
    }
    pq->rear = NextPosIdx(pq->rear); // rear을 한 칸 이동
    pq->queArr[pq->rear] = data; // rear이 가리키는 곳에 데이터 저장

}
 
Data Dequeue(Queue *pq)
{
    if (QIsEmpty(pq)) // 아무것도 없는 상태에서 큐에 있는 데이터를 뺀다는 것은 오류.
    {
        printf("Queue Memory empty!\n");
        exit(-1);
    }
 
    pq->front = NextPosIdx(pq->front); // front를 한 칸 이동한다
    return pq->queArr[pq->front];  // front가 가리키는 데이터를 반환한다.
}
 
Data QPeek(Queue *pq)          
{
    if (QIsEmpty(pq))
    {
        printf("Queue Memory Error!");
        exit(-1);
    }
 
    return pq->queArr[NextPosIdx(pq->front)];
}

void getargs_ws(int *port,int *thread_num,int *queue_num)
{
  FILE *fp;

  if ((fp = fopen("config-ws.txt", "r")) == NULL)
    unix_error("config-ws.txt file does not open.");

  fscanf(fp, "%d", port);
  fscanf(fp, "%d", thread_num);
  fscanf(fp, "%d", queue_num);
  fclose(fp);
}

void consumer(int connfd, long arrivalTime)
{
  requestHandle(connfd, arrivalTime);
  Close(connfd);
}

double getwatch(void)
{
  struct timeval curTime;
  double tmp;

  gettimeofday(&curTime, NULL);
  
  tmp = (curTime.tv_sec - startTime.tv_sec) * 1000.0;
  
  return curTime.tv_sec - startTime.tv_sec + ((curTime.tv_usec - startTime.tv_usec) * 0.000001);
}

void *worker_thread(void *connfd)
{
  printf("start worker_thread\n");
   Data num;
  
  while(1)
  {
   sem_wait(&empty);
   sem_wait(&k);
   num=Dequeue(&q);
   sem_post(&k);
   sem_post(&full);
   printf("%d\n",num.fd);
   consumer(num.fd , num.time);      // request handle(GET or POST)

   char *method=getenv("REQUEST_METHOD");
   if(!strcmp(method,"POST")){
      pid_t pid1=Fork();
      if(pid1==0){
        Execve("./clientAlarm", (NULL) , (NULL));
      }
    }
  }
}


int main(void)
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  pthread_t *threads;  
  pthread_attr_t attr;
  Data data;
  data.fd=0;
  data.time=0;



  initWatch();
  getargs_ws(&port,&thread_num,&queue_num);

  sem_init(&full,0,queue_num);
  sem_init(&empty,0,0);
  sem_init(&k,0,1);

  if( pthread_attr_init(&attr) != 0 )       
    return 1;
  
  if( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 )  
    return -1;

  QueueInit(&q);
  threads=(pthread_t*)malloc(sizeof(pthread_t)*thread_num);

  int i;
  for(i = 0; i < thread_num; i++)                 
      pthread_create(&threads[i], &attr, worker_thread, 0);  

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    printf("port: %d\n",listenfd);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    
   data.fd=connfd;
   data.time=getwatch();

    sem_wait(&full);
    sem_wait(&k);
    Enqueue(&q,data);
    sem_post(&k);
    sem_post(&empty);

  }
  return(0);
}