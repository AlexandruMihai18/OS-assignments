// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

#include "thread.h"
#include "utils.h"

/* Initialize the internal structure of a thread */
thread_t *create_thread(void *func, unsigned int priority)
{
	thread_t *new_thread = malloc(sizeof(thread_t));

	DIE(!new_thread, "thread malloc failed\n");

	new_thread->state = NEW;
	new_thread->handler = func;
	new_thread->time = 0;
	new_thread->priority = priority;
	new_thread->waiting_io = INVALID_DEVICE;

	sem_init(&new_thread->is_running, 0, 0);

	return new_thread;
}

/* Destroy the thread semaphore and free it */
void free_thread(void **args)
{
	int ret;

	thread_t **thread = (thread_t **)args;

	ret = sem_destroy(&((*thread)->is_running));
	DIE(ret, "thread semaphore destroy failed\n");

	free(*thread);
}
