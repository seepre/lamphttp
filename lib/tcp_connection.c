//
// Created by HEADS on 2021/2/20.
//

#include "tcp_connection.h"
#include "utils.h"

int handle_connection_closed(struct tcp_connection *tcp_conn) {
    struct event_loop *ev_loop = tcp_conn->ev_loop;
    struct channel *ch = tcp_conn->channel;

    event_loop_remove_channel_event(ev_loop, ch->fd, ch);

    if (tcp_conn->conn_close_callback != NULL) {
        tcp_conn->conn_close_callback(tcp_conn);
    }
    return 0;
}


int handle_read(void *data) {
    struct tcp_connection *tcp_conn = (struct tcp_connection *) data;
    struct buffer *input_buffer = tcp_conn->input_buffer;
    struct channel *ch = tcp_conn->channel;

    if (buffer_socket_read(input_buffer, ch->fd) > 0) {
        if (tcp_conn->msg_callback != NULL) {
            tcp_conn->msg_callback(input_buffer, tcp_conn);
        }
    }  else {
        handle_connection_closed(tcp_conn);
    }
    return 0;
}

int handle_write(void *data) {
    struct tcp_connection *tcp_conn = (struct tcp_connection *)data;
    struct event_loop *ev_loop = tcp_conn->ev_loop;

    assert_in_same_thread(ev_loop);

    struct buffer *output_buffer = tcp_conn->output_buffer;
    struct channel *ch = tcp_conn->channel;

    ssize_t nwrited = write(ch->fd, output_buffer->data + output_buffer->read_index, buffer_readable_size(output_buffer));

    if (nwrited > 0) {
        output_buffer->read_index += nwrited;
        if (buffer_readable_size(output_buffer) == 0) {
            channel_write_event_disable(ch);
        }

        if (tcp_conn->write_completed_callback != NULL) {
            tcp_conn->write_completed_callback(tcp_conn);
        }
    } else {
        lamp_msgx("handle_write for tcp connection %s", tcp_conn->name);
    }
    return 0;
}

struct tcp_connection *tcp_connection_new(int connected_fd, struct event_loop *ev_loop, connection_completed_call_back conn_completed_callback,
        connection_close_call_back conn_close_callback,
        message_call_back msg_callback,
        write_completed_call_back write_completed_callback) {

    struct tcp_connection *tcp_conn = malloc(sizeof(struct tcp_connection));

    tcp_conn->write_completed_callback = write_completed_callback;
    tcp_conn->msg_callback = msg_callback;
    tcp_conn->conn_completed_callback = conn_completed_callback;
    tcp_conn->conn_close_callback = conn_close_callback;
    tcp_conn->ev_loop = ev_loop;
    tcp_conn->input_buffer = buffer_new();
    tcp_conn->output_buffer = buffer_new();

    char *buf = malloc(16);
    sprintf(buf, "connection-%d\0", connected_fd);

    tcp_conn->name = buf;
    tcp_conn->channel = channel_new(connected_fd, EVENT_READ, handle_read, handle_write, tcp_conn);

    if (tcp_conn->conn_completed_callback != NULL) {
        tcp_conn->conn_completed_callback(tcp_conn);
    }

    event_loop_add_channel_event(tcp_conn->ev_loop, connected_fd, tcp_conn->channel);
    return tcp_conn;
}

int tcp_connection_send_data(struct tcp_connection *tcp_conn, void *data, int size) {
    size_t nwrited = 0;
    size_t nleft = size;
    int fault = 0;

    struct channel *ch = tcp_conn->channel;
    struct buffer *output_buffer = tcp_conn->output_buffer;

    if (!channel_write_event_is_enabled(ch) && buffer_readable_size(output_buffer) == 0) {
        nwrited = write(ch->fd, data, size);
        if (nwrited >= 0) {
            nleft = nleft - nwrited;
        } else {
            nwrited = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }

    if (!fault && nleft > 0) {
        buffer_append(output_buffer, data + nwrited, nleft);
        if (!channel_write_event_is_enabled(ch)) {
            channel_write_event_enable(ch);
        }
    }

    return nwrited;
}

int tcp_connection_send_buffer(struct tcp_connection *tcp_conn, struct buffer *buf) {
    int size = buffer_readable_size(buf);
    int result = tcp_connection_send_data(tcp_conn, buf->data + buf->read_index, size);
    return result;
}

void tcp_connection_shutdown(struct tcp_connection *tcp_conn) {
    if (shutdown(tcp_conn->channel->fd, SHUT_WR) < 0) {
        lamp_msgx("tcp_connection_shutdown failed, socket = %d", tcp_conn->channel->fd);
    }
}