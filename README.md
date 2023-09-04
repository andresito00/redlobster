# A Limit Order Book

This implementation of a limit order book is designed to ingest a feed of actions as demonstrated by the strings present in `actions.txt`.

The implementation is designed to support 3 actions:
1. Place and fill orders or book the ones that are not filled immediately
2. Cancel orders that are on the book using their unique Order IDs (OID).
3. Print the book in sorted order:
    - Asks from highest to lowest price
    - Offers from highest to lowest price
    - Within price levels, I print oldest to youngest.

The implementation must flag invalid input:
- Print errors starting with "E [OID]"
- Duplicate OIDs are an error, though I am interpreting this to mean that we only care about dups when they are on the book, after an order has been filled its OID should be available for subsequent orders.

## Build
Requires CMake > 3.18.2 and a compiler supporting C++20.

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

## Run

By default the simple_cross binary is compiled to recive input over stdin, so unless you change
STDIN CMakeLists.txt line you'll have to invoke like so:
```bash
$ ./build/simple_crosss < actions.txt
# OR
$ python3 ./test/gen_actions.py SYMBOLS ACTIONS_PER_SYMBOL | ./build/simple_cross
```

### How To

The GitHub repository runs the test battery automatically.

```bash
$ cd build/
& ctest -V --output-on-failure
# OR
$ ./test.sh # from the root repo directory
```

## A Note on Style

I used [Google's style guide on C++](https://google.github.io/styleguide/cppguide.html) with a few tweaks implemented to my taste that can be seen in the .clang-format file. I tried to be consistent.

CMake applies other such Google clang-tidy rules if it is installed and the the lines are uncommented in the CMakeLists.txt.

Side note: It appears Google prefers raw pointers to non-const references. I actually prefer this because it is more obvious to the reader when someone is modifying someone else's memory. I didn't see those warnings until I built on my Mac.

### Hooks and automated formatting
To run pre-commit formatting specified by `.pre-commit-config.yaml`, `.clang-format`, and `black`.
```
$ apt install -y clang-format
$ python3 -m pip install pre-commit
# And from the repo root
$ pre-commit install
```
