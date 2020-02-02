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
### How to build all the tracker
Download this repo. In its root directory you will see a Makefile. Positioned inside this root directory (project root folder), open a terminal and then type the following:
$ **make release all**

Note that this will require a `make` environment and a C++ compiler (package `build-essential` covers both in Debian-type Linux distros).
This creates two binaries, the `application` (./bin/tracker) and the `unit test` (./test/bin/test). After completing these two builds, the tests will be run: the terminal should show the progression and timing for the tests execution.

### How to build just the tracker application (without running tests)
$ **make release app**
To START running the tracker application (with web server incorporated):
$ bin/**tracker** IPfilter port threadQty(max 200)      example:
bin/tracker 0.0.0.0 8080 200


### How to build just the tests and run it
$ **make release test** to build the automated test and run them in one shot.

Or
$ **make release webclient-cmd** to build the web command line client.
To run the client web request commands manually:
bin/**http-client-command** <G|Ppayload> <IpAddress> <port> /command

Or
$ **make release webclient-test** to build the web client batch tests.
To run the client batch test in one shot:
bin/**client_benchmark**

### How to run the tests
As commented above, immediately after a successful build, a test suite is run as a final part of the mentioned build. If you want to run the test again, then type:
$ ./test/bin/test

### Web Tests since version 2.0.0
#### Test command by command with a browser
Use a web plugin (such as Talend API Tester) to perform the following **POST** commands:
http://localhost:8080/isHotKey      Payload (plain text): 'text of the key' (the answer is in the reply payload: YES or NO)
http://localhost:8080/keySent       Payload (plain text): 'text of the key' (reports the key to the tracker)
http://localhost:8080/setTopHotKeys Payload 12 (for setting 12 key the max report size)

Use a web browser to perform the following **GET** commands:
http://localhost:8080/getTopHotKeys returning a JSON with n top keys and its frequencies (12 keys in our example)
http://localhost:8080/totalKeys returning the total number of registered keys.
http://localhost:8080/restore will restore the whole collection of keys from a previous backup in disk. This command should be performed at application startup.
http://localhost:8080/shutdown To perform a clean shutdown of the application.

#### Batch Test using command line

POST keySent :
```
$ bin/http-client-command P"key text" 127.0.0.1 8080 /keySent
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 141

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>keySent:</h1>
<p>reported key sent : "key text"</p>
</body>
</html>

$
```

POST isHotKey :
```
$ bin/http-client-command P"key text" localhost 8080 /isHotKey
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 116

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>isHotKey:</h1>
<p>NO</p>
</body>
</html>

$
```

POST setTopHotKeys or setKeyReportBaseSize (set base size to 12) :
$ bin/http-client-command P12 localhost 8080 /setTopHotKeys
or
```
bin/http-client-command P12 localhost 8080 /setKeyReportBaseSize
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 155

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>setKeyReportBaseSize:</h1>
<p>Top key report size is now 12</p>
</body>
</html>

$
```

GET getTopHotKeys (get a JSON with n top keys and its frequencies (12 keys in our example)) :
```
$ bin/http-client-command G localhost 8080 /getTopHotKeys
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 632

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>getTopHotKeys:</h1>
<p>{[{"Key": "testing_keySent_001_0016","Frequency": 1},{"Key": "testing_keySent_001_0015","Frequency": 1},{"Key": "testing_keySent_001_0014","Frequency": 1},{"Key": "testing_keySent_001_0013","Frequency": 1},{"Key": "testing_keySent_001_0012","Frequency": 1},{"Key": "testing_keySent_001_0011","Frequency": 1},{"Key": "testing_keySent_001_0010","Frequency": 1},{"Key": "testing_keySent_001_0009","Frequency": 1},{"Key": "testing_keySent_001_0008","Frequency": 1},{"Key": "testing_keySent_001_0007","Frequency": 1}]}</p>
</body>
</html>

$
```
Or, this:
```
$ bin/http-client-command G localhost 8080 /getTopHotKeys
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 608

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>getTopHotKeys:</h1>
<p>{[{"Key": "fifty-two fifty-two","Frequency": 57180},{"Key": "fifty fifty-two","Frequency": 57064},{"Key": "fifty fifty-one","Frequency": 57036},{"Key": "fifty fifty","Frequency": 56840},{"Key": "fifty-one fifty-one","Frequency": 56812},{"Key": "fifty-one fifty","Frequency": 56776},{"Key": "fifty-one fifty-two","Frequency": 56712},{"Key": "fifty-two fifty-one","Frequency": 56692},{"Key": "fifty-two fifty-three","Frequency": 56676},{"Key": "fifty-three fifty-three","Frequency": 56536}]}</p>
</body>
</html>

$
```

GET totalkeys (get the total number of different key sent and registered by the tracker) :
```
$ bin/http-client-command G localhost 8080 /totalKeys
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 122

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>totalKeys:</h1>
<p>1000005</p>
</body>
</html>

$
```

GET time (Tracker's server time UTC) :
```
$ bin/http-client-command G localhost 8080 /time
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 210

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>time:</h1>
<p>Current time is <b>2020-02-02 21:26:28</b> UTC. It is <b>1580678788</b> sec from epoch (01-01-1970).</p>
</body>
</html>

$
```

GET shutdown (cleanly shutdown of Tracker's server, returning to OS prompt) :
```
$ bin/http-client-command G localhost 8080 /shutdown
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 135

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>shutdown:</h1>
<p>Shutdown in progress.</p>
</body>
</html>

$
```
Simultaneously on the server side, you will see the shutdown process:
```
Shutdown in progress.
Server stopped.
Exit tracker application.

$ echo $?
0
$
```
As you can see the exit code is 0 (success).


GET restart (cleanly restart of Tracker's server, returning to the caller script with code 129 indicating restart) :
```
$ bin/http-client-command G localhost 8080 /restart
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 133

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>restart:</h1>
<p>Restart in progress.</p>
</body>
</html>

$
```
Simultaneously on the server side, you will see the shutdown process:
```
Restart in progress.
Server stopped.
Restarting tracker application.

$ echo $?
129
$
```
As you can see the exit code is 128+1 (signal received is SIGHUP = 1) indicating to the caller shell script that perform some cleaning (restart) and then the executable must be called again (bin/tracker 0.0.0.0 8080 200).

GET restore (optionally performed at startup, restoring the previously populated keys anf freqs info) :
```
$ bin/http-client-command G localhost 8080 /restore
HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 125

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>restore:</h1>
<p>Restored OK.</p>
</body>
</html>

$
```
Or, if there was some trouble with the files keys.csv and frequencies.csv, the answer would be :
```
$ bin/http-client-command G localhost 8080 /restore
HTTP/1.1 500 Internal Server Error
Server: Boost Beast
Content-Type: text/html
Content-Length: 128

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>restore:</h1>
<p>Restore FAILED.</p>
</body>
</html>

$
```

#### Batch test of client/server performance
	1) Start the server: 
```
$ export LD_LIBRARY_PATH=~/Proyecto/boost/lib
bin/tracker 0.0.0.0 8080 200
tracker pid is: 25865
Server started.
Accepting messages.
```
	2) Start the client batch test:  bin/client_benchmark 
```
$ bin/client_benchmark
Test for POST request to http://localhost:8080/keySent
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 146

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>keySent:</h1>
<p>reported key sent : XXX-XXXXXX-1A</p>
</body>
</html>

It took 482 uSec.

Test for POST request to http://localhost:8080/keySent
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 146

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>keySent:</h1>
<p>reported key sent : XXX-XXXXXX-2B</p>
</body>
</html>

It took 188 uSec.

Test for POST request to http://localhost:8080/keySent
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 146

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>keySent:</h1>
<p>reported key sent : XXX-XXXXXX-3C</p>
</body>
</html>

It took 158 uSec.

Test for POST request to http://localhost:8080/setKeyReportBaseSize
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 155

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>setKeyReportBaseSize:</h1>
<p>Top key report size is now 15</p>
</body>
</html>

It took 183 uSec.

Test for POST request to http://localhost:8080/isHotKey
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 116

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>isHotKey:</h1>
<p>NO</p>
</body>
</html>

It took 142 uSec.

Test for POST request to http://localhost:8080/keySent
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 145

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>keySent:</h1>
<p>reported key sent : YYY-YYYYYY-4</p>
</body>
</html>

It took 137 uSec.

Test for POST request to http://localhost:8080/isHotKey
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 116

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>isHotKey:</h1>
<p>NO</p>
</body>
</html>

It took 158 uSec.

Test for GET request to http://localhost:8080/getTopHotKeys
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 281

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>getTopHotKeys:</h1>
<p>{[{"Key": "YYY-YYYYYY-4","Frequency": 1},{"Key": "XXX-XXXXXX-3C","Frequency": 1},{"Key": "XXX-XXXXXX-2B","Frequency": 1},{"Key": "XXX-XXXXXX-1A","Frequency": 1}]}</p>
</body>
</html>

It took 183 uSec.

Test for GET request to http://localhost:8080/totalKeys
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 116

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>totalKeys:</h1>
<p>4</p>
</body>
</html>

It took 108 uSec.

Test for GET request to http://localhost:8080/getTopHotKeys
Response content: HTTP/1.1 200 OK
Server: Boost Beast
Content-Type: text/html
Content-Length: 281

<html>
<head>
<title>Reply from Hotspot Tracker</title>
</head>
<body>
<h1>getTopHotKeys:</h1>
<p>{[{"Key": "YYY-YYYYYY-4","Frequency": 1},{"Key": "XXX-XXXXXX-3C","Frequency": 1},{"Key": "XXX-XXXXXX-2B","Frequency": 1},{"Key": "XXX-XXXXXX-1A","Frequency": 1}]}</p>
</body>
</html>

It took 139 uSec.


Stress Test for massive multiple requests.

LOOP TEST TOOK 19243 mSec .

$ 
```

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
### Direct instertion to MapManager and direct insertion to the message queue (/keySent and /setTopHotKeys)
For the time being the direct insertion over the maps (MapManager) throughput is aprox. 40MKeys in 8.6s (4.65 MKeys/sec), while using the original threaded queue (version 1.0.0) this decreases to 40MKeys in 36s (1.11 MKeys/sec).
This poor performance is due to contention between the push() and pop() methods of this queue when performing mutex acquisition. The solution to this problem is to replace the queue implementation for another one inheriting from boost::lockfree::detail::queue . That is, a mutexless, lock-free implementation from the boost library. The aim of the former version 1 was to implement all standard code using just STL, but the philosophy was going to change in future versions, using libraries more suitable for this solution.
Now, starting from version 1.1.0 and afterwards, the queue is really implemented as a boost::lockfree::detail::queue , then the performance for direct access to the queue (/keySent or /setTopHotKeys) has been improved to 40MKeys in 15.5s (2.58 MKeys/sec).
The reference for all of these performance measurment was an i5-6260U CPU 1.8-2.6 GHz 16GiB DRAM computer, running Ubuntu 18.04.3 x86-64.
### Insertion via web client
Using bin/client_benchmark to insert keys from a web client perspective, the performance measured by the test was 1MKey in about 19.2 sec , that is 52 Kkeys/sec aproximately. As you can see, the web conexion slows down all the insertion process, so a direct TCP or even direct call is prefered. This web server might be eliminated in future versions.

