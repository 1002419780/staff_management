#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <pthread.h>
#include <signal.h>
#include "peoplelog.h"
#include <arpa/inet.h>
#define MESSAGE(msg) do{\
	perror(msg);\
	printf("__%d__",__LINE__);\
}while(0)

#define IP "192.168.1.23"
#define PORT 8888
char num[20]="";
int flag=0;
char right=0;
int sockfd=0;
void handler(int sig)
{
	do_quit(sockfd);
}

int main()
{

	//注册信号处理函数
	sighandler_t s =signal(2,handler);
	if(s ==SIG_ERR)
	{
		MESSAGE("signal");
		return -1;
	}
	//网络初始化
	int fd=0;
	if(init_socket(&fd)<0)
	{
		printf("网络初始化失败\n");
		return -1;
	}
	sockfd=fd;
	while(1)
	{
		system("clear");
		printf("********************************\n");
		printf("************1.注册**************\n");
		printf("************2.登录**************\n");
		printf("************3.退出**************\n");
		printf("********************************\n");

		int choose;
		printf("输入你的选择>>>>>");
		scanf("%d",&choose);
		while(getchar()!=10);

		switch(choose)
		{
		case 1:
			//注册
			do_zuce(fd);
			break;
		case 2:
			//登录
			do_log(fd);
			if(flag == 1)
			{
				operation(fd);
			}
			break;
		case 3:
			//退出
			do_quit(fd);
			close(sockfd);
			return 0;
		default:
			printf("输入错误，请重新输入\n");
			break;
		}
		printf("输入任意字符清屏>>>>>");
		while(getchar()!=10);
	}
	return 0;
}

int operation(int fd)
{
	while(1)
	{
err:
		printf("*********************\n");
		printf("********1.增*********\n");
		printf("********2.删*********\n");
		printf("********3.改*********\n");
		printf("********4.查*********\n");
		printf("********5.返回*******\n");
		printf("*********************\n");
		int choose;
		printf("输入你的选择>>>>>");
		scanf("%d",&choose);
		while(getchar()!=10);
		switch(choose)
		{
		case 1:
			//增
			add_staff(fd);
			break;
		case 2:
			//删
			del_staff(fd);
			break;
		case 3:
			//改
			modify_staff(fd);
			break;
		case 4:
			//查
			search_staff(fd);
			break;
		case 5:
			//返回
			return 0;
		default:
			printf("输入有误,请重新输入\n");
			goto err;

		}
	}
}
int search_staff(int fd)
{
	struct people staff_all={'s'};
	staff_all.mod=right;
	printf("请输入你想查看的员工工号>>>");
	scanf("%s",staff_all.number);
	if(send(fd,&staff_all,sizeof(staff_all),0)<0)
	{
		MESSAGE("send");
		return -1;
	}
	recv(fd,&staff_all,sizeof(staff_all),0);
	printf("%s\n",staff_all.text);

	return 0;
}

