CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
LDFLAGS = -lreadline

TARGET = myshell
SRC = myshell.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean