.PHONY: all up

SRC=$(wildcard src/*.*)

all: ${SRC}
	platformio run

up: ${SRC}
	platformio run --target=upload
