CC=g++
CFLAGS=-O2 -Wall -std=c++11
#CFLAGS=-O0 -g -Wall -std=c++11 # debugging
INCLUDES=
LIBS=-ldivsufsort

SRCS=bit_stream.cc \
block.cc \
block_compressor.cc \
block_decompressor.cc \
block_reader.cc \
block_writer.cc \
compress.cc \
decompress.cc \
gap_array.cc \
huffman.cc \
jump_sequence.cc \
rle.cc
OBJS=$(SRCS:.cc=.o)

TARGET=zjump

.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJS) $(TARGET).o
	$(CC) $(CFLAGS) $(OBJS) $(TARGET).o -o $(TARGET) $(LIBS)

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)

