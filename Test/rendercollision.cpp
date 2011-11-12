#include <GL/glut.h>
#include <iostream>
#include <cstdio>
#include "btBulletDynamicsCommon.h"


static float t = 0.0;

static btScalar matrix[16];
static btTransform trans;

static btDiscreteDynamicsWorld *dynamicsWorld;
static btRigidBody *box1, *box2, *box3;


static void draw(void)
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float xpos = box2->getCenterOfMassPosition().getX();
    float ypos = box2->getCenterOfMassPosition().getY();
    float zpos = box2->getCenterOfMassPosition().getZ();

    printf("%f %f %f\n",xpos,ypos,zpos);


//*** draw box1 
glColor3f(0.0, 0.0, 1.0);
glPushMatrix();
box1->getMotionState()->getWorldTransform(trans);
trans.getOpenGLMatrix(matrix);
glMultMatrixf(matrix);
glutSolidCube(40);
glPopMatrix();

//*** draw box2
glColor3f(1.0, 1.0, 0.0);
glPushMatrix();
box2->getMotionState()->getWorldTransform(trans);
trans.getOpenGLMatrix(matrix);
glMultMatrixf(matrix);
glutSolidCube(10);
glPopMatrix();

// draw box 3
glColor3f(0.0,0.0,1.0);
glPushMatrix();
box3->getMotionState()->getWorldTransform(trans);
trans.getOpenGLMatrix(matrix);
glMultMatrixf(matrix);
glutSolidCube(30);
glPopMatrix();

glutSwapBuffers();
}


static void tim(void)
{
float dtime = t;
t = glutGet(GLUT_ELAPSED_TIME) / 500.0;
dtime = t - dtime;

if(dynamicsWorld)
dynamicsWorld->stepSimulation(dtime, 10);

glutPostRedisplay();
}


void keyPress(unsigned char key, int x, int y){
    if (key == 'w'){
      btVector3 velocity(0,0,0);
      velocity = box2->getLinearVelocity();
      box2->applyCentralImpulse(btVector3(0,0,-3));
    }
    if (key == 's'){
      btVector3 velocity(0,0,0);
      velocity = box2->getLinearVelocity();
      box2->applyCentralImpulse(btVector3(0,0,3));
    }
   if (key == 'a'){
      btVector3 velocity(0,0,0);
      velocity = box2->getLinearVelocity();
      box2->applyCentralImpulse(btVector3(-3,0,0));
    }
   if (key == 'd'){
      btVector3 velocity(0,0,0);
      velocity = box2->getLinearVelocity();
      box2->applyCentralImpulse(btVector3(3,0,0));
    }
    else if (key == 27){
      quick_exit(0);
    }
    tim();
}

int main(int argc, char** argv)
{
//*** init Bullet Physics
btQuaternion qtn;

btCollisionShape *shape;
btDefaultMotionState *motionState; 

btDefaultCollisionConfiguration *collisionCfg 
= new btDefaultCollisionConfiguration();

btAxisSweep3 *axisSweep 
= new btAxisSweep3(btVector3(-100,-100,-100), btVector3(100,100,100), 128);

dynamicsWorld = new btDiscreteDynamicsWorld(new btCollisionDispatcher(collisionCfg), 
axisSweep, new btSequentialImpulseConstraintSolver, collisionCfg);

dynamicsWorld->setGravity(btVector3(0, -10, 0));


//*** box1 - STATIC / mass=btScalar(0.0)
shape = new btBoxShape(btVector3(20,20,20));
trans.setIdentity();
qtn.setEuler(0, 0.0, 0.0);
trans.setRotation(qtn);
trans.setOrigin(btVector3(0, -20, 0));
motionState = new btDefaultMotionState(trans);
btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(btScalar(0.0), motionState, shape, btVector3(0,0,0));
boxRigidBodyCI.m_friction = 0.4;
box1 = new btRigidBody(boxRigidBodyCI);
dynamicsWorld->addRigidBody(box1);


//*** box3 - STATIC / mass=btScalar(0.0)
shape = new btBoxShape(btVector3(15,15,15));

trans.setIdentity();
qtn.setEuler(0, 0.0, 0.0);
trans.setRotation(qtn);
trans.setOrigin(btVector3(-35, -35, 0));
motionState = new btDefaultMotionState(trans);
btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI3(btScalar(0.0), motionState, shape, btVector3(0,0,0));
boxRigidBodyCI3.m_friction = 0.4;
box3 = new btRigidBody(boxRigidBodyCI3);
dynamicsWorld->addRigidBody(box3);


//*** box2 - DYNAMIC / mass=btScalar(1.0) 
shape = new btBoxShape(btVector3(5,5,5));

trans.setIdentity();
qtn.setEuler(0.8, 0.7, 0.4);
trans.setRotation(qtn);
trans.setOrigin(btVector3(-10, 50, 0));
motionState = new btDefaultMotionState(trans);
btScalar mass = 1;
btVector3 Intertia(0,0,0);
shape->calculateLocalInertia(mass, Intertia);
btRigidBody::btRigidBodyConstructionInfo *boxRigidBodyCI2;
boxRigidBodyCI2 = new btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, Intertia);
boxRigidBodyCI2->m_restitution = 0.4;
box2 = new btRigidBody(*boxRigidBodyCI2);
box2->setActivationState(DISABLE_DEACTIVATION);
//box2->setLinearFactor(btVector3(1,1,0));
//box2->setAngularFactor(btVector3(0,0,1));
dynamicsWorld->addRigidBody(box2);


//*** init GLUT 
glutInit(&argc, argv);
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
glutCreateWindow("Jump you little bastard");

glutDisplayFunc(draw);
glutIdleFunc(tim);
glutKeyboardFunc(keyPress);


//*** init OpenGL
glEnable(GL_CULL_FACE);
glEnable(GL_DEPTH_TEST);
glEnable(GL_LIGHT0);
glEnable(GL_LIGHTING);
glEnable(GL_COLOR_MATERIAL);

glMatrixMode(GL_PROJECTION);
gluPerspective( 90.0, 1.0, 1.0, 1000.0);
glMatrixMode(GL_MODELVIEW);
gluLookAt(0.0, 5.0, 60.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); 


glutMainLoop();


//*** EXIT
delete shape;
delete motionState;
delete collisionCfg;
delete axisSweep;
}
//---------------------------------------------------------------------------------------------------
