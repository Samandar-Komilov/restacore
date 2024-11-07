CC = gcc
CFLAGS = -pthread
BUILD_DIR = build

all: server client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

server: $(BUILD_DIR)/server

client: $(BUILD_DIR)/client


$(BUILD_DIR)/server: src/server/server.c src/server/server_auth.c src/shared/network.c src/shared/*.h
	$(CC) src/server/server.c src/server/server_auth.c src/shared/network.c -o $(BUILD_DIR)/server $(CFLAGS)

$(BUILD_DIR)/client: src/client/client.c src/shared/network.c src/shared/*.h
	$(CC) src/client/client.c src/shared/network.c -o $(BUILD_DIR)/client $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*
