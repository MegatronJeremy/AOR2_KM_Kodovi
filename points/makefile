CC=gcc
INCDIRS=-I.
OPT=-O0
CFLAGS=-Wall -Wextra -g $(INCDIRS) $(OPT)

CFILES=points.c
OBJECTS=points.o

BINARY=bin

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

# regular expression where % is a wildcard
# @ - left side of the colon
# ^ - right side of the colon
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	
clean:
	rm -rf $(BINARY) $(OBJECTS)

