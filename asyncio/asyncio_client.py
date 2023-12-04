import asyncio
import datetime
import logging
import threading
import time


logger = logging.getLogger(__name__)


class Client:
    """
    This client is going to listen to a flaky server that is sending it messages. We are going to write a robust
    client that performs the network IO in a background thread.
    """

    def __init__(self, use_background_thread: bool = True):
        self.msgs = []

        if use_background_thread:
            self.loop = asyncio.new_event_loop()
            self.thread = threading.Thread(target=self.start_background_loop, args=(self.loop,), daemon=True)
            self.thread.start()


    async def send_loop(self, reader):
        while True:
            pass


    async def recv_loop(self, writer):
        while True:
            pass


    async def main(self, host: str, port: int):
        reader, writer = asyncio.open_connection(host, port)
        await asyncio.gather(self.send_loop(writer), self.recv_loop(reader))
        writer.close()
        await writer.wait_closed()




    def start_background_loop(loop: asyncio.AbstractEventLoop) -> None:
        """
        Only one asyncio event loop can be run per thread. This function is a target of a thread and assigns a given
        loop to this thread that then runs forever.
        """
        logger.info("Starting background loop")
        asyncio.set_event_loop(loop)
        loop.run_forever()
        logger.info("Exiting background loop")


    def start_client(self) -> None:
        """
        When it comes to threads - we will try to terminate them gracefully. If they don't terminate after a timeout
        we will terminate them ungracefully. If the main thread exits - we want all threads to terminate so the 
        process doesn't hang. So always use daemon threads, but don't rely on just exiting the main thread to 
        terminate them.

        
        """

        start_time = datetime.now()

        # Adds a coroutine
        future = asyncio.run_coroutine_threadsafe(self.main(), self.loop)
        for url, status_code in future.result():
            print(f"{url} -> {status_code}")

        exec_time = (datetime.now() - start_time).total_seconds()
        print(f"It took {exec_time:,.2f} seconds to run")
        loop.stop()


if __name__ == "__main__":
    client = Client()
    client.start_client()
    while True:
        time.sleep(1.0)