CC	= gcc
BIN	= bin/
SRC	= src/
INCLUDE	= include/
EXEC	= k_means
THREADS = 1


CFLAGS = -O2 -fopenmp -ftree-vectorize -msse4 -mavx -funroll-loops

.DEFAULT_GOAL = k_means


k_means: $(SRC)k_means.c	
	$(CC) $(CFLAGS) $(SRC)k_means.c -o $(BIN)$(EXEC)

clean:
	rm -r bin/*
runseq:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS) 1
runpar:
	./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS) $(THREADS)