/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef SO_RINGBUFFER_H
#define SO_RINGBUFFER_H

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>

typedef struct so_ring_buffer_t
{
    char *data;

    size_t read_pos;
    size_t write_pos;

    size_t len;
    size_t cap;

    /* Synchronization primitives */
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;

    int stopped;
} so_ring_buffer_t;

int ring_buffer_init(so_ring_buffer_t *rb, size_t cap);
ssize_t ring_buffer_enqueue(so_ring_buffer_t *rb, void *data, size_t size);
ssize_t ring_buffer_dequeue(so_ring_buffer_t *rb, void *data, size_t size);
void ring_buffer_destroy(so_ring_buffer_t *rb);
void ring_buffer_stop(so_ring_buffer_t *rb);

#endif /* SO_RINGBUFFER_H */
