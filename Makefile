.PHONY: all up

PIOARGS=-v

SRC=$(wildcard src/*.*)

all: ${SRC} platformio.ini
	platformio run ${PIOARGS} 
                              
up: ${SRC}                    
	platformio run ${PIOARGS}  --target=upload
                              
mon:                          
	platformio run ${PIOARGS}  --target=monitor
