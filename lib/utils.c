//
// Created by HEADS on 2021/2/20.
//

#include "utils.h"
#include "log.h"

void assert_in_same_thread(struct event_loop *ev_loop) {
    if (ev_loop->owner_thread_id != pthread_self()) {
        LOG_ERROR("not in the same thread");
        exit(-1);
    }
}

int is_in_same_thread(struct event_loop *ev_loop) {
    return ev_loop->owner_thread_id == pthread_self();
}