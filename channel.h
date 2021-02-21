//
// Created by HEADS on 2021/2/20.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include "common.h"
#include "event_loop.h"
#include "buffer.h"

#define EVENT_TIMEOUT   0x01
#define EVENT_READ      0x02    // wait for a socket or FD to become readable
#define EVENT_WRITE     0x04    // wait for a socket or FD to become writeable
#define EVENT_SIGNAL    0x08    // wait for a POSIX signal to be raised

typedef int (*event_read_callback)(void *data);

typedef int (*event_write_callback)(void *data);

struct channel {
    int fd;
    int events;     // event type
    event_read_callback  event_read_callback_func;
    event_write_callback event_write_callback_func;
    void *data;     // callback data, maybe event_loop, maybe tcp_server or tcp_connection
};

struct channel* channel_new(int fd, int events, event_read_callback event_read_callback_func, event_write_callback event_write_callback_fund, void *data);

int channel_write_event_is_enabled(struct channel *ch);

int channel_write_event_enable(struct channel *ch);

int channel_write_event_disable(struct channel *ch);

#endif //CHANNEL_H
