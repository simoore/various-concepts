import aiohttp
import asyncio


async def scrape_page(session, url):
    print(f"Scraping {url}")
    async with session.get(url) as resp:
        return len(await resp.text())
    

async def main():
    urls = [
        "https://www.superdataminer.com/posts/66cff907ce8e",
        "https://www.superdataminer.com/posts/f21878c9897",
        "https://www.superdataminer.com/posts/b24dec228c43"
    ]

    coro_objs = []

    # This demonstrates the use of the async with syntax
    async with aiohttp.ClientSession() as session:
        for url in urls:
            coro_objs.append(scrape_page(session, url))
        results = await asyncio.gather(*coro_objs)

    for url, length in zip(urls, results):
        print(f"{url} -> {length}")

asyncio.run(main())
