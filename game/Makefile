CC = g++ -Wall -ansi
FLAGS = -lGLU -lGLEW -lglut
FILENAME = textfile.cpp
CONFIG = `sdl-config --cflags --libs` `pkg-config --cflags --libs libxml++-2.6`
all:
	$(CC) renderer.cpp -o output $(FLAGS) $(FILENAME) $(CONFIG)
	
run:
	./output
	
