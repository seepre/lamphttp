//
// Created by HEADS on 2021/2/20.
//

#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "log.h"
#include "event_dispatcher.h"
#include "channel.h"
#include "utils.h"
#include "channel_map.h"


// i/o thread
int event_loop_handle_pending_channel(struct event_loop *ev_loop) {
    pthread_mutex_lock(&ev_loop->mutex);
    ev_loop->is_handle_pending = 1;

    struct channel_elemnt *channel_elm = ev_loop->pending_head;
    while(channel_elm != NULL) {
        struct channel *ch = channel_elm->channel;
        int fd = ch->fd;
        if (channel_elm->type == 1) {
            event_loop_handle_pending_add(ev_loop, fd, ch);
        } else if (channel_elm->type == 2) {
            event_loop_handle_pending_remove(ev_loop, fd, ch);
        } else if (channel_elm->type == 3) {
            event_loop_handle_pending_update(ev_loop, fd, ch);
        }
        channel_elm = channel_elm->next;
    }

    ev_loop->pending_head = ev_loop->pending_tail = NULL;
    ev_loop->is_handle_pending = 0;

    pthread_mutex_unlock(&ev_loop->mutex);

    return 0;
}

void event_loop_channel_buffer_nolock(struct event_loop *ev_loop, int fd, struct channel *ch, int type) {
    struct channel_elemnt *channel_elm = malloc(sizeof(struct channel_elemnt));

    channel_elm->channel = ch;
    channel_elm->type = type;
    channel_elm->next = NULL;

    // first element
    if (ev_loop->pending_head == NULL) {
        ev_loop->pending_head = ev_loop->pending_tail = channel_elm;
    } else {
        ev_loop->pending_tail->next = channel_elm;
        ev_loop->pending_tail = channel_elm;
    }
}

int event_loop_do_channel_event(struct event_loop *ev_loop, int fd, struct channel *ch, int type) {
    pthread_mutex_lock(&ev_loop->mutex);
    assert(ev_loop->is_handle_pending == 0);
    event_loop_channel_buffer_nolock(ev_loop, fd, ch, type);
    pthread_mutex_unlock(&ev_loop->mutex);

    if (!is_in_same_thread(&ev_loop)) {
        event_loop_wakeup(ev_loop);
    } else {
        event_loop_handle_pending_channel(ev_loop);
    }
    return 0;
}

int event_loop_add_channel_event(struct event_loop *ev_loop, int fd, struct channel *ch) {
    return event_loop_do_channel_event(ev_loop, fd, ch, 1);
}

int event_loop_remove_channel_event(struct event_loop *ev_loop, int fd, struct channel *ch) {
    return event_loop_do_channel_event(ev_loop, fd, ch, 2);
}

int event_loop_update_channel_event(struct event_loop *ev_loop, int fd, struct channel *ch) {
    return event_loop_do_channel_event(ev_loop, fd, ch, 3);
}

int event_loop_handle_pending_add(struct event_loop *ev_loop, int fd, struct channel *ch) {
    lamp_msgx("add channel fd == %d, %s", fd, ev_loop->thread_name);

    struct channel_map *map = ev_loop->channelMap;

    if (fd < 0)
        return 0;

    if (fd >= map->nentries)
        if (map_make_space(map, fd, sizeof(struct channel *)) == -1)
            return -1;


    if ((map)->entries[fd] == NULL) {
        map->entries[fd] = ch;

        struct event_dispatcher *ev_dis = ev_loop->event_dispatcher;
        ev_dis->add(ev_loop, ch);
        return 1;
    }

    return 0;
}

int event_loop_handle_pending_remove(struct event_loop *ev_loop, int fd, struct channel *ch) {
    struct channel_map *map = ev_loop->channelMap;

    assert(fd == ch->fd);

    if (fd < 0)
        return 0;

    if (fd >= map->nentries)
        return -1;

    struct channel *ch2 = map->entries[fd];

    struct event_dispatcher *ev_dis = ev_loop->event_dispatcher;

    int retval = 0;
    if (ev_dis->del(ev_loop, ch2) == -1) {
        retval = -1;
    } else {
        retval = 1;
    }

    map->entries[fd] = NULL;

    return retval;
}

