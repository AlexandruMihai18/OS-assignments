// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>

#include "so_scheduler.h"
#include "priority_queue.h"
#include "linked_list.h"
#include "thread.h"
#include "utils.h"

struct scheduler_t {
	priority_queue_t *ready_queue; /* Priority queue for threads in READY state */
	list_t *all_threads; /* Linked list for all the threads forked */
	list_t *waiting_threads;
	thread_t *running_thread; /* The single thread that is using the CPU */

	unsigned int io; /* Number of io devices given */
	unsigned int time_quantum; /* Time given for each thread to execute until preempted */

	sem_t is_running; /* Semaphore that marks the execution of the scheduler */
};

struct scheduler_t *schedule_unit;

/* Mark a new thread as READY to be executed */
void mark_ready(thread_t **thread)
{
	(*thread)->time = 0;
	(*thread)->waiting_io = INVALID_DEVICE;
	(*thread)->state = READY;
	q_enqueue(schedule_unit->ready_queue, *thread, (*thread)->priority);
}

/* Changing the running thread with the highest priority thread from priority queue */
void change_running_thread(void)
{
	int ret;

	thread_t *running = schedule_unit->running_thread;

	queue_node_t *removed = q_dequeue(schedule_unit->ready_queue);

	thread_t *replace = (thread_t *)(removed->data);

	free(removed);

	/* Preemption of the running thread --> it goes back to READY */
	if (running && running->state != WAITING && running->state != TERMINATED)
		mark_ready(&schedule_unit->running_thread);

	schedule_unit->running_thread = replace;
	replace->state = RUNNING;

	/* Starting the new thread */
	ret = sem_post(&replace->is_running);
	DIE(ret, "current thread sempahore post failed\n");
}

/* Scheduling function in order to decide which thread runs next */
void scheduling(void)
{
	int ret;

	thread_t *running = schedule_unit->running_thread;

	/* Next thread to be executed */
	queue_node_t *top_node = q_peek(schedule_unit->ready_queue);

	/* No other threads are available */
	if (!top_node) {
		/* All threads are TERMINATED */
		if (running->state == TERMINATED) {
			ret = sem_post(&schedule_unit->is_running);
			DIE(ret, "schedule unit semaphore post failed\n");
		}

		/* Only the current thread is running */
		ret = sem_post(&running->is_running);
		DIE(ret, "running thread semaphore post failed\n");

		return;
	}

	/* There is no RUNNING thread (first thread to be executed)*/
	if (!running) {
		change_running_thread();
		return;
	}

	/* The thread was marked as WAITING */
	if (running->state == WAITING) {
		change_running_thread();
		return;
	}

	/* The thread was marked as TERMINATED */
	if (running->state == TERMINATED) {
		change_running_thread();
		return;
	}

	thread_t *current = (thread_t *)(top_node->data);

	/* Preemption: There is another thread with higher priority */
	if (running->priority < current->priority) {
		change_running_thread();
		return;
	}

	/* Preemption: The thread's CPU time has reach the limit */
	if (running->time >= schedule_unit->time_quantum) {
		/* Round Robin implementation --> the next thread with the same priority */
		if (running->priority == current->priority) {
			change_running_thread();
			return;
		}

		/* The running thread is the only one with the highest priority --> its quantum resets*/
		running->time = 0;
	}

	/* Mark the running thread as running again due to no preemption */
	ret = sem_post(&running->is_running);
	DIE(ret, "running thread semaphore post failed\n");
}

