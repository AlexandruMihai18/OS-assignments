/* SPDX-License-Identifier: MIT */

#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

typedef struct priority_queue_node_t priority_queue_node_t;
struct priority_queue_node_t {
	unsigned int priority;
	void *data;
};

typedef struct priority_queue_t priority_queue_t;
struct priority_queue_t {
	priority_queue_node_t *nodes;
	unsigned int size;
};

typedef struct queue_node_t queue_node_t;
struct queue_node_t {
	queue_node_t *next;
	void *data;
};

typedef struct queue_t queue_t;
struct queue_t {
	queue_node_t *tail;
	unsigned int size;
};

priority_queue_t *create_pq(unsigned int size);

queue_t *q_create(void);

void q_enqueue(priority_queue_t *list, void *data, unsigned int priority);

queue_node_t *q_peek(priority_queue_t *list);

queue_node_t *q_dequeue(priority_queue_t *list);

void free_node(queue_node_t **node);

void free_queue(queue_t **q);

void free_all(priority_queue_t **list);

#endif /* PRIORITY_QUEUE_H_ */
