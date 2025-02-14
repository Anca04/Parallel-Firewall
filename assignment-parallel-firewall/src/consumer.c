// SPDX-License-Identifier: BSD-3-Clause

#include "consumer.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log/log.h"
#include "packet.h"
#include "ring_buffer.h"

void *consumer_thread(void *arg)
{
	so_consumer_ctx_t *ctx = (so_consumer_ctx_t *)arg;
	so_packet_t packet;
	ssize_t result;
	so_action_t action;
	unsigned long hash;
	unsigned long timestamp;

	while (1) {
		result = ring_buffer_dequeue(ctx->producer_rb, &packet,
				sizeof(so_packet_t));

		if (result == -1 && ctx->producer_rb->stopped == 1 &&
			ctx->producer_rb->len == 0)
			break;

		if (result == 0)
			continue;

		action = process_packet(&packet);
		hash = packet_hash(&packet);
		timestamp = packet.hdr.timestamp;

		char out_buf[256];
		int len = snprintf(out_buf, sizeof(out_buf), "%s %016lx %lu\n",
					RES_TO_STR(action), hash, timestamp);

		pthread_mutex_lock(&ctx->log_mutex);
		write(ctx->out_fd, out_buf, len);
		pthread_mutex_unlock(&ctx->log_mutex);
	}
	return NULL;
}

int create_consumers(pthread_t *tids, int num_consumers, so_ring_buffer_t *rb,
						const char *out_filename)
{
	int out_fd = open(out_filename, O_RDWR | O_CREAT | O_TRUNC, 0666);

	if (out_fd < 0)
		exit(-1);

	so_consumer_ctx_t *ctx = malloc(num_consumers * sizeof(so_consumer_ctx_t));

	if (ctx == NULL) {
		close(out_fd);
		return -1;
	}

	for (int i = 0; i < num_consumers; i++) {
		pthread_mutex_t log_mutex;

		pthread_mutex_init(&log_mutex, NULL);
		ctx[i].producer_rb = rb;
		ctx[i].out_fd = out_fd;
		ctx[i].log_mutex = log_mutex;
		ctx[i].out_filename = out_filename;

		int ret = pthread_create(&tids[i], NULL, consumer_thread, &ctx[i]);

		if (ret != 0) {
			close(out_fd);
			return -1;
		}
	}

	return num_consumers;
}
