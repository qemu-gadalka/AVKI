CC = gcc
ARG = -O2 -Wall -Wextra

build: build/AVKI

build/AVKI: src/main.c
	mkdir -p build
	$(CC) $(ARG) src/main.c -o build/AVKI

clean:
	rm -rf build

.PHONY: build clean
