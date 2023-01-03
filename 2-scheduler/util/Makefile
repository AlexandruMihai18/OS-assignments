CC = gcc
CFLAGS = -fPIC -Wall -g

.PHONY: build
build: libscheduler.so

libscheduler.so: so_scheduler.o priority_queue.o linked_list.o thread.o
	$(CC) -shared -o $@ $^

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -o $@ -c $<

thread.o: thread.c
	$(CC) $(CFLAGS) -o $@ -c $<

priority_queue.o: priority_queue.c
	$(CC) $(CFLAGS) -o $@ -c $<

linked_list.o: linked_list.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f linked_list.o priority_queue.o so_scheduler.o libscheduler.so thread.o
