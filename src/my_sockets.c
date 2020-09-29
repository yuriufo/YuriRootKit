#include "my_sockets.h"

int server_send(struct socket *sock, struct sockaddr_in *addr,
                unsigned char *buf, int len)
{
    struct msghdr msghdr;
    struct iovec iov;
    int size = 0;

    if (sock->sk == NULL)
        return 0;

    iov.iov_base = buf;
    iov.iov_len = len;

    msghdr.msg_name = addr;
    msghdr.msg_namelen = sizeof(struct sockaddr_in);
    msghdr.msg_iter.iov = &iov;
    msghdr.msg_control = NULL;
    msghdr.msg_controllen = 0;
    msghdr.msg_flags = 0;

    iov_iter_init(&msghdr.msg_iter, WRITE, &iov, 1, len);
    pr_info("Send UDP PACKET to REMOTE SERVER %p", &addr->sin_addr.s_addr);

    size = sock_sendmsg(sock, &msghdr);

    return size;
}

int server_receive(struct socket *sock, struct sockaddr_in *addr,
                   unsigned char *buf, int len)
{
    struct msghdr msghdr;
    struct iovec iov;
    int size = 0;

    if (sock->sk == NULL)
        return 0;

    iov.iov_base = buf;
    iov.iov_len = len;

    msghdr.msg_name = addr;
    msghdr.msg_namelen = sizeof(struct sockaddr_in);
    msghdr.msg_iter.iov = &iov;
    msghdr.msg_control = NULL;
    msghdr.msg_controllen = 0;
    msghdr.msg_flags = 0;

    iov_iter_init(&msghdr.msg_iter, READ, &iov, 1, len);
    pr_info("Receive UDP PACKET from REMOTE SERVER %p", &addr->sin_addr.s_addr);

    size = sock_recvmsg(sock, &msghdr, msghdr.msg_flags);

    return size;
}