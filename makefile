CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O0 -g
INCLUDES = -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = jogo_007
SRC = jogo_007.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET) $(LDFLAGS) $(LIBS)

run: $(TARGET)
	GALLIUM_DRIVER=llvmpipe ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
