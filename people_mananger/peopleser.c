#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sqlite3.h>
#include <pthread.h>
#include <signal.h>
#include "peopleser.h"
#include <arpa/inet.h>
#define MESSAGE(msg) do{\
	perror(msg);\
	printf("__%d__",__LINE__);\
}while(0)

#define IP "192.168.1.23"
#define PORT 8888
char name[20]="";

int main()
{
	//初始化数据库
	sqlite3 *staff_db =NULL;
	if(init_sqlite(&staff_db)<0)
	{
		printf("初始化数据库失败\n");
		return -1;
	}
	//网络初始化
	int fd=0;
	if(init_socket(&fd)<0)
	{
		printf("init_socket error\n");
		return -1;
	}

	//多线程处理，主线程管理连接，分线程管理通信
	int newfd;
	struct sockaddr_in cin;
	socklen_t size=sizeof(cin);
	while(1)
	{
		newfd=accept(fd,(struct sockaddr *)&cin,&size);
		if(newfd < 0)
		{
			MESSAGE("accept");
			return -1;
		}
		printf("连接成功----fd:%d\n",newfd);
		//创建线程
		staff staff_ser ={newfd,cin,staff_db};
		pthread_t tid;
		if(pthread_create(&tid,NULL,handler_staff_ser,&staff_ser)!=0)
		{
			MESSAGE("pthread_create");
			return -1;
		}

	}
	return 0;
}

void* handler_staff_ser(void *arg)
{
	//线程分离
	pthread_detach(pthread_self());

	staff staff_ser =*(staff *)arg;
	int cfd=staff_ser.fd;
	struct sockaddr_in cin =staff_ser.cin;
	sqlite3 *staff_db =staff_ser.staff_db;

	//接受消息:
	int res=0;
	struct people staff_msg;
	while(1)
	{
		res=recv(cfd,&staff_msg,sizeof(staff_msg),0);
		if(res == -1)
		{
			MESSAGE("recv");
			break;
		}
		else if(res == 0)
		{
			printf("对方关闭\n");
			break;
		}
		char type =staff_msg.type;
		switch(type)
		{
		case 'z':
			//注册
			do_zuce(cfd,staff_msg,staff_db);
			break;
		case 'l':
			//登录
			do_log(cfd,staff_msg,staff_db);
			break;
		case 'd':
			//删除
			do_del(cfd,staff_msg,staff_db);
			break;
		case 'a':
			//增加
			do_add(cfd,staff_msg,staff_db);
			break;
		case 'm':
			//修改
			do_modify(cfd,staff_msg,staff_db);
			break;
		case 's':
			//查找
			do_search(cfd,staff_msg,staff_db);
			break;
		case 'q':
			//退出
			do_quit(cfd,staff_msg,staff_db);
			pthread_exit(NULL);
		}

	}
	sqlite3_close(staff_db);
	pthread_exit(NULL);
}

int do_search(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	char ret[10];
	char sql[256]="";
	char *errmsg=NULL;
	int line=0, list=0;
	char **pres=NULL;
	int row,column;
	bzero(sql,256);
	sprintf(sql,"select * from staff where number=\"%s\"",staff_msg.number);
	if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
	{
		printf("用户未注册\n");
		return -1;
	}
	if(row == 0)
	{
		strcpy(staff_msg.text,"notexits");
		if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
		{
			MESSAGE("send");
			return -1;
		}
	}
	else
	{
		if(strcmp(name,staff_msg.number)==0)
		{
			sprintf(sql,"select name, sex,age,address,salary,phone from staff where number=\"%s\"",staff_msg.number);
			if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
			{
				printf("没有该员工\n");
				return -1;
			}
			bzero(staff_msg.text, sizeof(staff_msg.text));
			int i = 1, j=column; //不需要字段名
			for(i=1; i<row+1; i++)
			{
				j=i*column;
				sprintf(staff_msg.text, "name:%s sex:%s  age:%s address:%s salary:%s phone:%s", pres[j], pres[j+1], pres[j+2],pres[j+3],pres[j+4],pres[j+5]);
			}
			if(send(cfd, &staff_msg, sizeof(staff_msg), 0) <0)
			{
				MESSAGE("send");
				return -1;
			}

		}
		else if(staff_msg.mod =='p')
		{
			sprintf(sql,"select name, sex,age,phone from staff where number=\"%s\"",staff_msg.number);
			if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
			{
				printf("没有该员工\n");
				return -1;
			}
			bzero(staff_msg.text, sizeof(staff_msg.text));
			int i = 1, j=column; //不需要字段名
			for(i=1; i<row+1; i++)
			{
				j=i*column;
				sprintf(staff_msg.text, "name:%s sex:%s  age:%s phone:%s", pres[j], pres[j+1], pres[j+2],pres[j+3]);
			}
			if(send(cfd, &staff_msg, sizeof(staff_msg), 0) <0)
			{
				MESSAGE("send");
				return -1;
			}
		}
		else if(staff_msg.mod =='r')
		{
			sprintf(sql,"select name, sex,age,address,salary,phone from staff where number=\"%s\"",staff_msg.number);
			if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
			{
				printf("没有该员工\n");
				return -1;
			}
			bzero(staff_msg.text, sizeof(staff_msg.text));
			int i = 0, j=column; 	
			for(i=1; i<row+1; i++)
			{
				j=i*column;
				sprintf(staff_msg.text, "name:%s sex:%s age:%s address:%s salary:%s phone:%s",\
						pres[j], pres[j+1], pres[j+2],pres[j+3],pres[j+4],pres[j+5]);
			}
			if(send(cfd, &staff_msg, sizeof(staff_msg), 0) <0)
			{
				MESSAGE("send");
				return -1;
			}

		}
	}
	sqlite3_free_table(pres);
	return 0;

}


