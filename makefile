# compile rainbow-xd.cpp for linux

# Not compatible with Windows terminal. Use WSL instead.

# Dependencies:
# build-essential including g++ and make, strip (binutils)

# Compiler
CC = g++
CPPFLAGS = -Wall -std=c++17 -O3 -static

# Files
SRC = rainbow-xd.cpp

# Targets
all: rainbow-xd

rainbow-xd: $(SRC)
	$(CC) $(CPPFLAGS) -o rainbow-xd $(SRC)
	strip --strip-all rainbow-xd

clean:
	rm -f rainbow-xd
	

install:
	cp rainbow-xd /usr/bin/rainbow-xd