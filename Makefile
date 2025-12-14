# --- Biến (Variables) ---
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -pthread # <--- Thêm cờ linker (bao gồm -pthread) ở đây

# Đường dẫn đến mã nguồn
CLIENT_SRC = TCP_Client/client.c 
SERVER_SRC = TCP_Server/server.c TCP_Server/file_utils.c

# Tên file thực thi
CLIENT_TARGET = client
SERVER_TARGET = server

# --- Quy tắc (Rules) ---
.PHONY: all clean

# Build cả hai chương trình
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Biên dịch client
$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_TARGET) $(LDFLAGS) # <--- Thêm $(LDFLAGS)

# Biên dịch server
$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_TARGET) $(LDFLAGS) # <--- Thêm $(LDFLAGS)

# Dọn dẹp file biên dịch
clean:
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET) *.o
