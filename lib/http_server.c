//
// Created by HEADS on 2021/2/21.
//

#include <assert.h>
#include "http_server.h"


int http_onConnectionCompleted(struct tcp_connection *tcp_conn) {
    lamp_msgx("connection completed");
    struct http_request *http_re = http_request_new();
    tcp_conn->request = http_re;
    return 0;
}

int process_status_line(char *start, char *end, struct http_request *http_re) {
    int size = end - start;

    // method
    char *space = memmem(start, end - start, " ", 1);
    assert(space != NULL);
    int method_size = space - start;
    http_re->method = malloc(method_size + 1);
    strncpy(http_re->method, start, space - start);
    http_re->method[method_size + 1] = '\0';

    // url
    start = space + 1;
    space = memmem(start, end - start, " ", 1);
    assert(space != NULL);
    int url_size = space - start;
    http_re->url = malloc(url_size + 1);
    strncpy(http_re->url, start, space - start);
    http_re->url[url_size + 1] = '\0';

    // version
    start = space + 1;
    http_re->version = malloc(end - start + 1);
    strncpy(http_re->version, start, end - start);
    http_re->version[end - start + 1] = '\0';
    assert(space != NULL);
    return size;
}

int parse_http_request(struct buffer *input, struct http_request *http_re) {
    int ok = 1;
    while(http_re->current_state != REQUEST_DONE) {
        if (http_re->current_state == REQUEST_STATUS) {
            char *crlf = buffer_find_CRLF(input);
            if (crlf) {
                int request_line_size = process_status_line(input->data + input->read_index, crlf, http_re);
                if (request_line_size) {
                    input->read_index += request_line_size;
                    input->read_index += 2;
                    http_re->current_state = REQUEST_HEADERS;
                }
            }
        } else if (http_re->current_state == REQUEST_HEADERS) {
            char *crlf = buffer_find_CRLF(input);
            if (crlf) {
                char *start = input->data + input->read_index;
                int request_line_size = crlf - start;
                char *colon = memmem(start, request_line_size, ": ", 2);
                if (colon != NULL) {
                    char *key = malloc(colon - start + 1);
                    strncpy(key, start, colon - start);
                    key[colon - start] = '\0';
                    char *value = malloc(crlf - colon - 1 + 1);
                    strncpy(value, colon + 2, crlf - colon - 2);
                    value[crlf - colon - 2] = '\0';

                    http_request_add_header(http_re, key, value);

                    input->read_index += request_line_size;
                    input->read_index += 2;
                } else {
                    input->read_index += 2;
                    http_re->current_state = REQUEST_DONE;
                }
            }
        }
    }
    return ok;
}

int http_onMessage(struct buffer *input, struct tcp_connection *tcp_conn) {
    lamp_msgx("get message from tcp connection %s", tcp_conn->name);

    struct http_request *http_re = (struct http_request *)tcp_conn->request;
    struct http_server *http_server = (struct http_server *)tcp_conn->data;

    if (parse_http_request(input, http_re) == 0) {
        char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        tcp_connection_send_data(tcp_conn, error_response, sizeof(error_response));
        tcp_connection_shutdown(tcp_conn);
    }

    if (http_request_current_state(http_re) == REQUEST_DONE) {
        struct http_response *http_response = http_response_new();

        if (http_server->request_callback != NULL) {
            http_server->request_callback(http_re, http_response);
        }

        struct buffer *buffer = buffer_new();
        http_response_encode_buffer(http_re, buffer);
        tcp_connection_send_buffer(tcp_conn, buffer);

        if (http_request_close_connection(http_re)) {
            tcp_connection_shutdown(tcp_conn);
        }

        http_request_reset(http_re);
    }
}

int http_onWriteCompleted(struct tcp_connection *tcp_conn) {
    lamp_msgx("write completed");
    return 0;
}

int http_onConnectionClosed(struct tcp_connection *tcp_conn) {
    lamp_msgx("connection closed");
    if (tcp_conn->request != NULL) {
        http_request_clear(tcp_conn->request);
        tcp_conn->request = NULL;
    }
    return 0;
}


struct http_server *http_server_new(struct event_loop *ev_loop, int port, request_callback *re_callback, int thread_num) {
    struct http_server *http_server = malloc(sizeof(struct http_server));
    http_server->request_callback = re_callback;

    struct acceptor *acceptor = acceptor_init(SERV_PORT);
    http_server->tcp_server = tcp_server_init(ev_loop, acceptor, http_onConnectionCompleted, http_onMessage,
                                              http_onWriteCompleted,
                                              http_onConnectionClosed, thread_num);

    http_server->tcp_server->data = http_server;

    return http_server;
}

void http_server_start(struct http_server *http_server) {
    tcp_server_start(http_server->tcp_server);
}