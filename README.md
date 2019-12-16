# MediaMath's Coding Challenge: KeyTracker
Mediamath Coding Challenge exercise about string key tracking.

##Overview
The purpose of this simple excercise is to provide a way to keep track of different key values (character strings) sent to a different key servers in a cloud partitioned topology. Therefore, using this tracker a better understanding of which are the most repeated keys (hot keys) can help in a design to a routing algorithm to the servers of those keys (hotspots) that dominate the traffic.

##Build and run
###How to build tracker
Download this repo. In its root directory you will see a Makefile. Positioned inside this directory (folder), open the console and then type the following:
$ make release all

This performs two builds, the application (./bin/tracker) and the unit test (./test/bin/test). After the two successful builds, the console should show the progression and the timing of the tests, and finally, the total of the test passed ok.

###How to run the tests further times
As commented above, immediately after a successful build a test suit is run as a final part of the mentioned build. If you want to run the test again, then type:
$ ./test/bin/test

##Further builds
###Rebuild all for debugging session
The above build generates by default executables not having debug information (release or production). When you perform a debug build typing make, a debug build is performed. You can confirm this reading the compilation of each .cpp file and noticing "g++ -g3 -O0 ...". When you need to do a debug build having debug info (release mode), you have to type:
$make

or

$make debug

###Partial builds
$ **make app**  Builds only newer files to produce the application : ./bin/tracker

$ **make test** Builds only newer files to produce the application : ./test/bin/test

$ **make all** builds all in debug (application and test). It is the same as just typing **make**.

$ **make release all** builds all in release (application and test). Note that make release does not trigger anything as "release" is not a file system target.

###Cleaning object files and executables
$ **make clean** Erases all the object and executables previosly built, enabling a further clean build from scratch.

$ **make cleanapp** Just cleans the object files related with the application, and its executable file tracker.

$ **make cleantest** Just cleans the object files related with the unit test, and its executable file test.

##Performance Issues

For the time being the direct insertion over the maps (MapManager) is aprox. 8.6s in 40MKeys (4.65 MKeys/sec) , while using the threaded queue is only 36s in 40MKeys (1.11 MKeys/sec).
This poor performance is due to contention between push() and pop() methods of this queue when performing mutex acquiring. The solution to this problem is to replace this queue for another inheritated from boost::lockfree::detail::queue . That is, a mutexless implementation from boost library. The aim of this current version 1 has been to implement all standard code using just STL, but this philosophy is going to be revised in further versions, using libraries more suitable for this solution.

