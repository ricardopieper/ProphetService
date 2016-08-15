# ProphetService

This is the background service of Prophet. 

##Build
ProphetService can be compiled both on Linux and Windows. However, currently the Windows version does not use OpenBlas. 
For Windows, open ProphetService.sln on Visual Studio.

For Linux, you'll need:

 - OpenBlas
 - CMake 2.8

For Linux, you need to install Openblas first. Compile and install OpenBlas according to the [OpenBlas's documentation](https://github.com/xianyi/OpenBLAS).

After installing OpenBlas, run `cmake . & make`.

##Running
To run the application, run `./ProphetService $IP $PORT`, where `$IP` and `$PORT` are the IP address and port of Cassandra's installation.
