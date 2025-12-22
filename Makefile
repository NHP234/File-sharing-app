# Root Makefile for File Sharing Application

.PHONY: all server client clean clean-server clean-client run-server run-client help

# Default target: build both server and client
all: server client

# Build server
server:
	@echo "Building server..."
	@cd TCP_Server && $(MAKE)
	@echo "Server built successfully!"

# Build client
client:
	@echo "Building client..."
	@cd TCP_Client && $(MAKE)
	@echo "Client built successfully!"

# Clean all
clean: clean-server clean-client
	@echo "All clean!"

# Clean server
clean-server:
	@echo "Cleaning server..."
	@cd TCP_Server && $(MAKE) clean

# Clean client
clean-client:
	@echo "Cleaning client..."
	@cd TCP_Client && $(MAKE) clean

# Run server (default port 8080)
run-server:
	@echo "Starting server on port 8080..."
	@cd TCP_Server && ./server 8080

# Run client (default localhost:8080)
run-client:
	@echo "Connecting to localhost:8080..."
	@cd TCP_Client && ./client 127.0.0.1 8080

# Help
help:
	@echo "File Sharing Application - Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make              - Build both server and client"
	@echo "  make server       - Build server only"
	@echo "  make client       - Build client only"
	@echo "  make clean        - Clean all build files"
	@echo "  make clean-server - Clean server build files"
	@echo "  make clean-client - Clean client build files"
	@echo "  make run-server   - Run server on port 8080"
	@echo "  make run-client   - Run client connecting to localhost:8080"
	@echo "  make help         - Show this help message"
