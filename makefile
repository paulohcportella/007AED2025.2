CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g $(shell pkg-config --cflags raylib)
LIBS = $(shell pkg-config --libs raylib) -lm -lpthread -ldl -lrt

TARGET = jogo_007
SRC = jogo_007.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: $(TARGET)
	GALLIUM_DRIVER=llvmpipe DISPLAY=:0 ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
