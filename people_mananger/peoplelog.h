#ifndef __PEOPLELOG_H__
#define __PEOPLELOG_H__

struct people
{
	char type;
	char mod; //权限
	char name[20];
	char text[128];//存放信息和密码
	char sex[3];
	char age[4];
	char address[20];
	char salary[6];
	char number[20];//工号
	char phone[20];
	int stage;
	int operate;
};

int do_zuce();// z
int do_log();// l
int do_quit();// q
int init_socket();
typedef void(*sighandler_t)(int);
int del_staff();
int operation();
int add_staff();
int modify_staff();
int search_staff();
#endif
