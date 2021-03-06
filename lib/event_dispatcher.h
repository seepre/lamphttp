//
// Created by HEADS on 2021/2/20.
//

#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include "channel.h"

struct event_loop;
struct timeval;
struct channel;

struct event_dispatcher {
    const char *name;

    void *(*init)(struct event_loop * a);

    // add channel event and notify dispatcher
    int (*add)(struct event_loop *, struct channel *ch);

    // delete channel event and notify dispatcher
    int (*del)(struct event_loop *, struct channel *ch);

    // update channel event and notify dispatcher
    int (*update)(struct event_loop *, struct channel *ch);

    // dispatcher and use event_loop callback use event_activate
    int (*dispatch)(struct event_loop *, struct timeval *);

    void (*clear)(struct event_loop *);
};

#endif //EVENT_DISPATCHER_H
