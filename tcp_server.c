//
// Created by HEADS on 2021/2/20.
//

#include <assert.h>
#include "common.h"
#include "tcp_server.h"
#include "thread_pool.h"
#include "utils.h"
#include "tcp_connection.h"

int tcp_server(int port) {
    int listen_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret1 < 0) {
        error(1, errno, "bind failed");
    }

    int ret2 = listen(listen_fd, LISTENO);
    if (ret2 < 0) {
        error(1, errno, "listen failed.");
    }

    signal(SIGPIPE, SIG_IGN);

    int conn_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        error(1, errno, "bind failed.");
    }

    return conn_fd;
}

int tcp_server_listen(int port) {
    int listen_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret1 < 0) {
        error(1, errno, "bind failed");
    }

    int ret2 = listen(listen_fd, LISTENO);
    if (ret2 < 0) {
        error(1, errno, "listen failed.");
    }

    signal(SIGPIPE, SIG_IGN);

    return listen_fd;
}


int tcp_nonblocking_server_listen(int port) {
    int listen_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    make_nonblocking(listen_fd);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret1 < 0) {
        error(1, errno, "bind failed.");
    }

    int ret2 = listen(listen_fd, LISTENO);
    if (ret2 < 0) {
        error(1, errno, "listen failed.");
    }

    signal(SIGPIPE, SIG_IGN);

    return listen_fd;
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}


struct TCPServer *tcp_server_init(struct event_loop *ev_loop, struct acceptor *acceptor,
        connection_completed_call_back conn_completed_call_back,
        message_call_back msg_callback,
        write_completed_call_back write_completed_callback,
        connection_close_call_back conn_close_callback,
        int thread_num) {

    struct TCPServer *tcp_server = malloc(sizeof(struct TCPServer));
    tcp_server->ev_loop = ev_loop;
    tcp_server->acceptor = acceptor;
    tcp_server->conn_completed_callback = conn_completed_call_back;
    tcp_server->msg_callback = msg_callback;
    tcp_server->write_completed_callback = write_completed_callback;
    tcp_server->conn_close_callback = conn_close_callback;
    tcp_server->thread_num = thread_num;
    tcp_server->thread_pool = thread_pool_new(ev_loop, thread_num);
    tcp_server->data = NULL;

    return tcp_server;
}

int handle_connection_established(void *data) {
    struct TCPServer *tcp_server = (struct TCPServer *)data;
    struct acceptor *acceptor = tcp_server->acceptor;
    int listen_fd = acceptor->listen_fd;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    make_nonblocking(conn_fd);

    lamp_msgx("new connection established, socket = %d", conn_fd);

    struct event_loop *ev_loop = thread_pool_get_loop(tcp_server->thread_pool);

    struct tcp_connection *tcp_conn = tcp_connection_new(conn_fd, ev_loop,
                                        tcp_server->conn_completed_callback,
                                        tcp_server->conn_close_callback,
                                        tcp_server->msg_callback,
                                        tcp_server->write_completed_callback);

    if (tcp_server->data != NULL) {
        tcp_conn->data = tcp_server->data;
    }
    return 0;
}


void tcp_server_start(struct TCPServer *tcp_server) {
    struct acceptor *acceptor = tcp_server->acceptor;
    struct event_loop *ev_loop = tcp_server->ev_loop;

    thread_pool_start(tcp_server->thread_pool);

    struct channel *ch = channel_new(acceptor->listen_fd, EVENT_READ, handle_connection_established, NULL, tcp_server);

    event_loop_add_channel_event(ev_loop, ch->fd, ch);
    return;
}

void tcp_server_set_data(struct TCPServer *tcp_server, void *data) {
    if (data != NULL) {
        tcp_server->data = data;
    }
}