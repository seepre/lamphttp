//
// Created by HEADS on 2021/2/20.
//

#ifndef COMMON_H
#define COMMON_H

#include "config.h"
#include "log.h"

#include "tcp_server.h"
#include "inetaddress.h"
#include "channel_map.h"

#include <sys/types.h>      // basic system data types
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <sys/select.h>
#include <sys/sysctl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <pthread.h>

#ifdef EPOLL_ENABLE     // config in config.h use CMakeList.txt
#include <sys/epoll.h>
#endif

void err_dump(const char *, ...);

void err_msg(const char *, ...);

void err_quit(const char *, ...);

void err_ret(const char *, ...);

void err_sys(const char *, ...);

void error(int status, int err, char *fmt, ...);

char *sock_ntop(const struct sockaddr_in *sin, socklen_t salen);

size_t readn(int fd, void *vptr, size_t n);

size_t read_message(int fd, char *buf, size_t length);

size_t readline(int fd, char *buf, size_t length);

int tcp_server(int port);

int tcp_server_listen(int port);

int tcp_nonblocking_server_listen(int port);

void make_nonblocking(int fd);

int tcp_client(char *address, int port);


#define     SERV_PORT       1234
#define     MAXLINE         4096
#define     UNIXSTR_PATH    "/var/lib/mytmp.sock"
#define     LISTENO         1024
#define     BUFFER_SIZE     4096

#endif //COMMON_H
