/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename, char *body, int * port)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s:%d\n", buf, port);
  sprintf(buf, "%sConnection: keep-alive\n", buf);
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
  clientSend(clientfd, filename, msg,&port);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[], float *threshold)
{
  FILE *fp;

  fp = fopen("config-pc.txt", "r");
  if (fp == NULL)
    unix_error("config-pc.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fscanf(fp, "%f", threshold);
  fclose(fp);
}

int main(void)
{
  char *name, *temp , *time;
  char hostname[MAXLINE], webaddr[MAXLINE];
  float threshold,value;
  int port;
  
  getargs_cg(hostname, &port, webaddr, &threshold);


  int counter = 0;
  int  fd;
  char  msg[MAXLINE];
  char  msg2[MAXLINE];
  int nread;
  int size;

  if(mkfifo("./fifo", 0666) == -1) {
          printf("fail mkfifo\n");
          exit(1);
    }


  if((fd = open("./fifo", O_RDWR)) < 0 ) {
          printf("fail open fifo");
          exit(1);
   }

   while(counter!=1){
      read(fd,msg,MAXLINE);
      counter++;
   }
   size=atoi(msg);
   read(fd,msg2,size);

strtok(msg2 , "=");
  name = strtok(NULL,"&");
  strtok(NULL,"=");
  time = strtok(NULL,"&");
  strtok(NULL,"=");
  temp = strtok(NULL , "&");
  value=atof(temp);

  unlink("./fifo");
  
  if(value>threshold)
  {
    userTask(name, hostname, port, webaddr, time, value);
  }


  
  return(0);
}