//改
int modify_staff(int fd)
{
	char ret[20]="";
	int res;
	struct people staff_all={'m'};
	if(right =='p')
	{
		while(1)
		{
err1:
			printf("******************\n");
			printf("*****1.修改名字***\n");
			printf("*****2.修改年龄***\n");
			printf("*****3.修改住址***\n");
			printf("*****4.修改电话号码\n");
			printf("*****5.修改完毕，退出\n");
			printf("请输入你要进行的操作>>>");
			strcpy(staff_all.number,num);
			scanf("%d",&staff_all.operate);
			while(getchar()!=10);
			if(staff_all.operate==5)
			{
				return 0;
			}
			if(staff_all.operate >5)
			{
				printf("输入错误,请重新输入\n");
				goto err1;
			}
			printf("请输入你修改之后的样子>>");
			scanf("%s",staff_all.text);
			while(getchar()!=10);
			if(send(fd,&staff_all,sizeof(staff_all),0)<0)
			{
				MESSAGE("send");
				return -1;
			}
		}
	}
	if(right =='r')
	{
		while(1)
		{
err2:
			printf("******************\n");
			printf("*****1.修改名字***\n");
			printf("*****2.修改年龄***\n");
			printf("*****3.修改住址***\n");
			printf("*****4.修改电话号码\n");
			printf("*****5.修改薪水***\n");
			printf("*****6.修改完毕，退出\n");
			printf("请输入你要进行的操作>>>");
			scanf("%d",&staff_all.operate);
			while(getchar()!=10);
			if(staff_all.operate==6)
			{
				return 0;
			}
			if(staff_all.operate >6)
			{
				printf("输入错误，请重新输入\n");
				goto err2;
			}
			printf("请输入你要修改的工号>>>");
			scanf("%s",ret);
			while(getchar()!=10);
			strcpy(staff_all.number,ret);
			printf("请输入你修改之后的样子>>");
			scanf("%s",staff_all.text);
			while(getchar()!=10);
			if(send(fd,&staff_all,sizeof(staff_all),0)<0)
			{
				MESSAGE("send");
				return -1;
			}
		}
	}
	res=recv(fd,&staff_all,sizeof(staff_all),0);
	if(res == -1)
	{
		MESSAGE("recv");
		return -1;
	}
	else if(res ==0)
	{
		printf("对方关闭\n");
	}
	else
	{
		if(strcmp(staff_all.text,"notexits")==0)
		{
			printf("该员工不存在\n");
		}
		else if(strcmp(staff_all.text,"ok")==0)
		{
			printf("修改成功\n");
		}
	}

	return 0;
}
//增加
int add_staff(int fd)
{
	struct people staff_all={'a'};
	if(right =='p')
	{
		strcpy(staff_all.number,num);
		staff_all.mod=right;
		printf("输入你的性别>>>>>");
		scanf("%s",staff_all.sex);
		while(getchar()!=10);

		printf("输入你的年龄>>>>");
		scanf("%s",staff_all.age);
		while(getchar()!=10);

		printf("输入你的住址>>>>");
		scanf("%s",staff_all.address);
		while(getchar()!=10);

		printf("输入你的电话号码>>");
		scanf("%s",staff_all.phone);
		while(getchar()!=10);

	}
	else if(right =='r')
	{
		staff_all.mod=right;
		printf("请输入你要增加信息的员工工号>>>");
		scanf("%s",num);
		while(getchar()!=10);
		strcpy(staff_all.number,num);
		printf("请输入你给的薪资>>>");
		scanf("%s",staff_all.salary);
		while(getchar()!=10);

	}
	if(send(fd,&staff_all,sizeof(staff_all),0)<0)
	{
		MESSAGE("send");
		return -1;
	}
	if(recv(fd,&staff_all,sizeof(staff_all),0)<0)
	{
		MESSAGE("recv");
		return -1;
	}
	printf("%s\n",staff_all.text);
	return 0;
}
//删除
int del_staff(int fd)
{
	int ret;
	if(right =='p')
	{
		printf("权限不够\n");
		return -1;
	}
	printf("输入你要删除的员工工号>>>>");
	char humen[20];
	scanf("%s",humen);
	struct people staff_all={'d'};
	strcpy(staff_all.number,humen);
	ret=send(fd,&staff_all,sizeof(staff_all),0);
	if(ret ==-1)
	{
		MESSAGE("send");
		return -1;
	}

	ret =recv(fd,&staff_all,sizeof(staff_all),0);
	if(ret < 0)
	{
		MESSAGE("recv");
		return -1;
	}
	else if(ret ==0)
	{
		printf("对方关闭\n");
		return -1;
	}
	printf("%s\n",staff_all.text);
	if(strcmp(staff_all.text,"ok")==0)
	{
		printf("删除成功\n");
	}
	else
	{
		printf("删除失败\n");
	}
	return 0;

}
//退出
int do_quit(int fd)
{
	int ret=0;
	struct people staff_all={'q'};
	strcpy(staff_all.number,num);
	ret=send(fd,&staff_all,sizeof(staff_all),0);
	if(ret ==-1)
	{
		MESSAGE("send");
		return -1;
	}
	exit(0);
	return 0;
}

