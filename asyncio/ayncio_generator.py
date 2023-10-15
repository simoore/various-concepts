"""
An asynchronous generator is a coroutine that uses the yield expression.

An asynchronous iterator is an iterator that yields awaitables. It defines two methods:
__anext__() returns the next awaitable.

Awaitables are coroutines, tasks, or futures.
"""

import asyncio

async def async_generator():
    for i in range(10):
        await asyncio.sleep(1)
        yield i

async def main_coro():
    async for item in async_generator():
        print(item)

if __name__ == "__main__":
    asyncio.run(main_coro())