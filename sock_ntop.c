//
// Created by HEADS on 2021/2/21.
//

#include "common.h"

char *sock_ntop(const struct sockaddr_in *sin, socklen_t salen) {
    char ports[8];
    static char str[128]; // Unix domain is largest

    if (inet_pton(AF_INET, &sin->sin_addr, str) == NULL)
        return NULL;

    if (ntohs(sin->sin_port) != 0) {
        snprintf(ports, sizeof(ports), ":%d", ntohl(sin->sin_port));
    }

    return str;
}