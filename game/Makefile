CC = g++ -Wall -ansi
FLAGS = -lGLU -lGLEW -lglut
LINKS = textfile.cpp
CONFIG = `sdl-config --cflags --libs` `pkg-config --cflags --libs libxml++-2.6`
INCLUDES = ../includes/bullet ../includes/bullet/BulletDynamics/libBulletDynamics.a ../includes/bullet/BulletCollision/libBulletCollision.a ../includes/bullet/LinearMath/libLinearMath.a

all:
	$(CC) renderer.cpp -o output $(FLAGS) $(LINKS) $(CONFIG) -I $(INCLUDES)
	
run:
	./output

	
