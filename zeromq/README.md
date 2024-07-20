## Example: Error Handling for Send Errors

```python
import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect("tcp://localhost:5555")

def send_message(socket, message, max_retries=5, backoff=1.0):
    retries = 0
    while retries < max_retries:
        try:
            socket.send(message, zmq.NOBLOCK)
            return True
        except zmq.ZMQError as e:
            if e.errno == zmq.EAGAIN:
                # Transient error, retry after a short delay
                print(f"Transient error (EAGAIN): {e}, retrying {retries + 1}/{max_retries}")
                time.sleep(backoff * retries)
                retries += 1
            elif e.errno in (zmq.ENOBUFS, zmq.ETIMEDOUT):
                # Transient errors, retry after a short delay
                print(f"Transient error ({e.errno}): {e}, retrying {retries + 1}/{max_retries}")
                time.sleep(backoff * retries)
                retries += 1
            else:
                # Non-transient error, log and exit
                print(f"Non-transient error ({e.errno}): {e}, not retrying")
                return False
    return False

def recreate_socket(context):
    new_socket = context.socket(zmq.PUSH)
    new_socket.connect("tcp://localhost:5555")
    return new_socket

message = b"Hello, World!"
if not send_message(socket, message):
    print("Message could not be sent after retries, recreating socket")
    socket.close()
    socket = recreate_socket(context)
    # Retry sending the message with the new socket if necessary
    if not send_message(socket, message):
        print("Message could not be sent with the new socket")

# Clean up
socket.close()
context.term()
```

## Example: Error Handling for Recv Errors

```python
import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.PULL)
socket.connect("tcp://localhost:5555")

poller = zmq.Poller()
poller.register(socket, zmq.POLLIN)

def recv_with_polling(socket, poller, max_retries=5, timeout=1000, backoff=1.0):
    retries = 0
    while retries < max_retries:
        socks = dict(poller.poll(timeout))
        if socks.get(socket) == zmq.POLLIN:
            try:
                message = socket.recv(zmq.NOBLOCK)
                return message
            except zmq.ZMQError as e:
                if e.errno == zmq.EAGAIN:
                    print(f"Transient error (EAGAIN): {e}, retrying {retries + 1}/{max_retries}")
                elif e.errno == zmq.ETIMEDOUT:
                    print(f"Transient error (ETIMEDOUT): {e}, retrying {retries + 1}/{max_retries}")
                else:
                    print(f"Non-transient error ({e.errno}): {e}, not retrying")
                    return None
        else:
            print(f"No message received, retrying {retries + 1}/{max_retries}")
        time.sleep(backoff * retries)
        retries += 1
    return None

message = recv_with_polling(socket, poller)
if message:
    print(f"Received message: {message}")
else:
    print("Failed to receive message after retries")

# Clean up
socket.close()
context.term()
```

# Example: REP Socket that Retries

```python
import zmq
import time
import threading

def rep_socket_thread(context, endpoint):
    def create_socket():
        new_socket = context.socket(zmq.REP)
        new_socket.bind(endpoint)
        return new_socket

    socket = create_socket()
    poller = zmq.Poller()
    poller.register(socket, zmq.POLLIN)

    while True:
        try:
            # Poll for incoming messages
            socks = dict(poller.poll(1000))  # 1 second timeout
            if socks.get(socket) == zmq.POLLIN:
                try:
                    message = socket.recv(zmq.NOBLOCK)
                    print(f"Received message: {message}")
                    
                    # Process the message and send a reply
                    reply = b"Reply"
                    socket.send(reply)
                    print(f"Sent reply: {reply}")
                except zmq.Again as e:
                    print(f"Transient error (EAGAIN) on recv: {e}")
                except zmq.ZMQError as e:
                    if e.errno == zmq.ETIMEDOUT:
                        print(f"Transient error (ETIMEDOUT) on recv: {e}")
                    else:
                        print(f"Non-transient error on recv ({e.errno}): {e}")
        except zmq.ZMQError as e:
            print(f"Error in poller: {e}")
            if e.errno in (zmq.ETERM, zmq.ENOTSOCK, zmq.EFSM):
                # Handle termination, invalid socket, or incorrect state
                print(f"Critical error ({e.errno}): {e}, recreating socket")
                socket.close()
                socket = create_socket()
                poller = zmq.Poller()
                poller.register(socket, zmq.POLLIN)
            else:
                print(f"Non-transient error ({e.errno}): {e}")
        except Exception as e:
            print(f"Unexpected error: {e}")
            # Optionally add a small sleep to avoid a busy loop in case of repeated errors
            time.sleep(1)

    # Clean up
    socket.close()

# Main context and thread setup
context = zmq.Context()
endpoint = "tcp://*:5555"

thread = threading.Thread(target=rep_socket_thread, args=(context, endpoint))
thread.start()

# The thread will run indefinitely, handling messages and errors
# To stop the thread and context cleanly, you should implement a proper shutdown mechanism
# Example:
# thread.join()
# context.term()
```

## Reference Projects

* https://github.com/tede12/RealMQ

1. "TCP/IP Sockets in C: Practical Guide for Programmers" by Michael J. Donahoo and Kenneth L. Calvert
2. "UNIX Network Programming, Volume 1: The Sockets Networking API" by W. Richard Stevens, Bill Fenner, and Andrew M. Rudoff
3. "Network Programming with Go" by Adam Woodbeck
4. "ZeroMQ: Messaging for Many Applications" by Pieter Hintjens
5. "Effective TCP/IP Programming: 44 Tips to Improve Your Network Programs" by Jon C. Snader
6. "Computer Networks: A Systems Approach" by Larry L. Peterson and Bruce S. Davie