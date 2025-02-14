/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __SO_CONSUMER_H__
#define __SO_CONSUMER_H__

#include "packet.h"
#include "ring_buffer.h"

typedef struct so_consumer_ctx_t
{
    struct so_ring_buffer_t *producer_rb;

    /* TODO: add synchronization primitives for timestamp ordering */
    const char *out_filename;
    int out_fd;
    pthread_mutex_t log_mutex;
} so_consumer_ctx_t;

int create_consumers(pthread_t *tids, int num_consumers, so_ring_buffer_t *rb, const char *out_filename);
int get_out_fd(so_consumer_ctx_t *ctx);
void close_file(int out_fd);

#endif /* __SO_CONSUMER_H__ */
