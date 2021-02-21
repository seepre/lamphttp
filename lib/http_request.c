//
// Created by HEADS on 2021/2/21.
//

#include "http_request.h"
#include "common.h"

#define INIT_REQUEST_HEADER_SIZE 128

const char *HTTP10 = "HTTP/1.0";
const char *HTTP11 = "HTTP/1.1";
const char *KEEP_ALIVE = "Keep-Alive";
const char *CLOSE = "close";


struct http_request *http_request_new() {
    struct http_request *http_re = malloc(sizeof(struct http_request));

    http_re->method = NULL;
    http_re->current_state = REQUEST_STATUS;
    http_re->version = NULL;
    http_re->url = NULL;
    http_re->request_headers = malloc(sizeof(struct http_request)*INIT_REQUEST_HEADER_SIZE);
    http_re->request_headers_number = 0;

    return http_re;
}

void http_request_clear(struct http_request *http_re) {
    if (http_re->request_headers != NULL) {
        int i;
        for (i = 0; i < http_re->request_headers_number; i ++) {
            free(http_re->request_headers[i].key);
            free(http_re->request_headers[i].value);
        }
        free(http_re->request_headers);
    }
    free(http_re);
}

void http_request_reset(struct http_request *http_re) {
    http_re->method = NULL;
    http_re->current_state = REQUEST_STATUS;
    http_re->version = NULL;
    http_re->url = NULL;
    http_re->request_headers_number = 0;
}

void http_request_add_header(struct http_request *http_re, char *key, char *val) {
    http_re->request_headers[http_re->request_headers_number].key = key;
    http_re->request_headers[http_re->request_headers_number].value = val;
    http_re->request_headers_number++;
}

char *http_request_get_header(struct http_request *http_re, char *key) {
    if (http_re->request_headers != NULL) {
        int i;
        for (i = 0; i < http_re->request_headers_number; i ++) {
            if (strncmp(http_re->request_headers[i].key, key, strlen(key)) == 0) {
                return http_re->request_headers[i].value;
            }
        }
    }
    return NULL;
}

enum http_request_state http_request_current_state(struct http_request *http_re) {
    return http_re->current_state;
}

int http_request_close_connection(struct http_request *http_re) {
    char *connection = http_request_get_header(http_re, "Connection");

    if (connection != NULL && strncmp(connection, CLOSE, strlen(CLOSE)) == 0) {
        return 1;
    }

    if (http_re->version != NULL &&
    strncmp(http_re->version, HTTP10, strlen(HTTP10)) == 0 &&
    strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 1) {
        return 1;
    }
    return 0;
}