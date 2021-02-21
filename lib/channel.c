//
// Created by HEADS on 2021/2/20.
//

#include "channel.h"


struct channel* channel_new(int fd, int events, event_read_callback event_read_callback_func, event_write_callback event_write_callback_func, void *data) {
    struct channel *chan = malloc(sizeof(struct channel));

    chan->fd = fd;
    chan->events = events;
    chan->event_read_callback_func = event_read_callback_func;
    chan->event_write_callback_func = event_write_callback_func;
    chan->data = data;

    return chan;
}

int channel_write_event_is_enabled(struct channel *ch) {
    return ch->events & EVENT_WRITE;
}

int channel_write_event_enable(struct channel *ch) {
    struct event_loop *ev_loop = (struct event_loop *) ch->data;

    ch->events = ch->events | EVENT_WRITE;
    event_loop_update_channel_event(ev_loop, ch->fd, ch);

    return  0;
}

int channel_write_event_disable(struct channel *ch) {
    struct event_loop *ev_loop = (struct event_loop *) ch->data;

    ch->events = ch->events & ~EVENT_WRITE;
    event_loop_update_channel_event(ev_loop, ch->fd, ch);

    return 0;
}