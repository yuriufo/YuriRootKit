#ifndef HIDEFILE_H
#define HIDEFILE_H

#include "utils.h"

int init_hidefile(void);

void cleanup_hidefile(void);

void add_hidefile(char *);

void delete_hidefile(char *);

#endif