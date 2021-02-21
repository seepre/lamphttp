//
// Created by HEADS on 2021/2/21.
//

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#define HTTP_RESPONSE_H

#include "buffer.h"

struct response_header {
    char *key;
    char *value;
};

enum HttpStatusCode {
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
};

struct http_response {
    enum HttpStatusCode status_code;
    char *status_message;
    char *content_type;
    char *body;
    struct response_header *response_headers;
    int response_headers_number;
    int keep_connected;
};

struct http_response *http_response_new();

void http_response_encode_buffer(struct http_response *http_response, struct buffer *buffer);

#endif //HTTP_RESPONSE_H
