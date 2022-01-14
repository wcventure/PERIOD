CC ?= gcc
TARGET = FFT
OBJS = fft.o
CFLAGS = -O0 -w 
LDFLAGS = -lm -pthread
MACROS = ./null_macros/c.m4.null

x = *

$(TARGET): $(OBJS)
	$(CC) -g $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS) -Wno-return-type

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:
	m4 ${MACROS} $*.H > $*.h

.C.c:
	m4 $(MACROS) $*.C > $*.c

.c.o:
	$(CC) -g -c $(CFLAGS) $*.c -Wno-return-type

.C.o:
	m4 $(MACROS) $*.C > $*.c
	$(CC) -g -c $(CFLAGS) $*.c -Wno-return-type

