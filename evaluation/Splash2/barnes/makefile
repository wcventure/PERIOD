CC ?= gcc
TARGET = BARNES
FILES = code.C code_io.C load.C grav.C getparam.C util.C \
	code.H defs.H stdinc.H vectmath.H
CS = code.c code_io.c load.c grav.c getparam.c util.c 
OBJS = code.o code_io.o load.o grav.o getparam.o util.o
CFLAGS = -O0 
LDFLAGS = -lm -pthread
MACROS = ./null_macros/c.m4.null
M4 = m4 -s -Ulen -Uindex

x = *

$(TARGET): $(OBJS)
	$(CC) -g $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS) -Wno-return-type

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:
	$(M4) ${MACROS} $*.H > $*.h

.C.c:
	$(M4) $(MACROS) $*.C > $*.c

.c.o:
	$(CC) -g -c $(CFLAGS) $*.c -Wno-return-type

.C.o:
	$(M4) $(MACROS) $*.C > $*.c
	$(CC) -g -c $(CFLAGS) $*.c -Wno-return-type

code.h: code.H defs.h
defs.h: defs.H stdinc.h vectmath.h
stdinc.h: stdinc.H 
vectmath.h : vectmath.H

code.o: code.C code.h defs.h
code_io.o: code_io.C code.h
getparam.o: getparam.C stdinc.h
grav.o: grav.C code.h
load.o: load.C code.h defs.h
util.o: util.C stdinc.h

