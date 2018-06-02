/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"

#define MAX_STR_SIZE 100

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
   rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    /* If you want to look for certain HTTP tags... */
    sscanf(buf, "Content-Length: %d ", &length);
    if (sscanf(buf, "Content-Length: %d ", &length) == 1);
     //printf("Length = %d\n", length);
   //printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, char* time, float value)
{
  int clientfd;
  char msg[MAXLINE];


  sprintf(msg, "name=%s&time=%s&value=%f ", myname, time, value);
  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, char *time, float *value)
{
  FILE *fp;

  fp = fopen("config-cp.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fscanf(fp, "%s", myname);
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%s", time);
  fscanf(fp, "%f", value);
  fclose(fp);
}

int main(void)
{
    FILE *fp;
  time_t now;
  struct tm *t;
  now = time(NULL);
  t  =localtime(&now);
  char times[1024]={};
  char month[12][4] = {"Jan" , "Feb" , "Mar" , "Apr" , "May" , "Jun" , "Jul" , "Aug" , "Sep" , "Oct" , "Nov" , "Dec"};
  char day[12][4] = {"Sun" , "Mon" , "Tye","Wed" , "Thu" , "Fri" ,"Sat"};
  
  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
  int port;
  float value;

  char cmd[MAX_STR_SIZE];
  char cmp[MAX_STR_SIZE];
  char *r1;
  char *r2;
  getargs_cp(myname, hostname, &port, filename, &times, &value);

   
  printf("If you want to see commands, type 'help'\n"); 
  while(1){
    printf(">> ");
    fgets(cmd,MAX_STR_SIZE, stdin);

    if (!strcmp(cmd , "quit\n") ) break;
    if (!strcmp(cmd , "help\n")) {
      printf("name            : print current sensor name\n");
      printf("name <sensor> : change sensor name to <sensor>\n");
      printf("value           : print current value of sensor\n");
      printf("value <n>       : set sensor value to <n> \n");
      printf("send             : send (current sensor name , time , value) to server\n");
      printf("quit             : quit the program\n");
    }

    r1=strtok(cmd," ");
    r2=strtok(NULL, "");

   
    if (!strcmp(r1 , "name\n")){
        printf("Current sensor is '%s'\n" , myname);
    }
    if(!strcmp(r1 , "name")&&!r2==NULL){
       r2=strtok(r2,"\n");
       strcpy(myname  , r2);
       printf("Sensor name is changed to '%s'\n",myname);
    }
    if(!strcmp(r1 , "value\n")){
      printf("Current value of sensor is '%f'\n",value);
    }
    if(!strcmp(r1 , "value")&&!r2==NULL){
      value=atof(r2);
      printf("Sensor value is changed to '%f'\n",value);
    }
    if(!strcmp(r1 , "send\n")){
      sprintf(times,"%s/%d/%d:%d",month[t->tm_mon], t->tm_mday , t->tm_hour , t->tm_min);
      printf("..sending name=%s&time=%s&value=%f\n",myname,times,value);
      userTask(myname, hostname, port, filename, times, value);
    }
    if(!strcmp(r1 , "quit\n")){
      break;
    }
    
    
  }

  fp = fopen("config-cp.txt", "w");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fprintf(fp, "%s\r\n", myname);
  fprintf(fp, "%s\r\n", hostname);
  fprintf(fp, "%d\r\n", port);
  fprintf(fp, "%s\r\n", filename);
  fprintf(fp, "%s\r\n", times);
  fprintf(fp, "%f\r\n", value);
  fclose(fp);

  
  
  
  return(0);
}
