# oatpp-threaded-starter [![Build Status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.oatpp-starter?branchName=master)](https://dev.azure.com/lganzzzo/lganzzzo/_build/latest?definitionId=10&branchName=master)

Starter project of oat++ (AKA oatpp) application as threaded task in another application. Based on oatpp Multithreaded (Simple) API.
This example project packs 6 different ways to start (and stop) a Oat++ server in a thread. Every way is packed in its own `App_<example>` and binary.

See more:

- [Oat++ Website](https://oatpp.io/)
- [Oat++ Github Repository](https://github.com/oatpp/oatpp)
- [Get Started](https://oatpp.io/docs/start)

## Overview

### Project layout

```
|- CMakeLists.txt                        // projects CMakeLists.txt
|- src/
|    |
|    |- controller/                      // Folder containing MyController where all endpoints are declared
|    |- dto/                             // DTOs are declared here
|    |- AppComponent.hpp                 // Service config
|    |- App_NoStop.cpp                   // Oat++ in a thread without stopping method
|    |- App_StopSimple.cpp               // Oat++ in a thread with simplest stopping method, same as server.run(true);
|    |- App_StopByConditionCheck.cpp     // Oat++ in a thread stopped by checking a condition
|    |- App_StopWithFullEnclosure.cpp    // Complete Oat++ Environment encapsuled in a thread
|    |- App_StopByConditionWithFullEnclosure.cpp    // Complete Oat++ Environment encapsuled in a thread with condition API
|    |- App_RunAndStopInFunctions.cpp    // Like StopByConditionCheck but encapsuled in handy functions
|
|- test/                                 // test folder
|- utility/install-oatpp-modules.sh      // utility script to install required oatpp-modules.  
```

### Example "NoStop"
This example lets Oat++ run in its own thread and keeps most of its data in the scope of the thread (thread storage).
However, this example has no way of gracefully stopping the server and is only intended for applications that
"should not stop" or for testing purposes.

### Example "StopSimple"
To keep the mechanism of stopping the server gracefully as easy as possible, this example moves all data of Oat++
back in the scope of the main-thread (global storage) and only the server is run in its own thread. The server is then gracefully
stoppable from the main-thread.
**This example has the same characteristics as using the deprecated** `server.run(true);` **API**

### Example "StopByConditionCheck"
In this example the new condition API is used to stop the server. Instead of calling `server.stop()`, the server checks
in each internal iteration if it should continue to run.
This API is compatible with both thread and global storage concepts.

### Example "StopWithFullEnclosure"
We were asked once if it is safe to have the whole environment in the scope of the thread, not just the server
and its components. This example shows a safe way keeping **everything** Oat++ related in the scope of a thread while
still being able to stop the server gracefully. 

### Example "StopByConditionWithFullEnclosure"
Because the "StopWithFullEnclosure" requires some synchronisation to keep everything nice and safe, using a condition
instead of a call to `stop()` can greatly reduce the footprint for the full enclosure.

### Example "RunAndStopInFunctions"
Example "StopByConditionCheck" extended by encapsulating starting and stopping of the server in small and handy functions.

---

### Build and Run

#### Using CMake

**Requires** 

- `oatpp` module installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./<ExampleName>-exe  # - run application.

```

#### In Docker

```
$ docker build -t oatpp-starter .
$ docker run -p 8000:8000 -t oatpp-starter
```
