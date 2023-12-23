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
        self.recv_loop_task = asyncio.create_task(self.__recv_loop())


    async def __recv_loop(self):
        """
        This is the client recv loop
        """
        logger.info("Starting receive loop")
        while True:
            logger.info("Waiting for data")
            try:
                recvbytes = await self.reader.read(100)
            except asyncio.CancelledError:
                logger.info("Recv loop cancelled")
                break
            logger.info("Received data %s", recvbytes)
        logger.info("Finishing recv loop")


    async def __disconnect(self):
        """
        This coroutine closes the connection to the server.
        """
        logger.info("Disconnecting from the server")
        self.recv_loop_task.cancel()
        await self.recv_loop_task
        self.writer.close()
        await self.writer.wait_closed()
        logger.info("We have disconnected from the server")


    ###########################################################################
    # PUBLIC FUNCTIONS
    ###########################################################################


    def wait_until_connected(self, timeout: float | None = None) -> None:
        """
        This waits until the connect_f
        """
        self.connect_future.result(timeout)
    

    def start_client(self) -> None:
        """
        When it comes to threads - we will try to terminate them gracefully. If they don't terminate after a timeout
        we will terminate them ungracefully. If the main thread exits - we want all threads to terminate so the 
        process doesn't hang. So always use daemon threads, but don't rely on just exiting the main thread to 
        terminate them.
        """
        connect_coro = self.__connect(host="127.0.0.1", port=43215)
        self.connect_future = asyncio.run_coroutine_threadsafe(connect_coro, self.__loop)
        self.wait_until_connected()


    def stop_client(self) -> None:
        """
        """
        self.disconnect_future = asyncio.run_coroutine_threadsafe(self.__disconnect(), self.__loop)
        self.disconnect_future.result()


###############################################################################
# Main Function
###############################################################################        


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    client = Client()
    client.start_client()
    
    time.sleep(5.0)
    logger.info("Slept for 5 seconds, closing loop")
    client.stop_client()
    logger.info("Terminating application")

