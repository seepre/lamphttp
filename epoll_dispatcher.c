//
// Created by HEADS on 2021/2/20.
//

#include <sys/epoll.h>
#include "event_loop.h"
#include "event_dispatcher.h"
#include "log.h"


#define MAXEVENTS 128

typedef struct {
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event *events;
}epoll_dispatcher_data;

static void *epoll_init(struct event_loop *);

static int epoll_add(struct event_loop *, struct channel *ch);

static int epoll_del(struct event_loop *), struct  channel *ch);

static int epoll_update(struct event_loop *, struct channel *ch);

static int epoll_dispatch(struct event_loop *, struct timeval *);

static void epoll_clear(struct event_loop *);

const struct event_dispatcher epoll_dispatcher = {
        "epoll",
        epoll_init,
        epoll_add,
        epoll_del,
        epoll_update,
        epoll_dispatch,
        epoll_clear,
};

void *epoll_init(struct event_loop *ev_loop) {
    epoll_dispatcher_data *edd = malloc(sizeof(epoll_dispatcher_data));

    edd->event_count = 0;
    edd->nfds = 0;
    edd->realloc_copy = 0;
    edd->efd = epoll_create1(0);
    if (edd->efd == -1) {
        error(1, errno, "epoll create failed.");
    }

    edd->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    return edd;
}

int epoll_add(struct event_loop *ev_loop, struct channel *ch) {
    epoll_dispatcher_data *edd = (epoll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int fd = ch->fd;
    int events = 0;
    if (ch->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (ch->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;

    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(edd->efd, EPOLL_CTL_ADD, fd, &event) == -1) {
        error(1, errno, "epoll_ctl add fd failed.");
    }
    return 0;
}

int epoll_del(struct event_loop *ev_loop, struct channel *ch) {
    epoll_dispatcher_data  *edd = (epoll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int fd = ch->fd;
    int events = 0;
    if (ch->events & EVENT_READ) {
        events = events | EPOLLIN;
    }

    if (ch->events | EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;

    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(edd->efd, EPOLL_CTL_DEL, fd, &event) == -1) {
        error(1, errno, "epoll_ctl delete fd failed.");
    }

    return 0;
}

int epoll_update(struct event_loop *ev_loop, struct channel *ch) {
    epoll_dispatcher_data *edd = (epoll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int fd = ch->fd;
    int events = 0;

    if (ch->events & EVENT_READ) {
        events = events | EPOLLIN;
    }

    if (ch->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;

    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(edd->efd, EPOLL_CTL_MOD, fd, &event) == -1) {
        error(1, errno, "epoll_ctl modify fd failed.");
    }

    return 0;
}

int epoll_dispatch(struct event_loop *ev_loop, struct timeval *timeval) {
    epoll_dispatcher_data *edd = (epoll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int i,n;

    n = epoll_wait(edd->efd, edd->events, MAXEVENTS, -1);
    lamp_msgx("epoll_wait wakeup, %s", ev_loop->thread_name);

    for (i = 0; i < n; i ++) {
        if ((edd->events[i].events & EPOLLERR) || (edd->events[i].events & EPOLLHUP)) {
            fprintf(stderr, "epoll error\n");
            close(edd->events[i].data.fd);
            continue;
        }

        if (edd->events[i].events & EPOLLIN) {
            lamp_msgx("get message channel fd=%d for read, %s", edd->events[i].data.fd, ev_loop->thread_name);

            channel_event_active(ev_loop, edd->events[i].data.fd, EVENT_READ);
        }

        if (edd->events[i].events & EPOLLOUT) {
            lamp_msgx("get message channel fd=%d for write %s", edd->events[i].data.fd, ev_loop->thread_name);

            channel_event_active(ev_loop, edd->events[i].data.fd, EVENT_WRITE);
        }
    }

    return 0;
}

void epoll_clear(struct event_loop *ev_loop) {
    epoll_dispatcher_data  *edd = (epoll_dispatcher_data *)ev_loop->event_dispatcher_data;

    free(edd->events);
    free(edd->efd);
    free(edd);
    ev_loop->event_dispatcher_data = NULL;
    return;
}