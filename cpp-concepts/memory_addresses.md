# Memory Management

To allow multiple processes to run on the same machine at once, we have two problems to solve - protection and 
relocation. 

## Memory Abstractions

To address these two issues each process runs in its own memory abstration called a virtual memory address space as 
opposed to in the physical memory of the machine. A process cannot alter the virtual address space of other processes
and the operating system, using CPU features, maintains seperation.

Physical memory addresses (PMA) are provided by the machine. There is only one PMA space per machine. The entire
address space may not be available to the process as addresses may be used by devices or the OS.

Virtual memory addresses (VMA) or logical memory addresses are provided by the OS to a process. There is one VMA space
per process. A VMA space is split into several segments.

## Address Space Translation

There are multiple methods to perform this task. Some are simple but obsolete due to performance issues.

### Dynamic Relocation

Programs are loaded into memory a various locations and the processor contains two registers: `base` and `limit`. When 
the operating system switches to a different process it loads the location of the start of the processes virtual memory
address into the `base` register and the length of the virtual memory space in the `limit` register. The CPU would
automatically add the `base` value to any instruction accessing memory. A fault is generated if the access is beyond
the limit of the virtual address space. This disadvantage of this approach was all the additional additions and 
comparisons the 

### Swapping

Generally the RAM required by all active processes is greater than what is physcally available. One strategy is 
to bring the entire process into RAM for a given amount of time, before writing  its address space to disk. This is
swapping. 

It has a number of disadvantages including fragmenting memory, not necessarily allowing a virtual address space to 
grow if there is no space adjacent to the process address space in physical memory, and the entire process must be in 
memory. A page table is kept mapping the pages of the virtual address space into physical address space so the
appropriate offset can be added to addresses in a given virtual page to the equivalent page in physical memory.

## Virtual Memory

Each process has its own memory address space and this is broken chuncks called pages. A page contains a set of 
contigious memory addresses. The CPU maps virtual addresses to physical addresses on the fly. If a page containing
a requested memory address is not in memory, the OS is alerted and it must load the page into memory. This situation
is called a page fault.

Process generally operate in the virtual address space - examples that don't can include OSes and bare metal 
applications. If using physical addresses the address is sent directly to the memory bus for instruction or load/stores.
If using virtual memory, the address is sent to an MMU that does the translation. There is a table detailing which 
virtual pages are in physical memory and where.

### Page Tables

Not only do page tables contain the mapping from virtual to physical memory and if the page is in physial memory - 
it can contain a read/write or read only bit, whether it has been written too (you don't have to store it if not),
wether it has been read, and whether caching is enabled for this piece of memory.

Each process has its own page table. When a process starts, the OS writes the processes page table to the relevent
registers in the CPU. When not active, the page table is kept in main memory. Another way the page table can be stored
is having it permantly in memory, and there is one register that points to the start of the page table that the OS
writes to when it starts a process.

## malloc

You can use `sbrk` \ `brk` or `mmap` system calls to get more data from the operating system. The it is up to the 
implementation of malloc on how structure data in the memory given to it from

## Multi-Core OS

A process from the kernal boots on one core then spawns a process for all other cores found. The process on each core 
checks if a user thread is waiting to be executed. If so it runs it for a bit, else it waits for it to appear. Most
of the kernal sits in common memory so shared memory is used to transmit information between them. Certain functions
of the OS (like the scheduler) could execute only in one core. For initial booting processor 0 is just like a single 
core processor, it just starts executing from a given fixed memory address. Then there is a hardware mechanism for it
to start the other cores and what address they should start executing from.

## Reference

* Modern Operating Systems, Fourth Edition, Andrew S. Tanenbaum
