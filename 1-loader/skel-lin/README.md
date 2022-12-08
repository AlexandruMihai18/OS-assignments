Nume: Alexandru Mihai
Grupa: 323CA

# Tema 1: ELF Executable Loader

# Organization

The loader file performs uploading an Executable and Linkable Format File (ELF)
and mapping it to the virtual memory page by page through a mechanism known
as *demand paging*.
The implementation is focused on resolving a page fault following a page fault
routine.

# Implementation

The SIGSEGV signal (marking a possible segmentation fault) is intercepted and 
has to undergo through a routine known as page fault routine in order to
resolve the issue.
Page fault has 3 possible outcomes:

* The address required is not valid and therefore represents an invalid memory
access, which would imply calling the old handler and result in a segmentation
fault (also known as major page fault) -- marked in the implementation by 
finding a NULL segment inside the ELF file.
* The address called has already been virtually mapped and does not require
another mapping in VAS (Virtual Address Space) and will produce a minor page
fault. -- marked in the implementation by searching throughout the linked list
that contains the already mapped pages.
* The address called is valid, however the required page has not been assigned
yet in VAS and will need to undergo to the demand paging mechanism.

A new page will be virtually mapped using mmap function and populated using the
data inside the ELF file. The mapped zone is initially marked with 0 and
consecutively populated using the read function from the given ELF file.
If a page is not fully populated (i.e the last page that needs to be mapped
with non NULL data) it will be partially occupied.
The already mapped pages of each segment are memorized inside the free so_seg_t
field called data. This field will contain a linked list with all the page
addresses that have already been mapped.


# Usage

In order to run the loader, we use the following command to build the library:

```
make
```

# Bibliography

* [Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)

* [Lab - Signals](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-04)

* [Lab - Memory Management](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-05)

* [Lab - Virtual Memory](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-06)