void *thread_routine(void *args)
{
	int ret;

	thread_t *thread = (thread_t *) args;

	ret = sem_wait(&thread->is_running);
	DIE(ret, "thread semaphore wait failed\n");

	thread->handler(thread->priority);

	thread->state = TERMINATED;

	scheduling();

	return NULL;
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	int ret;

	if (schedule_unit || io > SO_MAX_NUM_EVENTS || time_quantum <= 0)
		return -1;

	/* Initializing the scheduler */
	schedule_unit = malloc(sizeof(struct scheduler_t));
	DIE(!schedule_unit, "schedule unit malloc failed\n");

	schedule_unit->ready_queue = create_pq(SO_MAX_PRIO + 1);

	schedule_unit->all_threads = create_list();

	schedule_unit->waiting_threads = create_list();

	schedule_unit->running_thread = NULL;

	schedule_unit->io = io;
	schedule_unit->time_quantum = time_quantum;

	ret = sem_init(&schedule_unit->is_running, 0, 0);
	DIE(ret, "schedule unit semaphore init failed\n");

	ret = sem_post(&schedule_unit->is_running);
	DIE(ret, "schedule unit semaphore post failed\n");

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	int ret;

	if (!func || priority > SO_MAX_PRIO)
		return INVALID_TID;

	/* Creating a new thread and adding it to the priority queue */
	thread_t *new_thread = create_thread(func, priority);

	ret = pthread_create(&new_thread->tid, NULL, thread_routine, new_thread);
	DIE(ret, "pthread create on new thread failed\n");

	add_last(schedule_unit->all_threads, new_thread);
	q_enqueue(schedule_unit->ready_queue, new_thread, priority);

	/* First thread was initialized --> we marked the scheduler execution as waiting */
	if (!schedule_unit->running_thread) {
		ret = sem_wait(&schedule_unit->is_running);
		DIE(ret, "schedule unit semaphore wait failed\n");
	}

	new_thread->state = READY;

	/* If there another thread is running on the CPU it continues it execution until preempted */
	if (!schedule_unit->running_thread)
		scheduling();
	else
		so_exec();

	return new_thread->tid;
}

int so_wait(unsigned int io)
{
	if (io < 0 || io >= schedule_unit->io)
		return -1;

	/* Mark thread ad waiting on a specific device */
	schedule_unit->running_thread->state = WAITING;
	schedule_unit->running_thread->waiting_io = io;
	add_last(schedule_unit->waiting_threads, schedule_unit->running_thread);

	so_exec();

	return 0;
}

int so_signal(unsigned int io)
{
	int counter = 0;
	int step = 0;

	if (io < 0 || io >= schedule_unit->io)
		return -1;

	node_t *current = schedule_unit->waiting_threads->head;

	node_t *removed;

	thread_t *thread;

	/* Wake up all waiting thread from a specific device */
	while (current) {
		thread = (thread_t *)(current->data);
		if (thread->waiting_io == io) {
			removed = remove_node(schedule_unit->waiting_threads, step);
			thread = (thread_t *)(removed->data);

			current = removed->next;
			free(removed);

			mark_ready(&thread);
			counter++;
		} else {
			step++;
			current = current->next;
		}
	}

	so_exec();

	return counter;
}

void so_exec(void)
{
	int ret;

	thread_t *running = schedule_unit->running_thread;

	/* Marking the execution by increasing the time taken */
	running->time++;

	/* Running the scheduling algorithm after each step */
	scheduling();

	/* Mark thread as waiting if it's preempted */
	ret = sem_wait(&running->is_running);
	DIE(ret, "thread semaphore wait failed\n");
}

void so_end(void)
{
	if (!schedule_unit)
		return;

	int ret;

	ret = sem_wait(&schedule_unit->is_running);
	DIE(ret, "schedule unit semaphore wait failed\n");

	node_t *current = schedule_unit->all_threads->head;
	thread_t *thread;

	/* Waiting for all threads to finish their execution */
	while (current) {
		thread = (thread_t *)(current->data);

		ret =  pthread_join(thread->tid, NULL);
		DIE(ret, "pthread join failed\n");

		current = current->next;
	}

	/* Freeing the used resources and destroying all semaphores */
	free_all(&schedule_unit->ready_queue);

	free_list(&schedule_unit->waiting_threads, free_thread);

	free_list(&schedule_unit->all_threads, free_thread);

	ret = sem_destroy(&schedule_unit->is_running);
	DIE(ret, "schedule unit semaphore destroy failed\n");

	free(schedule_unit);
	schedule_unit = NULL;
}
