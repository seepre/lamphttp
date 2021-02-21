//
// Created by HEADS on 2021/2/20.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "event_loop.h"
#include "event_loop_thread.h"

struct thread_pool {
    struct event_loop *main_loop; // create main thread for pool
    int started; // is started
    int thread_number;  // thread number
    struct event_loop_thread *ev_loop_ts; // arr pointer to loop thread
    int position; // event_loop position
};

struct thread_pool *thread_pool_new(struct event_loop *main_loop, int thread_num);

void thread_pool_start(struct thread_pool *);

struct event_loop *thread_pool_get_loop(struct thread_pool *);

#endif //THREAD_POOL_H
