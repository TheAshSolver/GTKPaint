APP_NAME = app
SRC_DIR  = src
CC       = gcc
CFLAGS = -I./include
PKG_CONFIG = pkg-config
GTK_FLAGS  = $(shell $(PKG_CONFIG) --cflags --libs gtk4)

SRC = $(wildcard $(SRC_DIR)/*.c)

all: $(APP_NAME)

$(APP_NAME): $(SRC)
	$(CC) $(SRC) -o $(APP_NAME) $(GTK_FLAGS) $(CFLAGS)

run: $(APP_NAME)
	./$(APP_NAME)

clean:
	rm -f $(APP_NAME)

.PHONY: all run clean
