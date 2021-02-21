//
// Created by HEADS on 2021/2/21.
//

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

struct request_header {
    char *key;
    char *value;
};

enum http_request_state {
    REQUEST_STATUS,     // wait parse state
    REQUEST_HEADERS,    // wait parse header
    REQUEST_BODY,       // wait parse body
    REQUEST_DONE        // parse done
};

struct http_request {
    char *version;
    char *method;
    char *url;
    enum http_request_state current_state;
    struct request_header *request_headers;
    int request_headers_number;
};

// init request obj
struct http_request *http_request_new();

void http_request_clear(struct http_request *http_re);

void http_request_reset(struct http_request *http_re);

void http_request_add_header(struct http_request *http_re, char *key, char *value);

char *http_request_get_header(struct http_request *http_re, char *key);

enum http_request_state http_request_current_state(struct http_request *http_re);

int http_request_close_connection(struct http_request *http_re);

#endif //HTTP_REQUEST_H
