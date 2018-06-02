#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi


int main(void)
{

  char *name, *temp , *time;
  double value;
  rio_t rio;
  char *buf;
  buf=getenv("CONTENT_LENGTH");
  int length=atoi(buf);


  char restbody[100000];

  char buf2[100000]={};
  strcpy(buf2,getenv("REQUEST_BODY"));
  Rio_readinitb(&rio, STDIN_FILENO);

  if(strlen(buf2)<=length)
     Rio_readnb(&rio, restbody, length-strlen(buf2));

  sprintf(buf2, "%s%s", buf2, restbody);

  strtok(buf2 , "=");
  name = strtok(NULL,"&");
  strtok(NULL,"=");
  time = strtok(NULL,"&");
  strtok(NULL,"=");
  temp = strtok(NULL , "&");
  value=atof(temp);

  printf("200 from alarm.cgi \r\n");
  
  fprintf(stderr,"\r\n경고: %s sensor로부터 %s시각에 %f라는 값이 발생했습니다. \n>>",name,time,value);

  return(0);
}
