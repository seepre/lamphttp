//
// Created by HEADS on 2021/2/20.
//

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

struct tcp_connection {
    struct event_loop *ev_loop;
    struct channel *channel;
    char *name;
    struct buffer *input_buffer; // receive buffer
    struct buffer *output_buffer; // send buffer

    connection_completed_call_back conn_completed_callback;
    message_call_back msg_callback;
    write_completed_call_back write_completed_callback;
    connection_close_call_back conn_close_callback;

    void *data;
    void *request;
    void *response;
};

struct tcp_connection *tcp_connection_new(int fd, struct event_loop *ev_loop, connection_completed_call_back conn_completed_callback,
        connection_close_call_back conn_close_callback,
        message_call_back msg_callback,
        write_completed_call_back write_completed_callback);

int tcp_connection_send_data(struct tcp_connection *tcp_conn, void *data, int size);

int tcp_connection_send_buffer(struct tcp_connection *tcp_conn, struct buffer *buf);

void tcp_connection_shutdown(struct tcp_connection *tcp_conn);

#endif //TCP_CONNECTION_H
