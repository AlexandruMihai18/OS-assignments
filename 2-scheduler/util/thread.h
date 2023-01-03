/* SPDX-License-Identifier: MIT */

#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <semaphore.h>

#include "so_scheduler.h"

#define INVALID_DEVICE -1

typedef enum {NEW, READY, RUNNING, WAITING, TERMINATED} thread_state;

typedef struct thread_t thread_t;
struct thread_t {
	tid_t tid;
	thread_state state;
	so_handler *handler;

	unsigned int time;
	unsigned int priority;
	int waiting_io;

	sem_t is_running;
};

thread_t *create_thread(void *func, unsigned int priority);

void free_thread(void **args);

#endif /* THREAD_H_ */
