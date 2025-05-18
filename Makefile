CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRCS = src/fulani.c src/lexer.c src/parser.c src/ast.c src/interpreter.c
OBJS = $(SRCS:.c=.o)
TARGET = fulani 

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 
