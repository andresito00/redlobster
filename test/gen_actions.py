import sys
from typing import Iterable, List, Tuple
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


def create_action(symbol: str, price: int) -> str:
    ask_or_offer = random.choice(["B", "S"])
    return f"O {avo.get()} {symbol} {ask_or_offer} 10 {price:.5f}"


if __name__ == "__main__":
    num_symbols = int(sys.argv[1])
    num_actions = int(sys.argv[2])
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
    price_map: Iterable[Tuple[str, float]] = zip(symbols, prices)
    with open("gen_actions.txt", "w") as f:
        # for each symbol generate num_actions actions exc. print
        actions = []
        per_symbol_sigma = 0.25
        for p in price_map:
            # for each symbol, their target prices will be centered around
            # a much tighter distribution to simulate tight price level grouping

            # Use an async generator and async prints if this gets slow...
            s: np.ndarray = np.random.normal(p[1], per_symbol_sigma, num_actions)
            actions.extend([create_action(p[0], i) for i in s])

        # shuffle them all
        random.shuffle(actions)
        # [f.writelines(a+'\n') for a in actions]
        # f.write("P\n")
        [print(a) for a in actions]
        print("P")
