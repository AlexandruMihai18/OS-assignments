// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"
#include "utils.h"

list_t *create_list(void)
{
	list_t *new_list = malloc(sizeof(list_t));

	DIE(!new_list, "list malloc failed\n");

	new_list->head = NULL;
	new_list->size = 0;
	return new_list;
}

void add_last(list_t *list, void *data)
{
	node_t *new_node = malloc(sizeof(node_t));

	DIE(!new_node, "node malloc failed\n");

	new_node->data = data;
	new_node->next = NULL;
	list->size++;

	if (list->head == NULL) {
		list->head = new_node;
		return;
	}

	node_t *current = list->head;

	while (current->next)
		current = current->next;

	current->next = new_node;
}

node_t *remove_node(list_t *list, int index)
{
	DIE(index >= list->size, "index exceeds boundaries\n");

	node_t *removed = list->head;

	if (index == 0) {
		list->head = removed->next;
		return removed;
	}

	node_t *prev;

	for (int i = 0; i < index; i++) {
		prev = removed;
		removed = removed->next;
	}

	prev->next = removed->next;

	return removed;
}

unsigned int size(list_t *list)
{
	return list->size;
}

void free_list(list_t **list, void free_data(void **))
{
	node_t *current = (*list)->head;
	node_t *prev;

	while (current) {
		prev = current;
		current = current->next;
		free_data(&(prev->data));
		free(prev);
	}

	free(*list);
}
