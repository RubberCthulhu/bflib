Big fu***** library
===================

C library which includes implementation of common data structures, algorithms and other staff like red-black trees, binary heaps, lists, etc.

Build and test
--------------

Build the library only:
```
cmake .
make
```
or
```
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```

Build with tests:
```
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```

Run tests:
```
make test [ARGS="-V"]
```
