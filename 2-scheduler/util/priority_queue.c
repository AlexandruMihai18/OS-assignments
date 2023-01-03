// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

#include "priority_queue.h"
#include "utils.h"

priority_queue_t *create_pq(unsigned int size)
{
	priority_queue_t *new_list = malloc(sizeof(priority_queue_t));

	DIE(!new_list, "list malloc failed\n");

	new_list->size = size;
	new_list->nodes = malloc(sizeof(priority_queue_node_t) * size);
	DIE(!new_list->nodes, "nodes array malloc failed\n");

	for (int i = 0; i < size; i++) {
		(new_list->nodes)[i].priority = i;
		(new_list->nodes)[i].data = q_create();
	}
	return new_list;
}

queue_t *q_create(void)
{
	queue_t *new_queue = malloc(sizeof(queue_t));

	DIE(!new_queue, "queue malloc failed\n");

	new_queue->size = 0;
	new_queue->tail = NULL;
	return new_queue;
}

void q_enqueue(priority_queue_t *list, void *data, unsigned int priority)
{
	queue_t *q = list->nodes[priority].data;

	queue_node_t *q_node = malloc(sizeof(queue_node_t));

	DIE(!q_node, "queue node malloc failed\n");

	q_node->data = data;

	if (!q->size) {
		q_node->next = q_node;
		q->tail = q_node;
		q->size++;
		return;
	}

	q_node->next = q->tail->next;
	q->tail->next = q_node;
	q->tail = q_node;
	q->size++;
}

queue_node_t *q_peek(priority_queue_t *list)
{
	int priority = list->size - 1;

	while (priority >= 0) {
		if (((queue_t *)list->nodes[priority].data)->size)
			break;
		priority--;
	}

	if (priority == -1)
		return NULL;

	queue_t *q = list->nodes[priority].data;

	queue_node_t *removed = q->tail->next;
	return removed;
}

queue_node_t *q_dequeue(priority_queue_t *list)
{
	int priority = list->size - 1;

	while (priority >= 0) {
		if (((queue_t *)list->nodes[priority].data)->size)
			break;
		priority--;
	}

	if (priority == -1)
		return NULL;

	queue_t *q = list->nodes[priority].data;

	queue_node_t *removed = q->tail->next;

	q->size--;

	if (!q->size) {
		q->tail = NULL;
		return removed;
	}

	q->tail->next = removed->next;

	return removed;
}

void free_node(queue_node_t **node)
{
	free(*node);
}

void free_queue(queue_t **q)
{

	queue_node_t *current = NULL;

	if ((*q)->tail)
		current = (*q)->tail->next;

	queue_node_t *removed;

	while ((*q)->size) {
		removed = current;
		current = current->next;
		(*q)->size--;
		free_node(&removed);
	}

	free(*q);
}

void free_all(priority_queue_t **list)
{
	int priority = (*list)->size - 1;

	while (priority >= 0) {
		free_queue((queue_t **)(&((*list)->nodes[priority].data)));
		priority--;
	}

	free((*list)->nodes);
	free((*list));
}
