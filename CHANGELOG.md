Version 0.2.0:
--------------
Major features:
* Better decompression speed in all kind of files.
* Better compression ratio in general.

Other changes:
* format: new block format (incompatible with previous versions).
* cli: the input file is deleted after compressing and decompressing.
* cli: added --keep/-k option to avoid deleting the input file.
* build: added continuous integration with Travis CI.
* test: added Rle1 unit tests.

Version 0.1.0:
--------------
* build: added CMake building system.
* build: third party dependencies are built with CMake (ExternalProject).
* test: added unit testing (Google Test).

