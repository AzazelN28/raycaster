CC=g++
CFLAGS=-Wall -g `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

main:
	$(CC) $(CFLAGS) $(LIBS) -o dist/main main.cpp
