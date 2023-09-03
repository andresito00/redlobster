import sys
import asyncio
from typing import Dict, Iterable, List, Tuple
import numpy as np
import string
import random

SYMBOL_SIZE = 8


class AlwaysValidOID:
    def __init__(self):
        self.oid = -1

    def get(self):
        self.oid += 1
        return self.oid


avo = AlwaysValidOID()


def create_action(symbol: str, price: float) -> str:
    ask_or_offer = random.choice(["B", "S"])
    return f"O {avo.get()} {symbol} {ask_or_offer} 10 {price:.5f}"


async def main(num_symbols: int, num_actions: int):
    price_mu: int = 42  # target values centered around mean
    price_sigma: int = 8  # standard deviation
    # generate and append a random symbol string
    # of less than 8 characters
    prices: np.ndarray = np.random.normal(price_mu, price_sigma, num_symbols)
    assert prices[prices > 0.0].size == prices.size
    symbols: List[str] = [
        "".join(random.choice(string.ascii_uppercase) for _ in range(SYMBOL_SIZE))
        for _ in range(num_symbols)
    ]
    price_map: Dict[str, float] = dict(zip(symbols, prices))

    # for each symbol generate num_actions actions exc. print
    loop = asyncio.get_event_loop()
    w_transport, w_protocol = await loop.connect_write_pipe(
        asyncio.streams.FlowControlMixin, sys.stdout
    )
    writer = asyncio.StreamWriter(w_transport, w_protocol, None, loop)
    per_symbol_sigma = 0.25
    for i in range(num_actions * num_symbols):
        key = random.choice(list(price_map.keys()))
        # Use an async generator and async prints if this gets slow...
        s: np.ndarray = np.random.normal(price_map[key], per_symbol_sigma, 1)
        writer.write((create_action(key, s[0]) + "\n").encode("utf-8"))
        await writer.drain()
    writer.write("P\n")
    await writer.drain()


if __name__ == "__main__":
    num_symbols = int(sys.argv[1])
    num_actions = int(sys.argv[2])
    asyncio.run(main(num_symbols, num_actions))
