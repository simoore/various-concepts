import asyncio
from pprint import pprint
from threading import Thread


def exception_handler(loop, context):
    print('Exception handler called')
    pprint(context)


loop = asyncio.get_event_loop()

loop.set_exception_handler(exception_handler)

thread = Thread(target=loop.run_forever)
thread.start()


async def coro():
    print("coro")
    raise RuntimeError("BOOM!")


fut = asyncio.run_coroutine_threadsafe(coro(), loop)
try:
    print("success:", fut.result())
except:
    print("exception:", fut.exception())

loop.call_soon_threadsafe(loop.stop)

thread.join()