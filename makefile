CC= gcc
CFLAGS=-Wall -std=c99 -D_GNU_SOURCE

TARGETS = pman csc360_list

.PHONY: all clean

OBJ = $(patsubst %,obj/%.o,$(TARGETS))
SRC = ./%.c

all: pman 

obj/%.o: $(SRC) | obj
	$(CC) -c -o $@ $< $(CFLAGS)

obj:
	mkdir -p $@

pman: $(OBJ)
	$(CC) -o pman $^ $(CFLAGS) -lreadline

debug: CFLAGS += -g
debug: pman

clean:
	-rm -rf obj/ pman
