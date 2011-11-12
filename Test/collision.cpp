#include <iostream>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace std;

int main(void){
	cout<<"Bullet program\n";
	// Wrapper that figures out which paris of objects are succeptible to collision
	btBroadphaseInterface *broadphase = new btDbvtBroadphase();   
	// Object defining all the collision config
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	// Dispatcher registers the callback collision function  
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);  
	// The Object which defines how the physics equations are going to be solved
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;  
	
	// Create the discrete simulation world
	btDiscreteDynamicsWorld *dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase, solver,collisionConfiguration);
	
	// Setting gravity in this new world	
	dynamicsWorld->setGravity(btVector3(0,-10,0));
	
	// Do everything else
	
	// Defining the Shape of the ground to be reused later in defining the actual ground
	btCollisionShape *groundShape = new btBoxShape(btVector3(10,10,10));
	// Defining the object that is going to move
	btCollisionShape *fallShape = new btBoxShape(btVector3(2,2,2));
	// Defines the motion parameters for the ground
	btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
	// Creating the final rigid body
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	// Adding the rigid body to the world
	dynamicsWorld->addRigidBody(groundRigidBody);
	
	// Defining motion parameters for the falling body
	btDefaultMotionState* boxMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,50,0)));
	// Creating the final rigid body for the box
	btScalar mass = 1;
	btVector3 fallIntertia(0,0,0);
	fallShape->calculateLocalInertia(mass, fallIntertia);
	btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(mass, boxMotionState, fallShape, fallIntertia);
	boxRigidBodyCI.m_restitution = 0.6;
	btRigidBody* boxRigidBody = new btRigidBody(boxRigidBodyCI);
	// Adding the rigid body to the world
	dynamicsWorld->addRigidBody(boxRigidBody);
	
	for(int i=0; i<300; i++){
	  dynamicsWorld->stepSimulation(1.0/60.0, 10);
	  btTransform trans;
	  boxRigidBody->getMotionState()->getWorldTransform(trans);
	  cout <<"sphere height: "<< trans.getOrigin().getY() << endl;
	  
	}
	
	// Clean up
	dynamicsWorld->removeRigidBody(boxRigidBody);
	delete boxRigidBody->getMotionState();
	delete boxRigidBody;
	
	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;
	
	delete fallShape;
	delete groundShape;
	
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
	
	return 0;
}


