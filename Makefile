CC = gcc
CFLAGS = -lpthread
POSTGRES_FLAGS = -I/usr/include/postgresql
LPQ_FLAGS = -L/usr/lib -lpq
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)
BUILD_DIR = build2

all: server client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

server: $(BUILD_DIR)/server

client: $(BUILD_DIR)/client

$(BUILD_DIR)/server: server.c server_functions.c utils.h server_functions.h # src/gui/*.h
	$(CC) server.c server_functions.c -o $(BUILD_DIR)/server $(POSTGRES_FLAGS) $(LPQ_FLAGS) $(CFLAGS)

$(BUILD_DIR)/client: client.c utils.h
	$(CC) client.c -o $(BUILD_DIR)/client $(CFLAGS) 

clean:
	rm -rf $(BUILD_DIR)/*

# client src/gui/login.c