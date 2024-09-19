# io_multiplexing

## select

You pass `select` sets of file descriptors to monitor for reading, writing, and exceptional conditions. You also pass
it the maximum file descirptor number and it seaches through all numbers up to that maximum numbter for files that are 
ready for IO. This means if you only have a few sparsely spaced file descriptor numbers, you spend a lot of time 
checking IO that you are not interested in. It is suggested that modern application should use `poll` or `epoll`
instead.

Other features of `select`:
* It provides a timeout feature.
* There is a variant called `pselect` that is used to wait on signals and files.

To quote the man page <https://man7.org/linux/man-pages/man2/select.2.html>

```
WARNING: select() can monitor only file descriptors numbers that are less than FD_SETSIZE (1024)—an unreasonably low 
limit for many modern applications—and this limitation will not change. All modern applications should instead use 
poll(2) or epoll(7), which do not suffer this limitation.

select() allows a program to monitor multiple file descriptors, waiting until one or more of the file descriptors 
become "ready" for some class of I/O operation (e.g., input possible).  A file descriptor is considered ready if it is 
possible to perform a corresponding I/O operation (e.g., read(2), or a sufficiently small write(2)) without blocking.
```

## poll

`poll` in may ways is based on the same logic as select, but rather iterate through all file descriptor up to a 
maximum limit, it just iterates through a explict list of file descriptors. For a small number of descriptors, 
this is likely the best approach.

## epoll

As the number of file descriptors increases, `poll` slows down since we iterate through all descriptors. Instead, we'd 
like the operating system to actively keep a list of ready desciptors so when we do a system call, we already have 
the information ready. We use `epoll_create` to create an in kernal container containing file descriptors we want
to monitor and file descriptors that are ready. The kernal places references to the ready file descriptors in the 
ready list for us.

`epoll_ctrl` is used to add file descriptors to the interest list of the `epoll` instance. `epoll_wait` is used
to block for IO from registered list of file descriptors.

`epoll` has the concept of level-triggered vs edge-triggered. With level-trigger, the file descriptor will be on the
ready list of you can ready data from it. With edge-trigger, when a file descriptor moves from having no data to having
data it is placed on the ready list. If you only partially consume data from it, it is removed from the ready list
despite having additional data to read from it. The man file `https://man7.org/linux/man-pages/man7/epoll.7.html`
suggested you only use edge triggered with nonblocking file descriptors, and you need to consume all data
from them (ie. read or write returns EAGAIN) before calling `epoll_wait` again.

## References

<https://jvns.ca/blog/2017/06/03/async-io-on-linux--select--poll--and-epoll/>

<https://hechao.li/2022/01/04/select-vs-poll-vs-epoll/>

## Build Instructions

```bash
cmake --preset debug
cd build-debug
ninja
```
