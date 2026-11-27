CC      = gcc
CFLAGS  = -Wall -Wextra -g -D_GNU_SOURCE
OBJS    = main.o lexer.o parser.o config.o
TARGET  = unchair

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

main.o:   main.c types.h lexer.h parser.h config.h
lexer.o:  lexer.c lexer.h types.h
parser.o: parser.c parser.h config.h types.h
config.o: config.c config.h types.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean