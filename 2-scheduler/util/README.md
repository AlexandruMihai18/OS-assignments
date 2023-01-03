Nume: Alexandru Mihai

*Grupa: 323 CA*

# Tema 2: Thread Scheduler

# Organization

The project is providing a thread scheduler implementation that follows a priority queue
and the Round Robin approach in order to achieve fairness for CPU usage between threads.

The  ``so_scheduler.c`` contains the implementation of the scheduler that performs
the scheduler structure initialization and destruction, as well as the forking,
interruption, recalling and execution of a thread following each thread routine through its
handler.

The assignment is proving useful considering the architecture that modern computers are build
upon (limitted CPU units and a lot of tasks that need to perform at the same time -- or at
least seem like they do). 

The impletation was build through a similar data structure as the Linux Kernel (using a priority
queue of threads based on buckets for each priority), however the overall project structure could
have been improved.

# Implementation

The queue of READY threads was implemented through a priority queue based on sorted buckets by
priority. Each bucket contains a circular queue that will update itself each time a thread is
enqueued or dequeued.

All threads were memorized through a linked list in order to be able to free all the used resources
at the end.

The waiting threads were added in the waiting thread list in order to preserve the order of the
waiting signal and be able to respect a certain order while being added back to the READY queue.

# Usage
In order to build the dynamic library, we use the following command:

```
make
```

This will generate the libscheduler.so library that will be further used in order to check the given tests.

# Bibliography

* [Homework assigment](https://ocw.cs.pub.ro/courses/so/teme/tema-4)
* [Threads - Lab](https://ocw.cs.pub.ro/courses/so/teme/tema-4)
* [Scheduling - Course](https://ocw.cs.pub.ro/courses/so/cursuri/curs-04)
* [Threads - Course](https://ocw.cs.pub.ro/courses/so/cursuri/curs-08)
* [Syncronization - Course](https://ocw.cs.pub.ro/courses/so/cursuri/curs-09)

