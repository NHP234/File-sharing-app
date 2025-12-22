# Makefile for File Sharing Client

CC = gcc
CFLAGS = -Wall -g
TARGET = client
OBJS = client.o commands.o ui.o network.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

client.o: client.c common.h
	$(CC) $(CFLAGS) -c client.c

commands.o: commands.c common.h
	$(CC) $(CFLAGS) -c commands.c

ui.o: ui.c common.h
	$(CC) $(CFLAGS) -c ui.c

network.o: network.c common.h
	$(CC) $(CFLAGS) -c network.c

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

