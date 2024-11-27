CC = gcc
CFLAGS = -lpthread
POSTGRES_FLAGS = $(shell -I/usr/include/postgresql)
LPQ_FLAGS = $(shell -L/usr/lib -lpq)
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
BUILD_DIR = build

all: server client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

server: $(BUILD_DIR)/server

client: $(BUILD_DIR)/client

$(BUILD_DIR)/server: src/server.c src/server_functions.c src/utils.h src/server_functions.h # src/gui/*.h
	$(CC) src/server.c src/server_functions.c -o $(BUILD_DIR)/server $(POSTGRES_FLAGS) $(LPQ_FLAGS) $(CFLAGS)

$(BUILD_DIR)/client: src/client.c src/client_functions.c src/utils.h
	gcc src/server.c src/server_functions.c -o build/server -I/usr/include/postgresql -L/usr/lib -lpq -lpthread

clean:
	rm -rf $(BUILD_DIR)/*

# client src/gui/login.c