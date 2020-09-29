#ifndef MYSOCKETS_H
#define MYSOCKETS_H

#include "lib.h"

int server_send(struct socket *sock, struct sockaddr_in *addr,
                unsigned char *buf, int len);

int server_receive(struct socket *sock, struct sockaddr_in *addr,
                   unsigned char *buf, int len);

#endif