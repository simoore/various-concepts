On ubuntu,

```bash
sudo apt install libboost-dev
```

On windows I'm using clang 18. These examples use header only liraries from boost 1.87 downloaded from,

<https://www.boost.org/users/download/>

I was having some trouble with TCP server sockets and the documentation notes only client side TCP is supported
on windows <https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/overview/implementation.html>
