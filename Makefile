# CS438 - spring 2018 MP0
#
# This is a simple example of a makefile, use this for reference when you create your own
#
# NOTE: if you decide to write your solution in C++, you will have to change the compiler 
# in this file. 

CC=g++
CC_OPTS=
CC_LIBS=
CC_DEFINES=
CC_INCLUDES=
CC_ARGS=${CC_OPTS} ${CC_LIBS} ${CC_DEFINES} ${CC_INCLUDES}

# clean is not a file
.PHONY=clean

#target "all" depends on all others
all: client server listener talker

# client C depends on source file client.c, if that changes, make client will 
# rebuild the binary
client: client.cpp
	@${CC} ${CC_ARGS} -std=c++98 -pedantic -g -o http_client client.cpp
	
clean:
	@rm -f talker server http_client client listener *.o
