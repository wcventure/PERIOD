# Makefile for the StringBuffer bug

CXX?=clang++
CXXFLAGS=-g #-fsanitize=address
LDFLAGS=-lpthread -ldl #-fsanitize=address -static-libasan

all: main

main: stringbuffer.o main.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp stringbuffer.o $(LDFLAGS)

stringbuffer.o: stringbuffer.cpp stringbuffer.hpp
	$(CXX) $(CXXFLAGS) -c -o stringbuffer.o stringbuffer.cpp $(LDFLAGS)

clean:
	rm -f stringbuffer.o main
