#!/usr/bin/make
TARGET=leds_user
SOURCES=leds_user.c

OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
MY_INCLUDES=.
HEADERS=$(wildcard $(MY_INCLUDES)/*.h)
OS=$(shell uname)
LDFLAGS=-lpthread
CFLAGS=-g -Wall 
#CFLAGS=-g -Wall -DPOSIX_BARRIER

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	gcc  -o  $(TARGET) $(OBJECTS) $(LDFLAGS) -lX11

.SUFFIXES:      .o .cpp .h

.h:

.c.o: $(HEADERS)
	gcc  $(CFLAGS)  -I$(MY_INCLUDES) -c $< -o $@  -Wall 
	
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f  *~
	



