//
// Created by HEADS on 2021/2/21.
//

#include "lib/acceptor.h"
#include "lib/http_server.h"
#include "lib/common.h"
#include "lib/event_loop.h"

int onRequest(struct http_request *http_request, struct http_response *http_response) {
    char *url = http_request->url;
    char *question = memmem(url, strlen(url), "?", 1);
    char *path = NULL;
    if (question != NULL) {
        path = malloc(question - url);
        strncpy(path, url, question - url);
    } else {
        path = malloc(strlen(url));
        strncpy(path, url, strlen(url));
    }

    if (strcmp(path, "/") == 0) {
        http_response->status_code = OK;
        http_response->status_message = "OK";
        http_response->content_type = "text/html";
        http_response->body = "<html><head><title>This is Lamphttp</title></head><body><h1>Hello, lamphttp</h1></body></html>";
    } else {
        http_response->status_code = NotFound;
        http_response->status_message = "Not Found";
        http_response->keep_connected = 1;
    }

    return 0;
}

int main(int c, char **v) {
    struct event_loop *ev_loop = event_loop_init();

    struct http_server *http_server = http_server_new(ev_loop, SERV_PORT, onRequest, 2);
    http_server_start(http_server);

    event_loop_run(ev_loop);
}