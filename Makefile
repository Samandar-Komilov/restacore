CC = gcc
CFLAGS = -lpthread
POSTGRES_FLAGS = -I/usr/include/postgresql
LPQ_FLAGS = -lpq
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
BUILD_DIR = build2

all: server client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

server: $(BUILD_DIR)/server

client: $(BUILD_DIR)/client


# $(BUILD_DIR)/server: src/server/server.c src/server/server_auth.c src/shared/network.c src/shared/*.h # src/gui/*.h
# 	$(CC) src/server/server.c src/server/server_auth.c src/server/order.c src/shared/network.c -o $(BUILD_DIR)/server $(CFLAGS)

# $(BUILD_DIR)/client: src/client/client.c src/shared/network.c src/shared/*.h
# 	$(CC) src/client/client.c src/shared/network.c -o $(BUILD_DIR)/client $(CFLAGS) $(GTK_CFLAGS) $(GTK_LIBS)

$(BUILD_DIR)/server: server.c server_functions.c utils.h # src/gui/*.h
	$(CC) server.c server_functions.c -o $(BUILD_DIR)/server $(CFLAGS) $(POSTGRES_FLAGS) $(LPQ_FLAGS)

$(BUILD_DIR)/client: client.c utils.h
	$(CC) client.c -o $(BUILD_DIR)/client $(CFLAGS) 

clean:
	rm -rf $(BUILD_DIR)/*

# client src/gui/login.c