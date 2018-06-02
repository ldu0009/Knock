#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"
#include <ctype.h>
#include <string.h>
#define MAX_STR_SIZE 100

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
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
    //printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      //printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{

  char hostname[MAXLINE], webaddr[MAXLINE],webaddr2[MAXLINE];
  int port;
  char ch=NULL;
  char cmd[MAX_STR_SIZE];
  char cmp[MAX_STR_SIZE];
  char *r1;
  char *r2;
  char *r3;
  char *tmp;

  getargs_cg(hostname, &port, webaddr2);

  printf("If you want to see commands, type 'help'\n"); 
  pid_t pid = Fork();
  if(pid==0){
       Execve("./pushServer", (NULL) , environ);
  }
  else
  {
    while(1){
    strcpy(webaddr,webaddr2);

    printf(">> ");
    fgets(cmd,MAX_STR_SIZE, stdin);

    if (!strcmp(cmd , "quit\n") ) break;
    if (!strcmp(cmd , "help\n")) {
      printf("List             : print current sensor list\n");
      printf("INFO <sname>    : print <sname> sensor information\n");
      printf("GET <sname>     : print <sname> table recent data \n");
      printf("GET <sname> <n> : print <sname> table recent <n> data \n");
      printf("quit             : quit the program\n");
    }

    r1=strtok(cmd," ");
    r2=strtok(NULL, " ");
    r3=strtok(NULL, "");

    tmp = r1;
    
    while(*tmp){
        if(*tmp>=65 && *tmp <= 95) *tmp +=32;
        tmp++;
    }
    if (!strcmp(r1, "list\n")){
      sprintf(webaddr,"%scommand=LIST",webaddr);
      userTask(hostname, port, webaddr);
        
    }
  
    if(!strcmp(r1 , "info")&&!r2==NULL){
       r2=strtok(r2,"\n");
       sprintf(webaddr,"%scommand=INFO&value=%s",webaddr,r2);
       userTask(hostname, port, webaddr);
    }
  
    if(!strcmp(r1 , "get")&&!r2==NULL){
       if(r3==NULL){
        r2=strtok(r2,"\n");
        sprintf(webaddr,"%sNAME=%s&N=1",webaddr,r2);
        userTask(hostname, port, webaddr);
       }
       else{
        r3=strtok(r3,"\n");
        sprintf(webaddr,"%sNAME=%s&N=%d",webaddr,r2,atoi(r3));
        userTask(hostname, port, webaddr);
       }
    }

    if(!strcmp(r1 , "quit\n")){
      break;
    }
  
    
    
  }
    kill(pid, SIGINT);
  }
  
  return(0);
}