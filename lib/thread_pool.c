//
// Created by HEADS on 2021/2/20.
//

#include <assert.h>

#include "utils.h"
#include "thread_pool.h"

struct thread_pool *thread_pool_new(struct event_loop *main_loop, int thread_num) {

    struct thread_pool *thread_pool = malloc(sizeof(struct thread_pool));

    thread_pool->main_loop = main_loop;
    thread_pool->position = 0;
    thread_pool->thread_number = thread_num;
    thread_pool->started = 0;
    thread_pool->ev_loop_ts = NULL;

    return thread_pool;
}

void thread_pool_start(struct thread_pool *thread_pool) {
    assert(!thread_pool->started);
    assert_in_same_thread(thread_pool->main_loop);

    thread_pool->started = 1;
    if (thread_pool->thread_number <= 0) {
        return;
    }

    thread_pool->ev_loop_ts = malloc(thread_pool->thread_number * sizeof(struct event_loop_thread));
    int i;
    for (i = 0; i < thread_pool->thread_number; ++i) {
        event_loop_thread_init(&thread_pool->ev_loop_ts[i], i);
        event_loop_thread_start(&thread_pool->ev_loop_ts[i]);
    }
}

struct event_loop *thread_pool_get_loop(struct thread_pool *thread_pool) {
    assert(thread_pool->started);
    assert_in_same_thread(thread_pool->main_loop);

    struct event_loop *selected = thread_pool->main_loop;

    if (thread_pool->thread_number > 0) {
        selected = thread_pool->ev_loop_ts[thread_pool->position].ev_loop;
        if (++thread_pool->position >= thread_pool->thread_number) {
            thread_pool->position = 0;
        }
    }

    return selected;
}