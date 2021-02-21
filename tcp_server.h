//
// Created by HEADS on 2021/2/20.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

typedef int (*connection_completed_call_back)(struct tcp_connection *tcp_conn);

typedef int (*message_call_back)(struct buffer *buffer, struct tcp_connection *tcp_conn);

typedef int (*write_completed_call_back)(struct tcp_connection *tcp_conn);

typedef int (*connection_close_call_back)(struct tcp_connection *tcp_conn);

#include "acceptor.h"
#include "event_loop.h"
#include "thread_pool.h"
#include "buffer.h"
#include "tcp_connection.h"

struct  TCPServer {
    int port;
    struct event_loop *ev_loop;
    struct acceptor *acceptor;
    connection_completed_call_back  conn_completed_callback;
    message_call_back msg_callback;
    write_completed_call_back write_completed_callback;
    connection_close_call_back conn_close_callback;

    int thread_num;
    struct thread_pool *thread_pool;
    void *data;   // for callback
};

struct TCPServer *tcp_server_init(struct event_loop *ev_loop, struct acceptor *acceptor,
        connection_completed_call_back cnn_completed_callback,
        message_call_back msg_callback,
        write_completed_call_back write_completed_callback,
        connection_close_call_back conn_close_callback,
        int thread_num);

void tcp_server_start(struct TCPServer *tcp_server);

void tcp_server_set_data(struct TCPServer *tcp_server, void *data);

#endif //TCP_SERVER_H
