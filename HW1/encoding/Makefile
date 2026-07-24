
all: encoder

encoder: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f encoder core

.PHONY: all clean
