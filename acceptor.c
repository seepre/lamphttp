//
// Created by HEADS on 2021/2/20.
//

#include "acceptor.h"

struct acceptor *acceptor_init(int port) {
    struct acceptor *acr = malloc(sizeof(struct acceptor));
    acr->listen_port = port;
    acr->listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    make_nonblocking(acr->listen_fd);

    struct sockaddr_in serve_addr;
    bzero(&serve_addr, sizeof(serve_addr));

    serve_addr.sin_family = AF_INET;
    serve_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serve_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(acr->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(acr->listen_fd, (struct sockaddr *)&serve_addr, sizeof(serve_addr));
    if (ret1 < 0) {
        error(1, errno, "bind failed.");
    }

    int ret2 = listen(acr->listen_fd, LISTENO);
    if (ret2 < 0) {
        error(1, errno, "listen failed.");
    }

    return acr;
}
