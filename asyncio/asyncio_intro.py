import asyncio

from datetime import datetime


async def coro_sleep(seconds):
    """
    This coroutine returns after sleeping for a number of `seconds`
    """
    print(f'Sleeping for {seconds}')
    await asyncio.sleep(seconds)


async def coro_nested():
    print('Nested coroutine')


async def coro_func():
    """
    This is a cororoutine that just executes a single statement and behaves like a normal subroutine when executed.
    """
    print('Hello, asyncio')
    
    # This launches another coroutine and waits til it yields to continue execution flow from here
    await coro_nested()

    # Let's create three tasks that are to be executed concurrently using the asyncio event loop.
    sleep1 = coro_sleep(1)
    sleep2 = coro_sleep(2)
    sleep3 = coro_sleep(3)
    
    # Gathering tasks allows the underlying event loop that is executing the coroutines to perform co-operative 
    # multitasking. In this example the three sleep routines are executed concurrently so the total execution time
    # should be just longer than the 3 seconds of the longest coroutine.
    start = datetime.now()

    # asyncio.gather returns a future and we await
    await asyncio.gather(sleep1, sleep2, sleep3)
    duration = datetime.now() - start
    print(f'Execution time was {duration.total_seconds()} s')


if __name__ == '__main__':

    # This creates the coroutine object which maintains the state of the coroutine.
    coro_obj = coro_func()

    # We can execute a coroutine from a script using the asyncio.run(..) function.
    asyncio.run(coro_obj)