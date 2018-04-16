.PHONY: all up

SRC=$(wildcard src/*.*)

all: ${SRC} platformio.ini
	platformio run

up: ${SRC}
	platformio run --target=upload

mon:
	platformio run --target=monitor
