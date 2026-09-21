CC = gcc
ARG = -O3

build: build/AVKI

build/AVKI: src/main.c
	mkdir -p build
	$(CC) $(ARG) src/main.c -o build/AVKI

clean:
	rm -rf build

.PHONY: build clean
