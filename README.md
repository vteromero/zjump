## Zjump [![Build Status](https://travis-ci.org/vteromero/zjump.svg?branch=master)](https://travis-ci.org/vteromero/zjump)

__Zjump__ is a lossless compression algorithm that compresses data by using
several well-known compression techniques and some new ones (the exact used
algorithms will be explained in depth in a proper documentation). For now,
it offers similar compression ratios to _gzip_ or _bzip2_ when compressing
text data. However, the compression speed is slightly worse and the decompression
speed is clearly improveable.

Note that Zjump is an __experimental__ data compressor and early versions
may not be compatible one another. That means that it is still __unsafe__ to
use it in production environments.

The initial development goal is to find the right stack of algorithms and,
therefore, the right file format. After that, the focus will be on improving
performance and security.

### Limitations

* It only works in GNU/Linux systems.

### Requirements

* The Gcc compiler.
* One of these two build tools: gmake or CMake.

Zjump also depends on these open source projects:

* [libdivsufsort](https://github.com/y-256/libdivsufsort)
* [Google Test](https://github.com/google/googletest)

### Building

#### CMake (recommended)

You can build `zjump` by using CMake in this way:

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=RELEASE ..
    $ make

This will create the binary program within the build directory.

#### Makefile

To build simply do:

    $ make

### Running

Once you have built the program, you will be able to compress and decompress
files through command line. These are some examples:

    $ ./zjump file
    $ ./zjump -d file.zjump

To get more information on how to use it, just type `./zjump --help`

### To Do

* Create documentation about the compression algorithms used.
* Do some benchmarking comparing with the state-of-art in lossless compression.
Publish the results.

