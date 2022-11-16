
CC	= gcc
BIN	= bin/
SRC	= src/
INCLUDE	= include/
EXEC	= k_means


CFLAGS = -O2 -fopenmp -ftree-vectorize -msse4 -mavx -funroll-loops

.DEFAULT_GOAL = k_means


k_means: k_means.c	
	$(CC) $(CFLAGS) k_means.c -o $(BIN)$(EXEC)

clean:
	rm -r bin/*

run:
	./$(BIN)$(EXEC)