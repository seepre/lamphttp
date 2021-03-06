//
// Created by HEADS on 2021/2/20.
//

#ifndef UTILS_H
#define UTILS_H

#include "event_loop.h"

struct event_loop;

void assert_in_same_thread(struct event_loop *ev_loop);

// 1-same thread  0-not the same thread
int is_in_same_thread(struct event_loop *ev_loop);

#endif //UTILS_H