int do_modify(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	int ret;
	char sql[256]="";
	char *errmsg=NULL;
	char **pres=NULL;
	int row,column;
	bzero(sql,256);
	sprintf(sql,"select * from staff where number=\"%s\"",staff_msg.number);
	if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
	{
		printf("用户未注册\n");
		return -1;
	}
	if(row == 0)
	{
		strcpy(staff_msg.text,"notexits");
		if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
		{
			MESSAGE("send");
			return -1;
		}
	}
	else
	{
		switch(staff_msg.operate)
		{
		case 1:
			//修改名字
			strncpy(staff_msg.name,staff_msg.text,sizeof(staff_msg.name));
			sprintf(sql,"update staff set name =\"%s\" where number=\"%s\"",staff_msg.name,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			break;
		case 2:
			//修改年龄
			strncpy(staff_msg.age,staff_msg.text,sizeof(staff_msg.age));
			sprintf(sql,"update staff set age =\"%s\" where number=\"%s\"",staff_msg.age,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			break;
		case 3:
			//修改住址
			strncpy(staff_msg.address,staff_msg.text,sizeof(staff_msg.address));
			sprintf(sql,"update staff set address=\"%s\" where number=\"%s\"",staff_msg.address,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			break;
		case 4:
			//修改电话号码
			strncpy(staff_msg.phone,staff_msg.text,sizeof(staff_msg.phone));
			sprintf(sql,"update staff set phone =\"%s\" where number=\"%s\"",staff_msg.phone,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			break;
		case 5:
			//修改薪水
			strncpy(staff_msg.salary,staff_msg.text,sizeof(staff_msg.salary));
			sprintf(sql,"update staff set salary =\"%s\" where number=\"%s\"",staff_msg.salary,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			break;
		}

	}
	sqlite3_free_table(pres);
	return 0;
}

int do_add(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	int ret;
	char sql[256]="";
	char *errmsg=NULL;
	char **pres=NULL;
	int row,column;
	bzero(sql,256);
	sprintf(sql,"select * from staff where number=\"%s\"",staff_msg.number);
	if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
	{
		printf("用户未注册\n");
		return -1;
	}
	if(row == 0)
	{
		strcpy(staff_msg.text,"notexits");
		if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
		{
			MESSAGE("send");
			return -1;
		}
	}
	else
	{
		if(staff_msg.mod =='p')
		{
			sprintf(sql,"update staff set sex=\"%s\",age=\"%s\",address=\"%s\",phone=\"%s\" where number=\"%s\"",\
					staff_msg.sex,staff_msg.age,staff_msg.address,staff_msg.phone,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			strcpy(staff_msg.text,"ok");
			ret=send(cfd,&staff_msg,sizeof(staff_msg),0);
			if(ret < 0)
			{
				MESSAGE("send");
				return -1;
			}
		}
		else if(staff_msg.mod =='r')
		{

			sprintf(sql,"update staff set salary=\"%s\" where number=\"%s\"",staff_msg.salary,staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			strcpy(staff_msg.text,"ok");
			ret=send(cfd,&staff_msg,sizeof(staff_msg),0);
			if(ret < 0)
			{
				MESSAGE("send");
				return -1;
			}
		}
	}
	sqlite3_free_table(pres);
	return 0;
}


int do_del(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	int ret;
	char sql[256]="";
	char *errmsg=NULL;
	char **pres=NULL;
	int row,column;
	bzero(sql,256);
	sprintf(sql,"select * from staff where number=\"%s\"",staff_msg.number);
	if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
	{
		printf("用户未注册\n");
		return -1;
	}
	if(row == 0)
	{
		strcpy(staff_msg.text,"notexits");
		if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
		{
			MESSAGE("send");
			return -1;
		}
	}
	else
	{
	sprintf(sql,"delete from staff where number=\"%s\"",staff_msg.number);
	if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s  __%d__ \n",errmsg,__LINE__);
		return -1;
	}
	strcpy(staff_msg.text,"ok");
	ret=send(cfd,&staff_msg,sizeof(staff_msg),0);
	if(ret < 0)
	{
		MESSAGE("send");
		return -1;
	}
	}
	return 0;
}

int do_quit(int cfd,struct people staff_msg,sqlite3* staff_db)
{

	char sql[256]="";
	char *errmsg=NULL;
	sprintf(sql,"update staff set stage=0 where number=\"%s\";",staff_msg.number);
	if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s  __%d__ \n",errmsg,__LINE__);
		return -1;
	}
	printf("%s:退出\n",staff_msg.number);
	close(cfd);
	return 0;
}

int do_log(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	char sql[256]="";
	char *errmsg=NULL;
	bzero(sql,256);
	sprintf(sql,"select * from staff where number=\"%s\" and text=\"%s\"",staff_msg.number,staff_msg.text);
	char **pres=NULL;
	int row,column;
	strcpy(name,staff_msg.number);
	if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
	{
		printf("用户未注册\n");
		return -1;
	}
	if(row == 0)
	{
		strcpy(staff_msg.text,"notexits");
	}
	else
	{
		if(strcmp(pres[(row+1)*column-1],"0")!=0)
		{
			strcpy(staff_msg.text,"exists");
		}
		else
		{
			strcpy(staff_msg.text,"ok");
			bzero(sql,256);
			sprintf(sql,"update staff set stage=1 where number=\"%s\"",staff_msg.number);
			if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
			{
				printf("%s\n",errmsg);
				return -1;
			}
			bzero(sql,256);
			sprintf(sql,"select mod from staff where number=\"%s\"",staff_msg.number);
			char **pres=NULL;
			int row,column;
			if(sqlite3_get_table(staff_db,sql,&pres,&row,&column,&errmsg)!=0)
			{
				printf("用户未注册\n");
				return -1;
			}
			staff_msg.mod=*pres[(row+1)*column-1];
		}
	}
	if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
	{
		MESSAGE("send");
		return -1;
	}
	sqlite3_free_table(pres);
	return 0;
}

int do_zuce(int cfd,struct people staff_msg,sqlite3* staff_db)
{
	char sql[256]="";
	char *errmsg=NULL;
	sprintf(sql,"insert into staff(number,name,text,mod) values (\"%s\",\"%s\",\"%s\",\"%c\")",staff_msg.number,staff_msg.name,staff_msg.text,staff_msg.mod);
	if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
	{
		if(strcmp("UNIQUE constraint failed: staff.number",errmsg)==0)
		{
			strcpy(staff_msg.text,"exists");
		}
		else
		{
			strcpy(staff_msg.text,"error");
		}
	}
	else
	{
		printf("注册成功\n");
		strcpy(staff_msg.text,"ok");

		sprintf(sql,"update staff set stage=0 where number=\"%s\";",staff_msg.number);
		if(sqlite3_exec(staff_db,sql,NULL,NULL,&errmsg)!=0)
		{
			printf("%s  __%d__ \n",errmsg,__LINE__);
			return -1;
		}
	}
	if(send(cfd,&staff_msg,sizeof(staff_msg),0)<0)
	{
		MESSAGE("send");
		return -1;
	}
	return 0;
}

int init_socket(int* psfd)
{
	//socket 
	*psfd =socket(AF_INET,SOCK_STREAM,0);
	if(*psfd < 0)
	{
		MESSAGE("socket");
		return -1;
	}
	//快速复用
	int values =0;
	if(setsockopt(*psfd,SOL_SOCKET,SO_REUSEADDR,&values,sizeof(int))<0)
	{
		MESSAGE("setsockopt");
		return -1;
	}
	//填充服务器信息
	struct sockaddr_in sin;
	sin.sin_family =AF_INET;
	sin.sin_port =htons(PORT);
	sin.sin_addr.s_addr =inet_addr(IP);

	//bind
	if(bind(*psfd,(struct sockaddr *)&sin,sizeof(sin))<0)
	{
		MESSAGE("bind");
		return -1;
	}
	//listen
	if(listen(*psfd,5)<0)
	{
		MESSAGE("listen");
		return -1;
	}
	return 0;
}

int init_sqlite(sqlite3** pstaff_db)
{
	char sql[256]="";
	char *errmsg=NULL;
	//创建打开员工库
	if(sqlite3_open("./staff_db",pstaff_db)!=0)
	{
		printf("sqlite3 open error\n");
		return -1;
	}

	bzero(sql,256);
	sprintf(sql,"create table if not exists staff(mod char,name char,text char,sex char,age int,address char,salary int,number char primary key,phone char,stage int);");
	if(sqlite3_exec(*pstaff_db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s __%d__\n",errmsg,__LINE__);
		return -1;
	}
	printf("员工数据库创建成功\n");

	bzero(sql,256);
	sprintf(sql,"update staff set stage=0;");
	if(sqlite3_exec(*pstaff_db,sql,NULL,NULL,&errmsg)!=0)
	{
		printf("%s  __%d__ \n",errmsg,__LINE__);
		return -1;
	}
	printf("用户状态清空完成\n");
	return 0;

}

