// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"

#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "packet.h"

int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	if (!ring || cap == 0)
		return -1;

	ring->data = (char *)malloc(cap);
	if (!ring->data)
		return -1;

	ring->cap = cap;
	ring->len = 0;
	ring->read_pos = 0;
	ring->write_pos = 0;
	ring->stopped = 0;

	pthread_mutex_init(&ring->mutex, NULL);
	sem_init(&ring->full, 0, cap);
	sem_init(&ring->empty, 0, 0);

	return 0;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	if (!ring || !data || size == 0 || size > ring->cap)
		return -1;

	ssize_t packets_add = 0;

	while (ring->len == ring->cap)
		sem_wait(&ring->full);

	pthread_mutex_lock(&ring->mutex);

	size_t index = ring->write_pos % ring->cap;

	memcpy(ring->data + index, data, size);
	ring->write_pos = (ring->write_pos + size) % ring->cap;
	ring->len += size;
	packets_add++;

	pthread_mutex_unlock(&ring->mutex);
	sem_post(&ring->empty);

	return packets_add;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
	if (!ring || !data || size == 0 || size > ring->cap)
		return -1;

	ssize_t packets_read = 0;

	while (ring->len == 0 && ring->stopped == 0)
		sem_wait(&ring->empty);

	pthread_mutex_lock(&ring->mutex);

	if (ring->len < size && ring->stopped == 1) {
		pthread_mutex_unlock(&ring->mutex);
		return -1;
	}

	size_t index = ring->read_pos % ring->cap;

	memcpy(data, ring->data + index, size);
	ring->read_pos = (ring->read_pos + size) % ring->cap;
	ring->len -= size;
	packets_read++;

	pthread_mutex_unlock(&ring->mutex);
	sem_post(&ring->full);

	return packets_read;
}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{
	if (!ring)
		return;
	free(ring->data);
	pthread_mutex_destroy(&ring->mutex);
	sem_destroy(&ring->full);
	sem_destroy(&ring->empty);
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{
	if (!ring)
		return;
	pthread_mutex_lock(&ring->mutex);
	ring->stopped = 1;
	sem_post(&ring->empty);
	pthread_mutex_unlock(&ring->mutex);
}
