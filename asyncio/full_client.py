import asyncio
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
        """
        Creates the data structure to store received messages and creates the background loop if required.

        Parameters
        ----------
        use_background_thread
            True, if the client networking calls run in a background thread or in an event loop that will block 
            this thread when it starts running.
        """
        self.__msgs = []
        self.__response_prefix = None

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


    async def __connect(self, host: str, port: int, timeout_s: float | None = None) -> None:
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
        async with asyncio.timeout(timeout_s):
            self.reader, self.writer = await asyncio.open_connection(host, port)
        logger.info("Connection established")
        self.recv_loop_task = asyncio.create_task(self.__recv_loop_exception_handler())


    async def __recv_loop_exception_handler(self):
        while True:
            try:
                await self.__recv_loop()
            except asyncio.CancelledError:
                logger.info("Recv loop cancelled")
                break
            except Exception as e:
                logger.info("Unhandled recv loop exception", exc_info=e)


    async def __recv_loop(self):
        """
        This is the client recv loop
        """
        logger.info("Starting receive loop")
        while True:
            logger.info("Waiting for data")
            recvbytes = await self.reader.read(100)
            if recvbytes == b'':
                logger.info("Connection was closed by server")
                # How to notify main loop that this has happened
                break
            msg = recvbytes.decode()
            logger.info("Received data '%s'", msg)
            self.__msgs.append(msg)
            if self.__response_prefix is not None:
                if msg.startswith(self.__response_prefix):
                    # Add to response queue
                    pass
        logger.info("Finishing recv loop")


    async def __disconnect(self):
        """
        This coroutine closes the connection to the server.
        """
        logger.info("Closing the connection")
        if hasattr(self, "recv_loop_task"):
            self.recv_loop_task.cancel()
            await self.recv_loop_task
        if hasattr(self, "writer"):
            self.writer.close()
            await self.writer.wait_closed()
        logger.info("We closed the connection")


    async def __send(self, msg: str, response_prefix: str | None = None):
        logger.info("Sending message '%s'", msg)
        if hasattr(self, "writer"):
            self.writer.write(msg.encode())
            await self.writer.drain()
        if response_prefix is not None:
            # Wait for a response
            pass


    ###########################################################################
    # PUBLIC FUNCTIONS
    ###########################################################################
        

    def start_client(self, retries: int = 0, connection_timeout: float | None = None) -> bool:
        """
        When it comes to threads, we will try to terminate them gracefully. If they don't terminate after a timeout
        we will terminate them ungracefully. If the main thread exits, we want all threads to terminate so the 
        process doesn't hang. So always use daemon threads, but don't rely on just exiting the main thread to 
        terminate them.

        Parameters
        ----------
        retries
            The number of attempts to retry the connection before this function fails.
        connection_timeout
            The number of seconds to timeout an attempt to connect. If None, there is no timeout.

        Returns
        -------
        connected
            True if the connection was successful, false if not.
        """
        def wait_until_connected(fut):
            try:
                fut.result()
            except ConnectionRefusedError:
                logger.error("Connection refused")
                return False
            except TimeoutError:
                logger.error("Connection timeout")
                return False
            return True

        retry_count = 0
        while retry_count <= retries:
            connect_coro = self.__connect(host="127.0.0.1", port=43215, timeout_s=connection_timeout)
            connect_future = asyncio.run_coroutine_threadsafe(connect_coro, self.__loop)
            if wait_until_connected(connect_future):
                return True
            retry_count += 1
        return False


    def stop_client(self) -> None:
        """
        Disconnects the client and stops the recv loop task.
        """
        disconnect_future = asyncio.run_coroutine_threadsafe(self.__disconnect(), self.__loop)
        disconnect_future.result()


    def send(self, msg: str) -> None:
        """
        Sends a message.

        Parameters
        ----------
        msg
            The message to send to the server.
        """
        send_future = asyncio.run_coroutine_threadsafe(self.__send(msg), self.__loop)
        send_future.result()


    def is_recv_loop_running(self):
        """
        Can be used
        """
        # I don't like this approach
        if hasattr(self, "recv_loop_task") == False:
            return False
            # Is this threadsafe
        return not self.recv_loop_task.done()


    ###########################################################################
    # PUBLIC ASYNC FUNCTIONS
    ###########################################################################


    async def send_task(self, msg: str, response_prefix: str | None = None):
        """
        Add this task to an event loop with the main task running and the message will be sent to the server.
        Once the reply is received this task will terminate and the return value has the response value.
        """
        pass


    async def main_task():
        """
        If your application uses asyncio, you can add this task to your event loop.
        """
        pass


###############################################################################
# MAIN FUNCTIONS
###############################################################################        


if __name__ == "__main__":

    logging.basicConfig(level=logging.INFO)

    client = Client()
    client.start_client(retries=2)
    
    time.sleep(2.0)
    logger.info("Slept for 2 seconds, sending message")
    client.send("Client Message")

    time.sleep(5.0)
    logger.info("Slept for 5 seconds, closing loop")
    client.stop_client()

    logger.info("Terminating application")

