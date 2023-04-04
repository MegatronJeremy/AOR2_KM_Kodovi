CC=gcc
INCDIRS=-I.
OPT=-O3
CFLAGS=-Wall -Wextra -g $(INCDIRS) $(OPT)

CFILES=binary_search.c
OBJECTS=binary_search.o

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

