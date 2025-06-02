CC = gcc
CFLAGS = -std=c99 -pthread	# -Wall -Wextra - check all warnings when compile
SERVER_TARGET = server
CLIENT_TARGET = client
SERVER_SRC = maze-game/server.c
CLIENT_SRC = maze-game/client.c
CLIENT_LIBS = -lX11

.PHONY: all clean server client

all: server client

server: $(SERVER_SRC) maze-game/game_commons.h
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC)

client: $(CLIENT_SRC) maze-game/game_commons.h
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(CLIENT_LIBS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)


install-deps:
	@echo "Для работы нужны pthread и xlib11"
	@