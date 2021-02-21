//
// Created by HEADS on 2021/2/21.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "common.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

typedef int (*request_callback)(struct http_request *http_re, struct http_response *http_resp);

struct http_server {
    struct TCPServer *tcp_server;
    request_callback request_callback;
};

struct http_server *http_server_new(struct event_loop *ev_loop, int port, request_callback *request_callback, int thread_num);

void http_server_start(struct http_server *http_server);

int pars_http_request(struct buffer *input, struct http_request *http_re);

#endif //HTTP_SERVER_H
