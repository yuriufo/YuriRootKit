#ifndef LIB_H
#define LIB_H

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <asm/unistd.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/version.h>
#include <linux/limits.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/rcupdate.h>
#include <linux/rwlock.h>
#include <linux/pid.h>
#include <linux/seq_file.h>
#include <linux/dirent.h>

#include "config.h"

#define pr_fmt(fmt) MODULE_NAME_ ": " fmt

#endif