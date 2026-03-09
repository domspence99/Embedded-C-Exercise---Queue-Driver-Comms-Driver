CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -Iinclude

SRC = src/main.c src/queue.c src/comm.c
OBJ = $(SRC:.c=.o)

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
