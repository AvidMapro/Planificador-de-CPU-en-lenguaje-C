CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11
BIN=cpu_sim

SRC=src/main.c src/sim.c src/pq.c
all:
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN) *.o src/*.o gantt.txt events.csv
