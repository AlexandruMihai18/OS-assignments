// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "exec_parser.h"

struct page_node_t {
	struct page_node_t *next;
	void *addr;
} page_node_t;

static so_exec_t *exec;

struct sigaction old_action;

int fd;

/* checking if the page is already virtually mapped */
int find_page(void *addr, so_seg_t *segment)
{
	/* the data field is pointing to the first node of the list */
	struct page_node_t *current = (struct page_node_t *)(segment->data);

	while (current) {
		if (addr == current->addr)
			return 1;
		current = current->next;
	}

	return 0;
}

/* adding the new page as the last node of a list */
void add_page(void *addr, so_seg_t *segment)
{
	struct page_node_t *new_page = malloc(sizeof(page_node_t));

	new_page->addr = addr;
	new_page->next = NULL;

	if (!segment->data) {
		segment->data = new_page;
		return;
	}

	struct page_node_t *current = (struct page_node_t *)(segment->data);

	while (current->next)
		current = current->next;

	current->next = new_page;
}

/* finding the segment inside the ELF */
so_seg_t *find_segment(void *addr)
{
	int offset;

	for (int i = 0;  i < exec->segments_no; i++) {
		offset = (char *)addr - (char *)exec->segments[i].vaddr;
		if (offset >= 0 && offset < exec->segments[i].mem_size)
			return &(exec->segments[i]);
	}

	return NULL;
}

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	so_seg_t *segment = find_segment(info->si_addr);

	/* segment does not exist --> Seg fault! */
	if (!segment) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	size_t offset = (char *)info->si_addr - (char *)segment->vaddr;
	size_t page_size = getpagesize();
	size_t page_offset = offset - offset % page_size;

	/* page is already mapped */
	if (find_page((void *)segment->vaddr + page_offset, segment)) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	/* virtially mapping the new page */
	void *page_address = mmap((void *)segment->vaddr + page_offset, page_size, PERM_W, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, fd, 0);

	lseek(fd, segment->offset + page_offset, SEEK_SET);
	memset(page_address, 0, page_size);

	/* the page totally exceeds the file size --> map with zeros */
	if (segment->file_size >= page_offset) {
		/* the page partially exceeds the file size,
		 *  --> mark only a part
		 */
		if (segment->file_size < page_offset + page_size)
			read(fd, page_address, segment->file_size - page_offset);
		else
			read(fd, page_address, page_size);
	}

	/* marking the page as mapped */
	add_page(page_address, segment);

	/* adding protection layer */
	mprotect(page_address, page_size, segment->perm);
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	fd = open(path, O_RDONLY);

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);
	return -1;
}
