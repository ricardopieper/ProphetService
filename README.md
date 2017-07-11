# ProphetService

This is the background service of Prophet. 

## Build

ProphetService can be compiled both on Linux and Windows. However, currently the Windows version does not use OpenBlas. 
For Windows, open ProphetService.sln on Visual Studio.

For Linux, you'll need:

 - OpenBlas
 - CMake 2.8
 - libuv

For Linux, you need to install OpenBlas first. Compile and install OpenBlas according to [OpenBlas's documentation](https://github.com/xianyi/OpenBLAS).

Try running `cmake . & make`. If it fails because `libuv` is missing, please install [libuv](https://github.com/libuv/libuv).

After installing OpenBlas and checking whether libuv is installed, run `cmake . & make`.

## Running

You'll need to install Cassandra first, and then run the ProphetWeb app first, to create the database tables.

To run the application, run `./ProphetService $IP $PORT`, where `$IP` and `$PORT` are the IP address and port of Cassandra's installation.
