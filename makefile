CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

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
