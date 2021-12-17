#ifndef __PEOPPLESER_H__
#define __PEOPPLESER_H__

struct people
{
	char type;
	char mod;//权限
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

typedef struct
{
	int fd;
	struct sockaddr_in cin;
	sqlite3* staff_db;
}staff;


int do_zuce();
int do_log();
int do_quit();
int init_sqlite();
void *handler_staff_ser();
int init_socket();
int do_del();
int do_add();
int do_modify();
int do_search();
#endif
