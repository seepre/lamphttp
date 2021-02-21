//
// Created by HEADS on 2021/2/21.
//

#include "event_dispatcher.h"
#include "event_loop.h"
#include "log.h"

#define INIT_POLL_SIZE 1024

struct pollidx {
    int indxplus1;
};

struct poll_dispatcher_data {
    int event_count;
    int nfds;
    int realloc_copy;
    struct pollfd *event_set;
    struct pollfd *event_set_copy;
};


static void *poll_init(struct event_loop *);

static int poll_add(struct event_loop *, struct channel *ch);

static int poll_del(struct event_loop *, struct channel *ch);

static int poll_update(struct  event_loop *, struct channel *ch);

static int poll_dispatch(struct event_loop *, struct timeval *);

static void poll_clear(struct event_loop *);

const struct event_dispatcher poll_dispatcher = {
        "poll",
        poll_init,
        poll_add,
        poll_del,
        poll_update,
        poll_dispatch,
        poll_clear,
};

void *poll_init(struct event_loop *ev_loop) {
    struct poll_dispatcher_data *pdd = malloc(sizeof(struct poll_dispatcher_data));

    // The first element is listen_fd and others is connect_fd
    pdd->event_set = malloc(sizeof(struct pollfd) * INIT_BUFFER_SIZE);

    int i;
    for (i = 0; i < INIT_POLL_SIZE; i ++) {
        pdd->event_set[i].fd = -1;
    }
    pdd->event_count = 0;
    pdd->nfds = 0;
    pdd->realloc_copy = 0;

    return pdd;
}

int poll_add(struct event_loop *ev_loop, struct channel *ch) {
    struct poll_dispatcher_data *pdd = (struct poll_dispatcher_data *) ev_loop->event_dispatcher_data;

    int fd = ch->fd;
    int events = 0;

    if (ch->events & EVENT_READ) {
        events = events | POLLRDNORM;
    }

    if (ch->events & EVENT_WRITE) {
        events = events | POLLWRNORM;
    }

    int i = 0;
    for(i = 0; i < INIT_POLL_SIZE; i ++) {
        if (pdd->event_set[i].fd < 0) {
            pdd->event_set[i].fd = fd;
            pdd->event_set[i].events = events;
            break;
        }
    }

    lamp_msgx("poll added channel fd=%d, %s", fd, ev_loop->thread_name);
    if (i >= INIT_POLL_SIZE) {
        LOG_ERROR("too many clients, just abort it");
    }

    return 0;
}

int poll_del(struct event_loop *ev_loop, struct channel *ch) {
    struct poll_dispatcher_data *pdd = (struct poll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int fd = ch->fd;

    int i = 0;
    for(i = 0; i < INIT_POLL_SIZE; i++) {
        if (pdd->event_set[i].fd == fd) {
            pdd->event_set[i].fd = -1;
            break;
        }
    }

    lamp_msgx("poll delete channel fd=%d, %s", fd, ev_loop->thread_name);
    if (i >= INIT_POLL_SIZE) {
        LOG_ERROR("can not find fd, poll delete error");
    }

    return 0;
}

int poll_update(struct event_loop *ev_loop, struct channel *ch) {
    struct poll_dispatcher_data *pdd = (struct poll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int fd = ch->fd;

    int events = 0;
    if (ch->events & EVENT_READ) {
        events = events | POLLRDNORM;
    }

    if (ch->events & EVENT_WRITE) {
        events = events | POLLWRNORM;
    }

    int i = 0;
    for (i = 0; i < INIT_POLL_SIZE; i ++) {
        if (pdd->event_set[i].fd == fd) {
            pdd->event_set[i].events = events;
            break;
        }
    }

    lamp_msgx("poll updated channel fd=%d, %s", fd, ev_loop->thread_name);
    if (i >= INIT_POLL_SIZE) {
        LOG_ERROR("can not find fd, poll updated error");
    }

    return 0;
}


int poll_dispatch(struct event_loop *ev_loop, struct timeval *timeval) {
    struct poll_dispatcher_data *pdd = (struct poll_dispatcher_data *)ev_loop->event_dispatcher_data;

    int ready_num = 0;
    int timewait = timeval->tv_sec * 1000;

    if ((ready_num = poll(pdd->event_set, INIT_POLL_SIZE, timewait)) < 0) {
        error(1, errno, "poll failed.");
    }

    if (ready_num <= 0)
        return 0;

    int i;
    for (i = 0; i < INIT_POLL_SIZE; i ++) {
        int socket_fd;
        struct pollfd pollfd = pdd->event_set[i];
        if ((socket_fd = pollfd.fd) < 0)
            continue;

        if (pollfd.revents > 0) {
            lamp_msgx("get message channel i=%d, fd=%d, %s", i, socket_fd, ev_loop->thread_name);

            if (pollfd.revents & POLLRDNORM) {
                channel_event_active(ev_loop, socket_fd, EVENT_READ);
            }

            if (pollfd.revents & POLLWRNORM) {
                channel_event_active(ev_loop, socket_fd, EVENT_WRITE);
            }

            if (--ready_num <= 0)
                break;
        }
    }

    return 0;
}

void poll_clear(struct event_loop *ev_loop) {
    struct poll_dispatcher_data *pdd = (struct poll_dispatcher_data *) ev_loop->event_dispatcher_data;

    free(pdd->event_set);
    pdd->event_set = NULL;
    free(pdd);
    ev_loop->event_dispatcher_data = NULL;

    return;
}