CC = g++

MAKE_EXE = -o exe -time

GL_FLAGS =  -Wall -O3 -std=c++11 -lGLEW -lGL $(shell pkg-config sdl2 --cflags --libs)

LODEPNG_FLAGS = resources/LodePNG/lodepng.cpp -ansi -O3 -std=c++11

#UNNECCESARY_DEBUG = -Wall -Wextra -pedantic

all: build

build: main.cc
	$(CC) main.cc $(GL_FLAGS) $(LODEPNG_FLAGS) $(MAKE_EXE)
