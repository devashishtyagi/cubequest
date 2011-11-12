CC = g++ -Wall -ansi
FLAGS = -lGLU -lGLEW -lglut
INCLUDES = ../includes/bullet ../includes/bullet/BulletDynamics/libBulletDynamics.a ../includes/bullet/BulletCollision/libBulletCollision.a ../includes/bullet/LinearMath/libLinearMath.a
FILE1 = rendercollision.cpp
FILE2 = collision.cpp
OUTPUT1 = render
OUTPUT2 = collision

all:
	$(CC) $(FILE1) -I $(INCLUDES) $(FLAGS) -o $(OUTPUT1)
render:
	./render
collision:
	./collision