//登录
int do_log(int fd)
{
	//用工号和密码登录
	int res;
	struct people staff_all ={'l'};
	printf("请输入你的工号>>>>");
	scanf("%s",staff_all.number);
	while(getchar()!=10);
	printf("请输入你的密码>>>");
	scanf("%s",staff_all.text);
	while(getchar()!=10);
	strcpy(num,staff_all.number);
	//发送消息
	res=send(fd,&staff_all,sizeof(staff_all),0);
	if(res ==-1)
	{
		MESSAGE("send");
		return -1;
	}

	//接受消息
	memset(&staff_all,0,sizeof(staff_all));
	res=recv(fd,&staff_all,sizeof(staff_all),0);
	if(res == -1)
	{
		MESSAGE("recv");
		return -1;
	}
	else if(res == 0)
	{
		printf("对方关闭\n");
		return -1;
	}
	if(strcmp(staff_all.text,"notexits")==0)
	{
		printf("该员工不存\n");
	}
	else if(strcmp(staff_all.text,"exists")==0)
	{
		printf("该员工在登录\n");
	}
	else
	{
		if(strcmp(staff_all.text,"ok")==0)
		{
			printf("登录成功\n");
			right=staff_all.mod;
			flag=1;
		}
	}
	return 0;
}
//注册 
int do_zuce(int fd)
{
	int res;
	struct people staff_all={'z'};
err:
	printf("请输入你的身份  p:员工 r：管理员>>>");
	scanf("%c",&staff_all.mod);
	while(getchar()!=10);
	if(staff_all.mod !='p' && staff_all.mod !='r')
	{
		printf("输入的权限有误,请重新输入\n");
		goto err;
	}
	printf("请输入工号>>>>>");
	scanf("%s",staff_all.number);
	while(getchar()!=10);
	printf("请输入名字>>>>");
	scanf("%s",staff_all.name);
	while(getchar()!=10);
	printf("请输入密码>>>>");
	scanf("%s",staff_all.text);
	while(getchar()!=10);

	//发送信息
	res=send(fd,&staff_all,sizeof(staff_all),0);
	if(res < 0)
	{
		MESSAGE("send");
		return -1;
	}
	//接受消息
	memset(&staff_all,0,sizeof(staff_all));

	res =recv(fd,&staff_all,sizeof(staff_all),0);
	if(res == -1)
	{
		MESSAGE("recv");
		return -1;
	}
	else if(res == 0)
	{
		printf("对方关闭\n");
		return -1;
	}
	if(strcmp(staff_all.text,"ok")==0)
	{
		printf("注册成功\n");
	}
	else if(strcmp(staff_all.text,"exists")==0)
	{
		printf("该员工已经存在，请检查你输入的\n");
	}
	else
	{
		printf("未知错误，请联系管理员\n");
	}
	return 0;
}

//网络初始化
int init_socket(int *sfd)
{
	//socket
	*sfd = socket(AF_INET,SOCK_STREAM,0);
	if(*sfd < 0)
	{
		MESSAGE("socket");
		return -1;
	}
	//快速复用
	int values =0;
	if(setsockopt(*sfd,SOL_SOCKET,SO_REUSEADDR,&values,sizeof(int))<0)
	{
		MESSAGE("setsockopt");
		return -1;
	}
	//填充服务器信息
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port =htons(PORT);
	sin.sin_addr.s_addr =inet_addr(IP);

	if(connect(*sfd,(struct sockaddr*)&sin,sizeof(sin))< 0)
	{
		MESSAGE("connect");
		return -1;
	}
	return 0;

}

