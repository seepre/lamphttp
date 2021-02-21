//
// Created by HEADS on 2021/2/21.
//

#include "http_response.h"
#include "common.h"

#define INIT_RESPONSE_HEADER_SIZE 128

struct http_response *http_response_new() {
    struct http_response *http_response = malloc(sizeof(struct http_response));

    http_response->body = NULL;
    http_response->status_code = Unknown;
    http_response->status_message = NULL;
    http_response->response_headers = malloc(sizeof(struct response_header) * INIT_RESPONSE_HEADER_SIZE);
    http_response->response_headers_number = 0;
    http_response->keep_connected = 0;

    return http_response;
}

void http_response_encode_buffer(struct http_response *http_response, struct buffer *output) {
    char buf[32];

    snprintf(buf, sizeof(buf), "HTTP/1.1 %d", http_response->status_code);
    buffer_append_string(output, buf);
    buffer_append_string(output, http_response->status_message);
    buffer_append_string(output, "\r\n");

    if (http_response->keep_connected) {
        buffer_append_string(output, "Connection: close\r\n");
    } else {
        snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", strlen(http_response->body));
        buffer_append_string(output, buf);
        buffer_append_string(output, "Connection: Keep-Alive\r\n");
    }

    if (http_response->response_headers != NULL && http_response->response_headers_number > 0) {
        int i;
        for (i = 0; i < http_response->response_headers_number; i ++) {
            buffer_append_string(output, http_response->response_headers[i].key);
            buffer_append_string(output, ": ");
            buffer_append_string(output, http_response->response_headers[i].value);
            buffer_append_string(output, "\r\n");
        }
    }

    buffer_append_string(output, "\r\n");
    buffer_append_string(output, http_response->body);
}