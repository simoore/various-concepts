import asyncio
import logging
import threading
import time


logger = logging.getLogger(__name__)


class Client:
    """
    This client is going to listen to a flaky server that is sending it messages. We are going to write a robust
    client that performs the network IO in a background thread.

    TODO: Handling a crashed consumer
    TODO: Handling a crashed producer
    TODO: policy for recv or message loop task exiting
    """


    def __init__(self, use_background_thread: bool = True):
        """
        Creates the data structure to store received messages and creates the background loop if required.

        When it comes to threads - we will try to terminate them gracefully. If they don't terminate after a timeout
        we will terminate them ungracefully. If the main thread exits - we want all threads to terminate so the 
        process doesn't hang. So always use daemon threads, but don't only rely on just exiting the main thread to 
        terminate them.

        Parameters
        ----------
        use_background_thread
            True, if the client networking calls run in a background thread or in an event loop that will block 
            this thread when it starts running.
        """
        self.__msgs = []                    # Stores a list of parsed messages
        self.__msg_queue = asyncio.Queue()  # Passes message between recv loop and message loop

        if use_background_thread:
            self.__loop = asyncio.new_event_loop()
            self.__thread = threading.Thread(target=self.__start_background_loop, daemon=True)
            self.__thread.start()


    ###########################################################################
    # PRIVATE FUNCTIONS
    ###########################################################################


    def __start_background_loop(self) -> None:
        """
        Only one asyncio event loop can be run per thread. This is a thread target function for a thread created in 
        the constructor. It assigns an event loop to this thread that then runs forever. If you want you can call 

            self.loop.call_soon_threadsafe(self.loop.stop)

        to cancel the loop and exit the thread.
        """
        logger.info("Starting background loop")
        asyncio.set_event_loop(self.__loop)
        self.__loop.run_forever()
        logger.info("Exiting background loop")


    async def __connect(self, host: str, port: int) -> None:
        """
        Opens the connection and if successful it starts the recv loop.

        Parameters
        ----------
        host
            The host of the server to connect to.
        port
            The TCP port of the server to connect to.
        """
        logger.info("Opening connection")
        self.reader, self.writer = await asyncio.open_connection(host, port)
        logger.info("Connection established")
        self.background_task = asyncio.create_task(self.__background_tasks())


    async def __recv_loop(self):
        """
        This is the client recv loop
        """
        logger.info("Starting receive loop")
        while True:
            logger.info("Waiting for data")
            try:
                recvbytes = await self.reader.read(100)
                raise RuntimeError("Simulated error")
                if recvbytes == b"":
                    break
            except asyncio.CancelledError:
                logger.info("Recv loop cancelled")
                break
            await self.__msg_queue.put(recvbytes)
        logger.info("Finishing recv loop")


    async def __message_loop(self):
        """
        This loop processes messages from the recv loop
        """
        while True:
            try:
                recvbytes: bytes = await self.__msg_queue.get()
            except asyncio.CancelledError:
                logger.info("Cancelled message loop")
                break
            message = recvbytes.decode()
            self.__msgs.append(message)
            logger.info("Received data '%s'", message)


    async def __background_tasks(self):
        """
        What we want todo with our background tasks is to handle exceptions raised by them. When an exception is
        thrown by one of the tasks, the task ground will stop the other tasks in the group before the exception
        is passed up to the caller.
        """
        async with asyncio.TaskGroup() as tg:
            tg.create_task(self.__message_loop())
            tg.create_task(self.__recv_loop())
        logger.info("Terminating background tasks")
        

    async def __disconnect(self) -> None:
        """
        This coroutine closes the connection to the server. It cancels the background tasks and awaits them to finish.
        If the background tasks have raised an exception during execution, this function passes the exception onto 
        the caller of this function.
        """
        logger.info("Disconnecting from the server")
        self.background_task.cancel()
        try:
            await self.background_task
        except asyncio.CancelledError:
            logger.info("Background task cancelled")
        self.writer.close()
        await self.writer.wait_closed()
        logger.info("We have disconnected from the server")


    ###########################################################################
    # PUBLIC FUNCTIONS
    ###########################################################################


    async def start_client(self, retries: int = 0, timeout_s: float | None = None) -> None:
        """
        Attempts to create a connection with a remote host. It will retry a number of times before raising an
        exception.

        Parameters
        ----------
        retries
            The number of times to try and connect with the server.
        timeout_s
            The number of seconds to timeout the attempt to connect to the server.
        """
        retry_count = 0
        while True:
            try:
                async with asyncio.timeout(timeout_s):
                    await self.__connect(host="127.0.0.1", port=43215)
                return
            except (ConnectionRefusedError, TimeoutError) as e:
                logger.error("Start client error")
                retry_count += 1
                if retry_count > retries:
                    raise e
                await asyncio.sleep(1.0)
        

    async def stop_client(self) -> None:
        """
        Stops the client by ending background tasks and disconnecting from the server. If the background tasks
        raised an exception during execution, this raises the same exception.
        """
        await self.__disconnect()


    async def send(self, msg: str):
        """
        Send a message to the server.

        Parameters
        ----------
        msg
            The message to send to the server.
        """
        logger.info("Sending message: '%s'", msg)
        self.writer.write(msg.encode())
        await self.writer.drain()
        logger.info("Message sent")


    async def get_messages(self):
        """
        Or we could have a callback/task that the application can supply to handle these. The internal list storing 
        messages is cleared after this is sent.

        Returns
        -------
        The list of received messages to date.
        """
        local_msgs = self.__msgs
        self.__msgs = []
        return local_msgs
    

    def is_background_task_running(self) -> bool:
        fut = self.__loop.call_soon_threadsafe(self.background_task.done)


    def await_from_non_asyncio(self, coro):
        """
        Sends a coroutine for execution in the background thread event loop then awaits the result.

        Return
        ------
        future_result
            The return value of the coroutine.
        """
        fut = asyncio.run_coroutine_threadsafe(coro, self.__loop)
        return fut.result()


###############################################################################
# Main Function
###############################################################################        


async def app_async_main():
    """
    There are a few options to deal with the background tasks:
    * We could poll it to check if it is still running
    * We could launch two tasks, one that executes are nominal logic and one that awaits the background task
        and our main application would await both.
    * Not worry about it and only check it when we disconnect.
    """
    client = Client(False)
    await client.start_client(retries=1)
    logger.info("Client started. Doing work for 2 seconds")
    await asyncio.sleep(2.0)
    await client.send("Hello from async")
    await asyncio.sleep(5.0)
    logger.info("Slept for 5 seconds, closing loop")
    await client.stop_client()


def app_main():
    """
    """
    client = Client(True)
    client.await_from_non_asyncio(client.start_client(retries=1))
    logger.info("Client started. Doing work for 2 seconds")
    time.sleep(2.0)
    client.await_from_non_asyncio(client.send("Hello from non-async"))
    time.sleep(5.0)
    logger.info("Slept for 5 seconds, closing loop")
    client.await_from_non_asyncio(client.stop_client())
    msgs = client.await_from_non_asyncio(client.get_messages())
    logger.info("The number of messages received is %d", len(msgs))


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    use_backgroud_thread = True
    if use_backgroud_thread:
        app_main()
    else:
        asyncio.run(app_async_main())
    logger.info("Terminating application")
