# MediaMath's Coding Challenge: KeyTracker
Mediamath Coding Challenge exercise about string key tracking:
```
The Problem:

A load-balancer application redistributes inbound requests across a cluster of backend servers in a partition-by-key manner. However, the frequency of the partition keys varies greatly among different keys, with some outliers that can be 100x - 1000x more popular than the average keys. A naive partition-by-key routing algorithm will, unfortunately, create hotspots on some backend servers.

 We want to either avoid or significantly reduce the impact of the hotspots had this happened. A solution to address this problem is to detect and identify keys that will cause hotspots. The load-balancer could then use a different routing algorithm (like round-robin-partition) on requests with those keys.

 The Asks:

 Implement an efficient hotspot tracker to track frequency of keys. The tracker needs to dynamically keep track of at least the N most frequently used keys (hotspots). It must provide an interface to answer the following questions:

• Who are in the hotspots list? (top N keys by frequency)

• Is a given key a hotspot?

 In this problem, N is a configurable variable that is provided during initialization.

 You can implement your design in either C++ or Golang.
```

## Overview
The purpose of this simple excercise is to provide a way to keep track of different key values (character strings) sent to different key servers in a cloud partitioned topology. Therefore, using this tracker, a better understanding of which are the most repeated keys (hot keys) can help design a routing algorithm to the servers of those keys (hotspots) that dominate the traffic.

## Build and run
### How to build tracker
Download this repo. In its root directory you will see a Makefile. Positioned inside this root directory (project root folder), open a terminal and then type the following:
$ **make release all**

Note that this will require a `make` environment and a C++ compiler (package `build-essential` covers both in Debian-type Linux distros).
This creates two binaries, the `application` (./bin/tracker) and the `unit test` (./test/bin/test). After completing these two builds, the tests will be run: the terminal should show the progression and timing for the tests execution.

### How to run the tests
As commented above, immediately after a successful build, a test suite is run as a final part of the mentioned build. If you want to run the test again, then type:
$ ./test/bin/test

## Further builds
### Rebuild all for debugging session
The above build generates, by default, executables without debug information (intended for release/production). With the commands below, a debug build is performed. You can confirm this by checking the compiler output for each .cpp file and noticing "g++ -g3 -O0 ...". When you need to do a debug build having debug info, you have to type:
$make

or

$make debug

### Partial builds
$ **make app**  Builds only newer files to produce the application : ./bin/tracker

$ **make test** Builds only newer files to produce the application : ./test/bin/test

$ **make all** Builds everything in debug (application and test). It is the same as just typing **make**.

$ **make release all** Builds everything in release (application and test). Note that make release does not trigger anything as "release" is not a file system target.

### Cleaning object files and executables
$ **make clean** Erases all the object and executables previosly built, as a prior step to building from scratch.

$ **make cleanapp** Just cleans the object files related with the application, and its executable file tracker.

$ **make cleantest** Just cleans the object files related with the unit test, and its executable file test.

## Performance Issues

For the time being the direct insertion over the maps (MapManager) throughput is aprox. 40MKeys in 8.6s (4.65 MKeys/sec), while using the threaded queue this decreases to 40MKeys in 36s (1.11 MKeys/sec).
This poor performance is due to contention between the push() and pop() methods of this queue when performing mutex acquisition. The solution to this problem is to replace the queue implementation for another one inheriting from boost::lockfree::detail::queue. That is, a mutexless, lock-free implementation from the boost library. The aim of this current version 1 has been to implement all standard code using just STL, but this philosophy is going to change in future versions, using libraries more suitable for this solution.
