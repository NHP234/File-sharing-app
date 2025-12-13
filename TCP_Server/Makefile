# Makefile for File Sharing Server

CC = gcc
CFLAGS = -Wall -pthread -g
TARGET = server
OBJS = server.o auth.o group.o file_ops.o folder_ops.o utils.o network.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c

auth.o: auth.c common.h
	$(CC) $(CFLAGS) -c auth.c

group.o: group.c common.h
	$(CC) $(CFLAGS) -c group.c

file_ops.o: file_ops.c common.h
	$(CC) $(CFLAGS) -c file_ops.c

folder_ops.o: folder_ops.c common.h
	$(CC) $(CFLAGS) -c folder_ops.c

utils.o: utils.c common.h
	$(CC) $(CFLAGS) -c utils.c

network.o: network.c common.h
	$(CC) $(CFLAGS) -c network.c

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

