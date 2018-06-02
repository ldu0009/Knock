#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/mysql/mysql.h"
#include <sys/types.h>
#include <sys/stat.h>

#define true 1
#define false 0

//
// This program is intended to help you test your web server.
// 
MYSQL *conn;
void DBsdtcreate(char* name){
  int id;
  char* temp[1000];

  DBTsl_init(name);
  id=DBTsl_get(name);

  sprintf(temp,"create table sensorData%02d(time varchar(50), data double,num int auto_increment primary key)",id);

  if(mysql_query(conn, temp)) {
    printf("this table exists. \r\n");
  }

}

void DBTsl_init(char* name)
{
  char msg[1000];
  if(DBTsl_check(name))
  {
  sprintf(msg,"insert sensorList values");
  sprintf(msg,"%s('%s',NULL,%d)",msg,name,0);


  mysql_query(conn,msg);
  }
}

int DBTsl_get(char* name)
{
  mysql_query(conn,"select * from sensorList");
  
  MYSQL_RES *result = mysql_store_result(conn);

  int num_fields = mysql_num_fields(result);

  int i;
  int id;

  MYSQL_ROW row;
while( (row = mysql_fetch_row(result)) )
{
  if(!strcmp(name,row[0])){
    id=atoi(row[1]);
  }
}

mysql_free_result(result);
return id;


}

int DBTsl_check(char* name)
{
  mysql_query(conn,"select * from sensorList");
  
  MYSQL_RES *result = mysql_store_result(conn);

  int num_fields = mysql_num_fields(result);

  int i;

  MYSQL_ROW row;
while( (row = mysql_fetch_row(result)) )
{
  if(!strcmp(name,row[0])){

    return false;
  }
}

mysql_free_result(result);
return true;


}

void DBTsl_set(char* name)
{
  mysql_query(conn,"select * from sensorList");
  
  MYSQL_RES *result = mysql_store_result(conn);

  mysql_num_fields(result);

  int count;
  char *temp[1000];

  MYSQL_ROW row;
while( (row = mysql_fetch_row(result)) )
{
  if(!strcmp(name,row[0])){
    count=atoi(row[2]);
    count++;
    sprintf(temp,"update sensorList set count='%d' where name='%s'",count,name);
    mysql_query(conn,temp);
  }
}
mysql_free_result(result);


}

void DBTsd_init(char* name,char* time,double value){
  int id;
  char temp[1000];
  
  id=DBTsl_get(name);

  sprintf(temp,"INSERT INTO sensorData%02d  value('%s',%f,NULL)",id,time,value);

  mysql_query(conn,temp);

  DBTsl_set(name);

  //this change sltable count

}


int main(int argc, char *argv[])
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

  char *name, *temp , *time;
  double value;
  rio_t rio;
  char *buf;
  buf=getenv("CONTENT_LENGTH");
  int length=atoi(buf);
  char size[100000]={};
  char restbody[100000];
  char buf2[100000]={};
  strcpy(buf2,getenv("REQUEST_BODY"));
  Rio_readinitb(&rio, STDIN_FILENO);

  if(strlen(buf2)<=length)
    Rio_readnb(&rio, restbody, length-strlen(buf2));

  sprintf(buf2, "%s%s", buf2, restbody);
  sprintf(size,"%d",strlen(buf2));

  if((filedes = open("./fifo", O_RDWR)) < 0 ) {
          printf("DataPost.c fail open fifo");
          exit(1);
   }

   if(write(filedes, size,MAXLINE)==-1){
               printf("fail to call write()\n");
               exit(1);
    }



   if(write(filedes, buf2, MAXLINE)==-1){
               printf("fail to call write()\n");
               exit(1);
    }

   close(filedes);


  strtok(buf2 , "=");
  name = strtok(NULL,"&");
  strtok(NULL,"=");
  time = strtok(NULL,"&");
  strtok(NULL,"=");
  temp = strtok(NULL , "&");
  value=atof(temp);


  DBsdtcreate(name);
  DBTsd_init(name,time,value);
  mysql_close(conn);

  printf("HTTP/1.0 200 OK\r\n");
  printf("Server: My Web Server\r\n");
  printf("Content-Length: %d\r\n", length);
  printf("Content-Type: text/plain\r\n");
  fflush(stdout);

  return(0);

}
