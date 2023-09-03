# A Limit Order Book

## What and Why
This implementation of a limit order book is designed to ingest a feed of actions as demonstrated by the strings present in `actions.txt`.

The implementation is designed to support 3 actions:
1. Place and book orders that are not filled immediately
2. Cancel orders that are on the book using their unique Order IDs (OID).
3. Print the book in sorted order:
    - Asks from highest to lowest price
    - Offers from highest to lowest price
    - Within price levels, I print oldest to youngest.

### Workload Assumptions, Operations, and Complexity
- There are "relatively few" ticker symbols with extremely high amounts of activity.
- Their OrderBooks have long FIFOs of orders.
- Realistically, I expect price levels accesses to be extremely tightly grouped around the best offers/asks.

When building data structures around price level accesses this would normally make me reach for an array or two (perhaps one for whole currency and then an array of fractions for each of those), but with the time I've had to think about the problem, my reaction is that it is simply too much memory:
An 8-byte pointer * 2400 symbols in the NYSE * 9,999,999 (~192GB) to account for the possibility of the extremes. Plus, hash maps backed by linear probing algorithms instead of linked list searches exist to ameliorate some of the memory acceess concerns. (Though I did not implement or use one here, and I've never actually had the pleasure of benchmarking one myself.)


With the above in mind, what I have is:

- SimpleCross Contains one `order::BookMap`.
- A `order::BookMap` contains 2 data structures:
  - `std::unordered_map<std::string, order::OrderBook>`.
  - `std::unordered_map<order::oid_t, order::Order>`. (where `oid_t` is a `uint32_t`)
- A `order::OrderBook` contains 2 of the following data structures:
  - `LevelMap<order::price_t, order::Order, std::deque, std::less>`
  - These are used to maintain outstanding orders at each price level.
  - LevelMap is backed by a `std::map<std::price_t, std::deque<order::Order>>`

#### Place Order
- n = # asks or offers >>>> m = # symbols >? k = # price levels per symbol
```
Worst:
O(n) OID dup-check +
O(m) symbol -> OrderBook +
O(log(k))^2 price level lookup and erase * O(n) order fill

==> O(n * log(k)^2)
```

I think the the typical complexity should be something like:
```
Typical:
O(1) OID dup-check/log +
O(1) symbol -> OrderBook +
O(log(k) price level lookup) * O(1) order fill/book

==> O(log(k))
```

#### Cancel Order
```
Worst:
O(n) OID log +
O(m) symbol -> OrderBook +
O(log(k))^2 price level lookup and erase +
O(1) Order lookup by idx (to set .qty = 0)

==> O(n)
# I think, because we assume fewer price levels than orders...
```

```
Typical:

==> O(log(k))

# Find the symbol, find the price level, and 0-out the order idx
```

#### Print Orders
```
==> O(m * k * n)

# Access all of the symbols then sorted price levels, then all of the orders.
```

Because of my choices, I expect:
1. More consistent performance of sorted book output. (instead of sort -> print)
2. *Hopefully* kinder access patterns for the memory subsystem when dealing with the most commonly accessed FIFOs.
3. Efficient cancels when only given an OID.

`test_full_fills_asc_desc` appears to complete 32K order placements + 32K order fills in less about 0.5 seconds on my 2.3GHz i9.

### Pain Points
If I knew anything else about the distribution of a typical workload and I could assert that cancels are rare/don't need to be optimized, I would lean away from my choice to implement the qty counters at each tier of the LevelMap.

That code is error prone and not fun to maintain. Further wrapping the counter/size-tracking operations (as one might notice I have tried) will unfortunately result in more log(k) lookups of the price levels, which I'd like to avoid in `update_book` since we've already found the level we need.

`update_book` is somewhat unwieldy because of the responsibility I've given it to pick up `kill_order`'s trash.

## Build
Requires CMake > 3.18.2 and a compiler supporting C++17.

I used the ones listed below.

On my Mac I had to `#ifdef` out he use of an `arithmetic` Concept
used to restrict the possible types of my `LevelMap` template. My version of
AppleClang doesn't support all of C++20 apparently.
Why did I also support building on Mac? Performance profiling doesn't work great in an Ubuntu VM.
The built-in `Instruments` tooling provided by XCode is actually pretty handy for finding expensive call graphs,
but I'd like to find some micro-architecture introspection tools. There do appear to be some online.

### Linux
```bash
Ubuntu clang version 12.0.0-3ubuntu1~20.04.5
```
```bash
g++ (Ubuntu 10.5.0-1ubuntu1~20.04) 10.5.0
```

### Mac
```bash
Apple clang version 12.0.0 (clang-1200.0.32.29)
```

### How To
```bash
$ cmake -Bbuild/
$ make -Cbuild all
```

## Test

### How To
```bash
$ cd build/
& ctest -V --output-on-failure
# OR
$ ./test.sh # from the root repo directory
```

## A Note on Style

I used Google's style guide on C++ with a few tweaks implemented to my taste that can be seen in the .clang-format file. I tried to be consistent.

CMake applies other such Google clang-tidy rules.
