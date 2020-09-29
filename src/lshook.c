#include "lshook.h"

struct linux_dirent
{
	unsigned long d_ino;
	unsigned long d_off;
	unsigned short d_reclen;
	char d_name[1];
};

int (*original_getdents)(unsigned int fd, struct linux_dirent *dirp, unsigned int count);

asmlinkage int hooked_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{
	int ret = 0;
	pr_info("successfully hook!\n");
	ret = original_getdents(fd, dirp, count);
	return ret;
}

void lshook(void)
{
	original_getdents = (int (*)(unsigned int, struct linux_dirent *, unsigned int))(((int **)sys_call_table)[__NR_getdents]);
	disable_write_protection();
	((int **)sys_call_table)[__NR_getdents] = (int *)(&hooked_getdents);
	enable_write_protection();
	pr_info("original getdents addr: %p\n", original_getdents);
	pr_info("hooked getdents addr: %p\n", &hooked_getdents);
}

void cleanup_lshook(void)
{
	if (original_getdents)
	{
		disable_write_protection();
		((int **)sys_call_table)[__NR_getdents] = (int *)original_getdents;
		enable_write_protection();
	}
	pr_info("Cleanup lshook\n");
}