int event_loop_handle_pending_update(struct event_loop *ev_loop, int fd, struct channel *ch) {
    lamp_msgx("update channel fd =%d, %s", fd, ev_loop->thread_name);

    struct channel_map *map = ev_loop->channelMap;

    if (fd < 0)
        return 0;

    if ((map)->entries[fd] == NULL)
        return -1;

    struct event_dispatcher *ev_dis = ev_loop->event_dispatcher;
    ev_dis->update(ev_loop, ch);
}

int channel_event_active(struct event_loop *ev_loop, int fd, int events) {
    struct channel_map *map = ev_loop->channelMap;

    lamp_msgx("activate channel fd = %d, revenst=%d, %s", fd, events, ev_loop->thread_name);

    if (fd < 0)
        return 0;

    if (fd >= map->nentries)
        return -1;

    struct channel *ch = map->entries[fd];
    assert(fd == ch->fd);

    if (events & (EVENT_READ)) {
        if (ch->event_read_callback_func)
            ch->event_read_callback_func(ch->data);
    }

    if (events & (EVENT_WRITE)) {
        if (ch->event_write_callback_func) ch->event_write_callback_func(ch->data);
    }

    return 0;
}

void event_loop_wakeup(struct event_loop *ev_loop) {
    char one = 'a';
    ssize_t n = write(ev_loop->socket_pair[0], &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("wakeup event loop thread failed.");
    }
}

int handle_wakeup(void *data) {
    struct event_loop *ev_loop = (struct event_loop *) data;
    char one;
    ssize_t n = read(ev_loop->socket_pair[1], &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("handle wakeup failed.");
    }
    lamp_msgx("wakeup, %s", ev_loop->thread_name);
}

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *ev_loop = malloc(sizeof(struct event_loop));

    pthread_mutex_init(&ev_loop->mutex, NULL);
    pthread_cond_init(&ev_loop->cond, NULL);

    if (thread_name != NULL) {
        ev_loop->thread_name = thread_name;
    } else {
        ev_loop->thread_name = "main thread";
    }

    ev_loop->quit = 0;
    ev_loop->channelMap = malloc(sizeof(struct channel_map));
    map_init(ev_loop->channelMap);

#ifdef EPOLL_ENABLE
    lamp_msgx("set epoll as dispatcher, %s", ev_loop->thread_name);
    ev_loop->event_dispatcher = &epoll_dispatcher;
#else
    lamp_msgx("set poll as dispatcher, %s", ev_loop->thread_name);
    ev_loop->event_dispatcher = &poll_dispatcher;
#endif
    ev_loop->event_dispatcher_data = ev_loop->event_dispatcher->init(ev_loop);

    ev_loop->owner_thread_id = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ev_loop->socket_pair) < 0) {
        LOG_ERROR("socketpair set failed.");
    }

    ev_loop->is_handle_pending = 0;
    ev_loop->pending_head = NULL;
    ev_loop->pending_tail = NULL;

    struct channel *ch = channel_new(ev_loop->socket_pair[1], EVENT_READ, handle_wakeup, NULL, ev_loop);
    event_loop_add_channel_event(ev_loop, ev_loop->socket_pair[1], ch);

    return ev_loop;
}

int event_loop_run(struct event_loop *ev_loop) {
    assert(ev_loop != NULL);

    struct event_dispatcher *dispatcher = ev_loop->event_dispatcher;

    if (ev_loop->owner_thread_id != pthread_self()) {
        exit(1);
    }

    lamp_msgx("event loop run, %s", ev_loop->thread_name);
    struct timeval timeval;
    timeval.tv_sec = 1;

    while(!ev_loop->quit) {
        dispatcher->dispatch(ev_loop, &timeval);

        event_loop_handle_pending_channel(ev_loop);
    }

    lamp_msgx("event loop end, %s", ev_loop->thread_name);
    return 0;
}