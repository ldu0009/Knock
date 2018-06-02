#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "/usr/include/mysql/mysql.h"

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
// # export QUERY_STRING="name=temperature&time=.2&value=33.0"
// # ./dataGet.cgi

MYSQL *conn;
char result123[MAXLINE];

void htmlReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  /* Make the response body */
  // sprintf(content, "%s<html>\r\n<head>\r\n", content);
  // sprintf(content, "%s<title>CGI test result</title>\r\n", content);
  // sprintf(content, "%s</head>\r\n", content);
  // sprintf(content, "%s<body>\r\n", content);
  // sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
  // sprintf(content,"%s<p>Env : %s</p>\r\n", content, buf);
  // ptr = strsep(&buf, "&");
  // while (ptr != NULL){
  //   sprintf(content, "%s%s\r\n", content, ptr);
  //   ptr = strsep(&buf, "&");
  // }
  // sprintf(content, "%s</body>\r\n</html>\r\n", content);
  
  /* Generate the HTTP response */
  printf("HTTP/1.0 200 OK\r\n");
  printf("Content-Length: %d\r\n", strlen(result123));
  printf("Content-Type: text/html\r\n\r\n");
  printf("%s", result123);
  fflush(stdout);
}

void textReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  buf = getenv("QUERY_STRING");
  sprintf(content,"%sEnv : %s\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  
  /* Generate the HTTP response */
  printf("Content-Length: %d\n", strlen(result123));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

void GET_List(void){
  char result1[MAXLINE];
  mysql_query(conn,"select * from sensorList");
  MYSQL_RES *result = mysql_store_result(conn);

  mysql_num_fields(result);

  MYSQL_ROW row;

  while( (row = mysql_fetch_row(result)) )
  {
    sprintf(result1,"%s%s ",result1,row[0]);
  }
  sprintf(result123,"%s\r\n",result1);

mysql_free_result(result);

}


void GET_sensorData(char* name,int num){
  char result1[MAXLINE];
  char temp[MAXLINE];
  mysql_query(conn,"select * from sensorList");
  MYSQL_RES *result = mysql_store_result(conn);

  int id;
  int count;
  int exist=0;
  double value;

  MYSQL_ROW row;
  MYSQL_ROW row2;
while( (row = mysql_fetch_row(result)) )
{
  if(!strcmp(name,row[0])){
    id=atoi(row[1]);
    count=atoi(row[2]);
    exist++;

  }
}


if(exist==0){
  printf("not exist %s sensor\n",name);
  exit(1);
};
count=count-num;
if(count<0)
  printf("not exist %d data\n",num);

sprintf(temp,"select * from sensorData%02d",id);

mysql_query(conn,temp);
result = mysql_store_result(conn);

while( (row2 = mysql_fetch_row(result)) )
{
  int i;
  if(i=(atoi(row2[2]))>count){
    sprintf(result1,"%s%s %s\r\n",result1,row2[0],row2[1]);
    //printf("%s %s\n",row2[0],row2[1]);
  }

}
sprintf(result123,"%s",result1);

 mysql_free_result(result);

}

void get_lnfo(char *name)
{
  char temp[MAXLINE];
  mysql_query(conn,"select * from sensorList");
  MYSQL_RES *result = mysql_store_result(conn);

  int id,count;
  int exist=0;
  double min, max, average;

  MYSQL_ROW row;
  MYSQL_ROW row2;
  while( (row = mysql_fetch_row(result)) )
  {
    if(!strcmp(name,row[0])){
      id=atoi(row[1]);
      count=atoi(row[2]);
      exist++;

    }
  }

  if(exist==0){
    printf("not exist %s sensor\n",name);
    exit(1);
  };

  sprintf(temp,"select * from sensorData%02d",id);

  mysql_query(conn,temp);
  result = mysql_store_result(conn);

  while( (row2 = mysql_fetch_row(result)) )
  {
    int i;
    if(i=atoi(row2[2])==1){
      max=atof(row2[1]);
      min=atof(row2[1]);
    }else
    {
      if(max<atof(row2[1]))
        max=atof(row2[1]);
      if(min>atof(row2[1]))
        min=atof(row2[1]);
    }
    average+=atof(row2[1]);

  }
  sprintf(result123,"id   count    max              average              min\r\n");
  sprintf(result123,"%s%d    %d      %f       %f           %f\r\n",result123,id,count,max,average/count,min);
  mysql_free_result(result);


}


int main(void)
{

  char *server = "localhost";
  char *user = "root";
  char *password = "592482";

  int filedes;

  if(!(conn = mysql_init((MYSQL*)NULL))){
    printf("init fail\n");
    exit(1);
  }

  if(!mysql_real_connect(conn, server, user, password, "OPS", 3306, NULL, 0)){
    printf("connect error\n");
    exit(1);
  }

  char *names, *temp ;
  char *cmp;
  double value;
  rio_t rio;
  char *buf;
  int nums;

  char buf2[100000]={};
  strcpy(buf2,getenv("QUERY_STRING"));
  
  cmp=strtok(buf2,"=");
  if(!strcmp(cmp,"NAME"))
  {
    names= strtok(NULL,"&");
    strtok(NULL,"=");
    temp = strtok(NULL,"\n");
    nums=atoi(temp);

    GET_sensorData(names,nums);
    
  }else if(!strcmp(cmp,"command"))
  {
    temp = strtok(NULL,"&");
    if(!strcmp(temp,"LIST"))     //list
          GET_List();
    else if(!strcmp(temp,"INFO")){  //info
      strtok(NULL,"=");
      temp = strtok(NULL,"\n");
      get_lnfo(temp);
    }

  }

  mysql_close(conn);

  htmlReturn();


  return(0);
}
