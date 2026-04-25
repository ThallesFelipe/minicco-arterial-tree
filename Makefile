CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -pedantic
TARGET = programa
SRC = minicco.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET) segments.csv
