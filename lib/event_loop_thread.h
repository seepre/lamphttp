//
// Created by HEADS on 2021/2/20.
//

#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include <pthread.h>

struct event_loop_thread {
    struct event_loop *ev_loop;
    pthread_t thread_tid;
    pthread_mutex_t  mutex;
    pthread_cond_t  cond;
    char *thread_name;
    long thread_count;
};

// init event_loop_thread witch malloced memory
int event_loop_thread_init(struct event_loop_thread *, int);

// run event_loop by sub thread from main thread
struct event_loop *event_loop_thread_start(struct event_loop_thread *);


#endif //EVENT_LOOP_THREAD_H
