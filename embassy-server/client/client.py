import asyncio

async def tcp_echo_client(message: str):
    reader, writer = await asyncio.open_connection("192.168.10.3", 6789)

    print(f"Send: {message!r}")
    writer.write(message.encode("ascii"))
    await writer.drain()

    data = await reader.read(100)
    print(f"Received: {data.decode()!r}")

    print("Close the connection")
    writer.close()
    await writer.wait_closed()

asyncio.run(tcp_echo_client("Hello World!"))