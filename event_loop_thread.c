//
// Created by HEADS on 2021/2/20.
//

#include <assert.h>
#include "event_loop_thread.h"
#include "event_loop.h"

void *event_loop_thread_run(void *arg) {
    struct event_loop_thread *ev_loop_t = (struct event_loop_thread *) arg;

    pthread_mutex_lock(&ev_loop_t->mutex);

    ev_loop_t->ev_loop = event_loop_init_with_name(ev_loop_t->thread_name);
    lamp_msgx("event loop thread init and signal, %s", ev_loop_t->thread_name);

    pthread_cond_signal(&ev_loop_t->cond);

    pthread_mutex_unlock(&ev_loop_t->mutex);

    event_loop_run(ev_loop_t->ev_loop);
}

int event_loop_thread_init(struct event_loop_thread *ev_loop_t, int i) {
    pthread_mutex_init(&ev_loop_t->mutex, NULL);
    pthread_cond_init(&ev_loop_t->cond, NULL);

    ev_loop_t->ev_loop = NULL;
    ev_loop_t->thread_count = 0;
    ev_loop_t->thread_tid = 0;

    char *buf = malloc(16);
    sprintf(buf, "Thread-%d\0", i + 1);
    ev_loop_t->thread_name = buf;

    return 0;
}

struct event_loop *event_loop_thread_start(struct event_loop_thread *ev_loop_t) {
    pthread_create(&ev_loop_t->thread_tid, NULL, &event_loop_thread_run, ev_loop_t);

    assert(pthread_mutex_lock(&ev_loop_t->mutex) == 0);

    while(ev_loop_t->ev_loop == NULL) {
        assert(pthread_cond_wait(&ev_loop_t->cond, &ev_loop_t->mutex) == 0);
    }
    assert(pthread_mutex_unlock(&ev_loop_t->mutex) == 0);

    lamp_msgx("event loop thread started, %s", ev_loop_t->thread_name);
    return ev_loop_t->ev_loop;
}