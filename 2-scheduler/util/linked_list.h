/* SPDX-License-Identifier: MIT */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

typedef struct node_t node_t;
struct node_t {
	node_t *next;
	void *data;
};

typedef struct list_t list_t;
struct list_t {
	node_t *head;
	unsigned int size;
};

list_t *create_list(void);

void add_last(list_t *list, void *data);

node_t *remove_node(list_t *list, int index);

unsigned int size(list_t *list);

void free_list(list_t **list, void free_data(void **));

#endif /* LINKED_LIST_H_ */
