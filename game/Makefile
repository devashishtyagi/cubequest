CC = g++ -Wall -ansi
FLAGS = -lGLU -lGLEW -lglut
FILENAME = textfile.cpp
CONFIG = `sdl-config --cflags --libs` `pkg-config --cflags --libs libxml++-2.6`
all:
	$(CC) renderer.cpp -o output $(FLAGS) $(FILENAME) $(CONFIG)
	gcc -Wall -ansi lesson19.c -o particle -lGL -lGLU `sdl-config --cflags --libs`
	
run:
	./output

run1:
	./particle
	
