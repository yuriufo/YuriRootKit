#ifndef HIDEPID_H
#define HIDEPID_H

#include "utils.h"

int init_hidepid(void);

void cleanup_hidepid(void);

void add_hidepid(int);

void delete_hidepid(int);

#endif