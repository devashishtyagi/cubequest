#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>

#include "btBulletDynamicsCommon.h"
#include "fbo.h"
#include "shader.h"
#include "parser.cpp"

#define WAV_FILE "data/music_game.wav"

/* Max number of particles */
#define MAX_PARTICLES 150

using namespace std;

/* screen dimensions */
int SCREEN_WIDTH  = 600;
int SCREEN_HEIGHT = 600;
int SCREEN_BPP = 32;


Uint8 *audio_buf=NULL;
Uint32 wav_bytes=0;
/*creating the framebuffer object*/
fbo *scene;
fbo *scene2;
fbo *scene3;
fbo *scene4;

/* playfield sml */
char filename[] = "data/playfield.xml";
char vertex_shader[] = "shaders/bloom1.vs";
char fragment_shader[] = "shaders/bloom2.fs";
char fragment_shader_msaa[] = "shaders/hdr_msaa.fs";
char vertex_shader_msaa[] = "shaders/hdr_msaa.vs";


/* playfield data */
vector<wall> wallData;
vector<obstacle> obsData;
vector<holes> holesData;
vector<location> flameData, powerData;

/* cube drawing data */
wall cubeData(location(-0.5, 1.0, -2.0), location(0.5, 2.0, -1.0));

/* Simulation data */
static float simTime = 0.0;
static btScalar matrix[16];
static btTransform trans;
static btDiscreteDynamicsWorld *dynamicsWorld;
static vector<btRigidBody*> simWall;
static vector<btRigidBody*> simObs;
static btRigidBody *simCube;

/* Surface about which reflection is to occur */
int to_reflect=0;

/*the light position*/
GLfloat lightZeroPosition[] = {0.0, 4.5, 10.0, 1.0};
GLfloat lightZeroColor[] = {1.0, 0.0, 0.0, 1.0}; /* green-tinted */
GLfloat lightOnePosition[] = {0.0, 4.5, 10.0, 0.0};
GLfloat lightOneColor[] = {1.0, 0.0, 0.0, 1.0}; /* red-tinted */


/* variables specifying the camera attributes */
float eyec[] = {0.0f, 5.0f, 5.0f};
float eye[] = {2.0f, 5.0f, 10.0f};
float object[] = {0.0f, 0.0f, 0.0f};
float normal[] = {0.0f, 1.0f, 0.0f};

/* Particle system definitions */
int rainbow=FALSE;
float slowdown = 5000.0f; /* Slow Down Particles                                */
float xspeed;          /* Base X Speed (To Allow Keyboard Direction Of Tail) */
float yspeed=200;          /* Base Y Speed (To Allow Keyboard Direction Of Tail) */
float zoom = -10.0f;   /* Used To Zoom Out                                   */

GLuint loop;           /* Misc Loop Variable                                 */
GLuint col = 0;        /* Current Color Selection                            */
GLuint delay;          /* Rainbow Effect Delay                               */
Shader shader,shader2;


/* Create our particle structure */
typedef struct
{
    int   active; /* Active (Yes/No) */
    float life;   /* Particle Life   */
    float fade;   /* Fade Speed      */

    float r;      /* Red Value       */
    float g;      /* Green Value     */
    float b;      /* Blue Value      */

    float x;      /* X Position      */
    float y;      /* Y Position      */
    float z;      /* Z Position      */

    float xi;     /* X Direction     */
    float yi;     /* Y Direction     */
    float zi;     /* Z Direction     */

    float xg;     /* X Gravity       */
    float yg;     /* Y Gravity       */
    float zg;     /* Z Gravity       */
} particle;

/* Rainbow of colors */
static GLfloat colors[12][3] =
{
        { 1.0f,  0.5f,  0.5f},
	{ 1.0f,  0.75f, 0.5f},
	{ 1.0f,  1.0f,  0.5f},
	{ 0.75f, 1.0f,  0.5f},
        { 0.5f,  1.0f,  0.5f},
	{ 0.5f,  1.0f,  0.75f},
	{ 0.5f,  1.0f,  1.0f},
	{ 0.5f,  0.75f, 1.0f},
        { 0.5f,  0.5f,  1.0f},
	{ 0.75f, 0.5f,  1.0f},
	{ 1.0f,  0.5f,  1.0f},
	{ 1.0f,  0.5f,  0.75f}
};

/* Our beloved array of particles */
particle particles[MAX_PARTICLES];

/* This is our SDL surface */
SDL_Surface *surface;
char filelocation[] = "data/plate.bmp";
char cubefilename[] = "data/cube3.bmp";

/* Storage For One Texture ( NEW ) */
GLuint texture[3];

/* Music functionality */
int running;

/* Update the positions of Obstacles */
void updateObsPostion(float dtime){
	int len = obsData.size();
	for(int i=0; i<len; i++){
		obsData.at(i).check(dtime);
	}
}

/* Cleaning up simulation */
void SimQuit(){
	int len = (int) simWall.size();
	for(int i=0; i<len; i++)
		delete simWall.at(i);
	delete simCube;
	delete dynamicsWorld;
}

void Quit( int returnCode )
{
	SimQuit();
    SDL_Quit( );
    exit( returnCode );
}

void audio_callback(void *udata, Uint8 *stream, int len)
{
  static int pos=0;

  if((pos+len)>wav_bytes)
  {
    int maxlen=wav_bytes-(pos+len);
    running=0;

    if(maxlen>0)
    {
      memcpy(stream,audio_buf+pos,maxlen);
      pos+=maxlen;
    }
  }
  else
  {
    memcpy(stream,audio_buf+pos,len);
    pos+=len;
    fprintf(stderr,".");
  }
}

/* function to load in bitmap as a GL texture */
int LoadGLTextures( )
{
    int Status = FALSE;
    SDL_Surface *TextureImage[3];

    if ( ( TextureImage[0] = SDL_LoadBMP( filelocation) ) )
        {
	    Status = TRUE;
	    glGenTextures( 1, &texture[0] );
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[0]->w,
			  TextureImage[0]->h, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

    if ( TextureImage[0] )
	    SDL_FreeSurface( TextureImage[0] );//GLuint texture[1];     /* Storage For Our Particle Texture                   */

/* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage[1] = SDL_LoadBMP( "data/particle.bmp" ) ) )
        {

	    /* Set the status to true */
	    Status = TRUE;

	    /* Create The Texture */
	    glGenTextures( 1, &texture[1] );

	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[1] );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[1]->w,
			  TextureImage[1]->h, 0, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage[1]->pixels );

	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }

    /* Free up any memory we may have used */
    if ( TextureImage[1] )
	    SDL_FreeSurface( TextureImage[1] );

    /* Texture for the cube map */

    if ( ( TextureImage[2] = SDL_LoadBMP( cubefilename) ) )
        {
	    Status = TRUE;
	    glGenTextures( 1, &texture[2] );
	    glBindTexture( GL_TEXTURE_2D, texture[2] );

	    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[2]->w,
			  TextureImage[2]->h, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage[2]->pixels );

	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

    if ( TextureImage[2] )
	    SDL_FreeSurface( TextureImage[2] );     /* Storage For Our Particle Texture                   */

    return Status;
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
    /* Height / width ration */
    GLfloat ratio;

    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;
    glViewport( 0, 0, ( GLint )width, ( GLint )height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    gluPerspective( 60.0f, ratio, 1.0f, 1000.0f );

    glMatrixMode( GL_MODELVIEW );

    glLoadIdentity( );

    scene = new fbo(2*SCREEN_WIDTH,2*SCREEN_HEIGHT);
    scene->initFrameBuffer();
    scene2 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene2->initFrameBuffer();
    scene3 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene3->initFrameBuffer();
    scene4 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene4->initFrameBuffer();

    return(TRUE);
}


/* function to reset one particle to initial state */
/* NOTE: I added this function to replace doing the same thing in several
 * places and to also make it easy to move the pressing of numpad keys
 * 2, 4, 6, and 8 into handleKeyPress function.
 */
void ResetParticle( int num, int color, float xDir, float yDir, float zDir )
{
    /* Make the particels active */
    particles[num].active = TRUE;
    /* Give the particles life */
    particles[num].life = 1.0f;
    /* Random Fade Speed */
    particles[num].fade = ( float )( rand( ) %100 ) / 1000.0f + 0.00003f;
    /* Select Red Rainbow Color */
    particles[num].r = colors[color][0];
    /* Select Green Rainbow Color */
    particles[num].g = colors[color][1];
    /* Select Blue Rainbow Color */
    particles[num].b = colors[color][2];
    /* Set the position on the X axis */
    particles[num].x = 0.0f;
    /* Set the position on the Y axis */
    particles[num].y = 0.0f;
    /* Set the position on the Z axis */
    particles[num].z = 0.0f;
    /* Random Speed On X Axis */
    particles[num].xi = xDir;
    /* Random Speed On Y Axi */
    particles[num].yi = yDir;
    /* Random Speed On Z Axis */
    particles[num].zi = zDir;
    /* Set Horizontal Pull To Zero */
    particles[num].xg = 0.0f;
    /* Set Vertical Pull Downward */
    particles[num].yg = -0.03f;
    /* Set Pull On Z Axis To Zero */
    particles[num].zg = 0.0f;

    return;
}


float mod(float a)
{
	if(a>=0.00)
		return a;
	else
	   return -a;
}
/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
	btVector3 velocity(0.0,0.0,0.0);
    float xpos = simCube->getCenterOfMassPosition().getX();
    float zpos = simCube->getCenterOfMassPosition().getZ();

    float xdir = xpos - eye[0];
    float zdir = zpos - eye[2];

    float magnitude = sqrt(xdir*xdir + zdir*zdir);

    xdir = xdir/magnitude;
    zdir = zdir/magnitude;

    float pxdir,pzdir;
    if (zdir>xdir){
    	pxdir = zdir;
    	pzdir = -xdir;
    }
    else{
    	pxdir = -zdir;
    	pzdir = xdir;
    }

	switch( (keysym->sym) ){
		case SDLK_ESCAPE:
			Quit(0);
			break;
		case SDLK_F1:
		    /* F1 key was pressed
		     * this toggles fullscreen mode
		     */
		    SDL_WM_ToggleFullScreen( surface );
		    break;
		case SDLK_KP_PLUS:
		    /* '+' key was pressed
		     * this speeds up the particles
		     */
		    if ( slowdown > 1.0f )
			slowdown -= 0.01f;
		    break;
		case SDLK_KP_MINUS:
		    /* '-' key was pressed
		     * this slows down the particles
		     */
		    if ( slowdown < 4.0f )
			slowdown += 0.01f;
		    break;
		case SDLK_PAGEUP:
		    /* PageUp key was pressed
		     * this zooms into the scene
		     */
		    zoom += 0.01f;
		    break;
		case SDLK_PAGEDOWN:
		    /* PageDown key was pressed
		     * this zooms out of the scene
		     */
		    zoom -= 0.01f;
		    break;
		case SDLK_LEFT:
			velocity = simCube->getLinearVelocity();
			if (velocity.length() < 8.0)
				simCube->applyCentralForce(btVector3(-6.0*pxdir,0, -6.0*pzdir));
			break;
	    case SDLK_RIGHT:
			velocity = simCube->getLinearVelocity();
			if (velocity.length() < 10.0)
				simCube->applyCentralForce(btVector3(6.0*pxdir,0, 6.0*pzdir));
			break;
	    case SDLK_UP:
			velocity = simCube->getLinearVelocity();
			if (velocity.length() < 10.0)
				simCube->applyCentralForce(btVector3(6.0*xdir,0,6.0*zdir));
			break;
	    case SDLK_DOWN:
			velocity = simCube->getLinearVelocity();
			if (velocity.length() < 10.0)
				simCube->applyCentralForce(btVector3(-6.0*xdir,0,-6.0*zdir));
			break;
	    case SDLK_SPACE:
	    	if (!(simCube->getLinearVelocity().getY() > 0.2 || simCube->getLinearVelocity().getY() < -0.2))
	    		simCube->setLinearVelocity(btVector3(simCube->getLinearVelocity().getX(), (btScalar) 10.0,simCube->getLinearVelocity().getZ()));
	    	break;
		default:
	       break;
	  }

    return;
}

/* handling mouse event */

void mouseMovement(int x,int y,int z) {
	static double xprev=0.0, yprev=0.0, zprev=1.0;
	double xnew = (double) x;
	double ynew = (double) y;
	double znew;
	xnew = xnew/((double) SCREEN_WIDTH/2) - 1.0;
	ynew = 1.0 - ynew/((double) SCREEN_HEIGHT/2);
	double zb = 1.0-xnew*xnew-ynew*ynew;
	if (zb>0)
		znew = sqrt(zb);
	else
		znew = 0.0;

	double radius = sqrt(eyec[0]*eyec[0]+eyec[1]*eyec[1]+eyec[2]*eyec[2]);
	eyec[0] = radius*xnew;
	//eyec[1] = radius*ynew;
	eyec[2] = radius*znew;

	xprev = xnew;
	yprev = ynew;
	zprev = znew;
}


/* initializing the simulation environment */
int initSim(void){
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

	btRigidBody::btRigidBodyConstructionInfo *boxRigidBodyCI;

	/* simulating the cube */
	shape = new btBoxShape(btVector3((cubeData.max.v[0] - cubeData.min.v[0])/2,(cubeData.max.v[1] - cubeData.min.v[1])/2,(cubeData.max.v[2] - cubeData.min.v[2])/2));
	trans.setIdentity();
	qtn.setEuler(0.8,0.7,0.4);
	trans.setRotation(qtn);
	trans.setOrigin(btVector3(btVector3((cubeData.min.v[0]+cubeData.max.v[0])/2,(cubeData.min.v[1]+cubeData.max.v[1])/2,(cubeData.min.v[2]+cubeData.max.v[2])/2)));
	motionState = new btDefaultMotionState(trans);
	btScalar mass = 1;
	btVector3 Intertia(0,0,0);
	shape->calculateLocalInertia(mass, Intertia);
	boxRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, Intertia);
	boxRigidBodyCI->m_restitution = 0.4;
	simCube = new btRigidBody(*boxRigidBodyCI);
	simCube->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(simCube);

	/* simulating the playfield */
	int len = (int) wallData.size();
	for(int i=0; i<len; i++){
		shape = new btBoxShape(btVector3((wallData.at(i).max.v[0] - wallData.at(i).min.v[0])/2,(wallData.at(i).max.v[1] - wallData.at(i).min.v[1])/2,(wallData.at(i).max.v[2] - wallData.at(i).min.v[2])/2));
		trans.setIdentity();
		qtn.setEuler(0, 0.0, 0.0);
		trans.setRotation(qtn);
		trans.setOrigin(btVector3((wallData.at(i).max.v[0] + wallData.at(i).min.v[0])/2,(wallData.at(i).max.v[1] + wallData.at(i).min.v[1])/2,(wallData.at(i).max.v[2] + wallData.at(i).min.v[2])/2));
		motionState = new btDefaultMotionState(trans);
		boxRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(btScalar(0.0), motionState, shape, btVector3(0,0,0));
		boxRigidBodyCI->m_friction = 0.1;
		simWall.push_back(new btRigidBody(*boxRigidBodyCI));
		dynamicsWorld->addRigidBody(simWall.at(i));
	}

	/* simulating the obstacles */
	len = (int) obsData.size();
	for(int i=0; i<len; i++){
		shape = new btBoxShape(btVector3((obsData.at(i).max.v[0] - obsData.at(i).min.v[0])/2,(obsData.at(i).max.v[1] - obsData.at(i).min.v[1])/2,(obsData.at(i).max.v[2] - obsData.at(i).min.v[2])/2));
		trans.setIdentity();
		qtn.setEuler(0, 0.0, 0.0);
		trans.setRotation(qtn);
		trans.setOrigin(btVector3((obsData.at(i).max.v[0] + obsData.at(i).min.v[0])/2,(obsData.at(i).max.v[1] + obsData.at(i).min.v[1])/2,(obsData.at(i).max.v[2] + obsData.at(i).min.v[2])/2));
		motionState = new btDefaultMotionState(trans);
		boxRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(btScalar(0.0), motionState, shape, btVector3(0,0,0));
		boxRigidBodyCI->m_friction = 0.1;
		simObs.push_back(new btRigidBody(*boxRigidBodyCI));
		simObs.at(i)->setCollisionFlags( simObs.at(i)->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		simObs.at(i)->setActivationState(DISABLE_DEACTIVATION);
		dynamicsWorld->addRigidBody(simObs.at(i));
	}

	return 1;
}
/* general OpenGL initialization function */
int initGL(void)
{

	int argc = 1;
	char *argv = "freaking stud";
	glutInit(&argc, &argv);

    if ( !LoadGLTextures( ) )
		return FALSE;

	glewInit();

	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	glClearColor(0.0f, 0.0f, 0.0f ,1.0f);
    glEnable( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );

    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
    glClearDepth( 1.0f );
  // glEnable(GL_LIGHTING);
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    /* Enable Blending */
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
    /* Enable Texture Mapping */
    glEnable( GL_TEXTURE_2D );
    /* Select Our Texture */
    glBindTexture( GL_TEXTURE_2D, texture[1] );


    /* Reset all the particles */
    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
	{
	    int color = ( loop + 1 ) / ( MAX_PARTICLES / 12 );
	    float xi, yi, zi;
	    xi =  ( float )( ( rand( ) % 50 ) - 26.0f ) * 10.0f;
	    yi = zi = ( float )( ( rand( ) % 50 ) - 25.0f ) * 10.0f;

	    ResetParticle( loop, color, xi, yi, zi );
     }



    vector<string> data1, data2, data3, data4, data5;
    readXML(filename, data1, data2, data3, data4, data5);
    getWallData(data1, wallData);
    getObsData(data2, obsData);
    getHoleData(data3, holesData);
    getFlameData(data4, flameData);
    getPowerData(data5, powerData);

	if (!initSim()){
		cout <<"Problem with simulation initialization\n";
		Quit(0);
	}

    glMatrixMode(GL_MODELVIEW);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition);
    glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);

    shader.init(vertex_shader,fragment_shader);
    shader2.init(vertex_shader_msaa,fragment_shader_msaa);


    scene = new fbo(2*SCREEN_WIDTH,2*SCREEN_HEIGHT);
    scene->initFrameBuffer();
    scene2 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene2->initFrameBuffer();
    scene3 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene3->initFrameBuffer();
    scene4 = new fbo(SCREEN_WIDTH,SCREEN_HEIGHT);
    scene4->initFrameBuffer();


    return( TRUE );
}

void drawHoles(float* pos, float radius){
    glLoadIdentity( );
    gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
    glColor3f(1.0f,0.0f,0.0f);
    glTranslatef(pos[0], pos[1], pos[2]);
    glutWireSphere(radius, 8, 8);
}

void drawParticles(float* pos){
	/* Select Our Texture */

		glLoadIdentity();
		gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
	    glBindTexture( GL_TEXTURE_2D, texture[1] );
	    glTranslatef(pos[0],pos[1],pos[2]);
	    /* Modify each of the particles */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		{
		    if ( particles[loop].active )
			{
			    /* Grab Our Particle X Position */
			    float x = particles[loop].x;
			    /* Grab Our Particle Y Position */
			    float y = particles[loop].y;
			    /* Particle Z Position + Zoom */
			    float z = particles[loop].z + zoom;

			    /* Draw The Particle Using Our RGB Values,
			     * Fade The Particle Based On It's Life
			     */
			    glColor4f( particles[loop].r,
				       particles[loop].g,
				       particles[loop].b,
				       particles[loop].life );

			    /* Build Quad From A Triangle Strip */
			    glBegin( GL_TRIANGLE_STRIP );
			      /* Top Right */
			      glTexCoord2d( 1, 1 );
			      glVertex3f( x + 0.2f, y + 0.2f, z );
			      /* Top Left */
			      glTexCoord2d( 0, 1 );
			      glVertex3f( x - 0.2f, y + 0.2f, z );
			      /* Bottom Right */
			      glTexCoord2d( 1, 0 );
			      glVertex3f( x + 0.2f, y - 0.2f, z );
			      /* Bottom Left */
			      glTexCoord2d( 0, 0 );
			      glVertex3f( x - 0.2f, y - 0.2f, z );
			    glEnd( );

			    /* Move On The X Axis By X Speed */
			    particles[loop].x += particles[loop].xi /
				( slowdown * 100 );
			    /* Move On The Y Axis By Y Speed */
			    particles[loop].y += particles[loop].yi /
				( slowdown * 100  );
			    /* Move On The Z Axis By Z Speed */
			    particles[loop].z += particles[loop].zi /
				( slowdown * 75 );

			    /* Take Pull On X Axis Into Account */
			    particles[loop].xi += particles[loop].xg;
			    /* Take Pull On Y Axis Into Account */
			    particles[loop].yi += particles[loop].yg;
			    /* Take Pull On Z Axis Into Account */
			    particles[loop].zi += particles[loop].zg;

			    /* Reduce Particles Life By 'Fade' */
			    particles[loop].life -= particles[loop].fade;

			    /* If the particle dies, revive it */
			    if ( particles[loop].life < 0.0f )
				{
				    float xi, yi, zi;
				    xi = xspeed +
					( float )( ( rand( ) % 60 ) - 32.0f );
				    yi = yspeed +
					( float)( ( rand( ) % 60 ) - 30.0f );
				    zi = ( float )( ( rand( ) % 60 ) - 30.0f );
				    ResetParticle( loop, col, xi, yi, zi );
	                        }
			}
		}

}

void drawRect(float* min, float* max){
	if (min[0] == max[0]){
		float x = min[0], y, z;
		for(y = min[1]; y < max[1]; y++){
			for(z = min[2]; z< max[2]; z++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+1.0, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+1.0, z+1.0);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z+1.0);
				glEnd();
			}
		}
	}
	else if(min[1] == max[1]){
		float y = min[1], x, z;
		for(x = min[0]; x < max[0]; x++){
			for(z = min[2]; z < max[2]; z++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x+1.0, y, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1.0, y, z+1);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z+1.0);
				glEnd();
			}
		}
	}
	else{
		float z = min[2], y, x;
		for(x = min[0]; x < max[0]; x++){
			for(y = min[1]; y < max[1]; y++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+1.0, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1.0, y+1.0, z);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x+1.0, y, z);
				glEnd();
			}
		}
	}
}


int drawGLScene( void )
{
    /* These are to calculate our fps */
    static GLint T0,T1     = 0;
    static GLint Frames = 0;
    int len;
    location vertexData[12];

    /* Supply Blur factor */
    static float blur_factor = 1.5;
    static float change = 1.0;

    float xpos = simCube->getCenterOfMassPosition().getX();
    float ypos = simCube->getCenterOfMassPosition().getY();
    float zpos = simCube->getCenterOfMassPosition().getZ();

    if (ypos < -40.0)
    	Quit(0);

	object[0] = xpos;
	object[1] = ypos;
	object[2] = zpos;

	eye[0] = xpos + eyec[0];
	eye[1] = ypos + eyec[1];
	eye[2] = zpos + eyec[2];

	scene->bind();
	glClearColor(0.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
    glLoadIdentity( );
    gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
    glViewport(0,0, 2*SCREEN_WIDTH, 2*SCREEN_HEIGHT);

    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    /* Draw 1 into the stencil buffer. */
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

    /* Now render floor; floor pixels just get their stencil set to 1. */
    /* Here I need to render the top of the floor only where the stencil buffer values would be set to 1
	   Thus first check which cube and then draw only the top surface*/
    int mycube=0;
    glColor3f(1.0, 1.0, 1.0);
    len = (int) wallData.size();
    for(int j=0; j<len; j++){
    /*I need to check which cube should render the reflecting surface*/
    	if(xpos<=(wallData[j]).max.v[0] && xpos>=(wallData[j]).min.v[0])
    	{
    		if(zpos<=(wallData[j]).max.v[2]  && zpos>=(wallData[j]).min.v[2])
    		{
    			mycube=j;
    			to_reflect=1;
    			break;
    		}
		}
	}
       
	float min[3], max[3];
	min[0] = (wallData[mycube]).min.v[0];
	min[1] = (wallData[mycube]).max.v[1];	
	min[2] = (wallData[mycube]).min.v[2];
	max[0] = (wallData[mycube]).max.v[0];
	max[1] = (wallData[mycube]).max.v[1];
	max[2] = (wallData[mycube]).max.v[2];
	drawRect(min, max);
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    /* Now, only render where stencil is set to 1. */
    glStencilFunc(GL_EQUAL, 1, 0xf);  /* draw if ==1 */
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_NORMALIZE);

    simCube->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(matrix);
    glTranslatef(0,2*wallData[mycube].max.v[1],0);
    glScalef(1.0, -1.0, 1.0);
    glMultMatrixf(matrix);
   
    if(to_reflect==1)
     {
    	 /* Drawing the reflected cube */
    	 glBindTexture(GL_TEXTURE_2D, texture[2]);
    	 wall newcubeData(location((cubeData.min.v[0]-cubeData.max.v[0])/2,(cubeData.min.v[1]-cubeData.max.v[1])/2,(cubeData.min.v[2]-cubeData.max.v[2])/2),
    	     		 location((-cubeData.min.v[0]+cubeData.max.v[0])/2,(-cubeData.min.v[1]+cubeData.max.v[1])/2,(-cubeData.min.v[2]+cubeData.max.v[2])/2));
    	 newcubeData.generateRect(vertexData);
    	 for(int i=0; i<6; i++){
    		 drawRect((vertexData[2*i]).v, (vertexData[2*i+1]).v);
    	 }
     }

     to_reflect=0;
     /* Disable noramlize again and re-enable back face culling. */
     glDisable(GL_NORMALIZE);
     glDisable(GL_STENCIL_TEST);
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, texture[0]);
     glLoadIdentity( );
     gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glColor4f(1.0, 1.0, 1.0,0.8f);
     drawRect(min, max);
     glDisable(GL_BLEND);
     len = (int) wallData.size();
     for(int j=0; j<len; j++){
    	 location vertexData[12];
    	 (wallData.at(j)).generateRect(vertexData);
    	 for(int i = 0; i < 6; i++){
    		 if (i!=4 || mycube!=j)
    			 drawRect((vertexData[2*i]).v, (vertexData[2*i+1]).v);
    	 }
     }
     
     glLoadIdentity( );
     gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
    /* Drawing the moving cube */
     glBindTexture(GL_TEXTURE_2D, texture[2]);
     simCube->getMotionState()->getWorldTransform(trans);
     trans.getOpenGLMatrix(matrix);
     glMultMatrixf(matrix);
     wall newcubeData(location((cubeData.min.v[0]-cubeData.max.v[0])/2,(cubeData.min.v[1]-cubeData.max.v[1])/2,(cubeData.min.v[2]-cubeData.max.v[2])/2),
    		 location((-cubeData.min.v[0]+cubeData.max.v[0])/2,(-cubeData.min.v[1]+cubeData.max.v[1])/2,(-cubeData.min.v[2]+cubeData.max.v[2])/2));
     newcubeData.generateRect(vertexData);
     for(int i=0; i<6; i++){
    	 drawRect((vertexData[2*i]).v, (vertexData[2*i+1]).v);
     }

     /* Draw the obstacles */
     glLoadIdentity( );
     gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);
     glBindTexture(GL_TEXTURE_2D, texture[0]);
     glColor3f(1.0f,1.0f,1.0f);
     len = (int) obsData.size();
     for(int i=0; i<len; i++){
    	 trans = simObs.at(i)->getWorldTransform();
    	 trans.setOrigin(btVector3((obsData.at(i).curr_min.v[0]+obsData.at(i).curr_max.v[0])/2,(obsData.at(i).curr_min.v[1]+obsData.at(i).curr_max.v[1])/2,(obsData.at(i).curr_min.v[2]+obsData.at(i).curr_max.v[2])/2));
    	 simObs.at(i)->setWorldTransform(trans);
    	 simObs.at(i)->getMotionState()->getWorldTransform(trans);
    	 trans.setOrigin(btVector3((obsData.at(i).curr_min.v[0]+obsData.at(i).curr_max.v[0])/2,(obsData.at(i).curr_min.v[1]+obsData.at(i).curr_max.v[1])/2,(obsData.at(i).curr_min.v[2]+obsData.at(i).curr_max.v[2])/2));
    	 simObs.at(i)->getMotionState()->setWorldTransform(trans);
    	 location vertexData[12];
    	 (obsData.at(i)).generateRect(vertexData);
    	 for(int i = 0; i < 6; i++){
    			 drawRect((vertexData[2*i]).v, (vertexData[2*i+1]).v);
    	 }
     }

     /* Draw Black Holes */
     glColor3f(1.0f, 1.0f, 1.0f);
     len = holesData.size();
     for(int i=0; i<len; i++)
     	 drawHoles((holesData.at(i)).pos.v, (holesData.at(i)).radius);
     glColor3f(1.0f,1.0f,1.0f);

     /* Draw flame to the screen */
    glEnable( GL_BLEND );
    /* Type Of Blending To Perform */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    /* Really Nice Point Smoothing */
    len = (int) flameData.size();
    for(int i=0; i<len; i++)
		drawParticles((flameData.at(i)).v);
    glDisable(GL_BLEND);
    glColor3f(1.0f,1.0f,1.0f);

    glPopMatrix();
    scene->unbind();


    /* Horizontal Blur */
    scene2->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    scene->textureBind();
    shader.bind();
    GLuint tex_location = glGetUniformLocation(shader.id(), "tex0");
    glUniform1i(tex_location, 0);
    GLuint offset_location = glGetUniformLocation(shader.id(), "sampleOffset");
    glUniform2f(offset_location, blur_factor/(float) SCREEN_WIDTH, 0.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0,-1.0); // The bottom left corner

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f); // The top left corner

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f); // The top right corner

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f); // The bottom right corner

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    shader.unbind();
    scene->textureUnbind();
    glDisable(GL_TEXTURE0);
    scene2->unbind();

    /* Vertically Blur */
    scene3->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    scene2->textureBind();
    shader.bind();
    glUniform2f(offset_location, 0.0, blur_factor/(float) SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0,-1.0); // The bottom left corner

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f); // The top left corner

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f); // The top right corner

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f); // The bottom right corner

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    shader.unbind();
    scene2->textureUnbind();
    glDisable(GL_TEXTURE0);
    scene3->unbind();

    /* Blending all the scene and drawing them the main scene */
    scene4->bind();
    shader2.bind();
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    scene->textureBind();
    /*shader2.bind();
    tex_location = glGetUniformLocation(shader.id(), "tex0");
    glUniform1i(tex_location, 0);
    offset_location = glGetUniformLocation(shader.id(), "sampleOffset");
    glUniform2f(offset_location, 2.0/(float) SCREEN_WIDTH, 0.0);*/
    glViewport(0,0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0,-1.0); // The bottom left corner

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f); // The top left corner

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f); // The top right corner

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f); // The bottom right corner

	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	scene->textureUnbind();
	/* End */

	/* Draw the horizontally blurred scene */
	glActiveTexture(GL_TEXTURE0);
	scene2->textureBind();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0,-1.0); // The bottom left corner

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f); // The top left corner

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f); // The top right corner

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f); // The bottom right corner
	glEnd();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    scene2->textureUnbind();
    /* End */

    /* Draw vertically blurred scene */
    glActiveTexture(GL_TEXTURE0);
	scene3->textureBind();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0,-1.0); // The bottom left corner

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f); // The top left corner

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f); // The top right corner

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f); // The bottom right corner

	glEnd();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_BLEND);
	scene3->textureUnbind();
    /* End */
	shader2.unbind();
	scene4->unbind();



	/*now draw scene 4*/
	glActiveTexture(GL_TEXTURE0);
		scene4->textureBind();

		glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0,-1.0); // The bottom left corner

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, 1.0f); // The top left corner

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f); // The top right corner

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, -1.0f); // The bottom right corner

		glEnd();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		/* End */

	SDL_GL_SwapBuffers( );

    /* Gather our frames per second */
    Frames++;
    {
	GLint t = SDL_GetTicks();
	if (t - T0 >= 5000) {
	    GLfloat seconds = (t - T0) / 1000.0;
	    GLfloat fps = Frames / seconds;
	    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
	    T0 = t;
	    col = (col+1) % 12;
	    Frames = 0;
	}
	updateObsPostion((float)(t-T1)/ 1000.0);

	blur_factor += change*((float)(t-T1)/ 1000.0);
	if (blur_factor >= 2.5 || blur_factor <= 0.5){
		change = -change;
	}

	T1 = t;
    }

    return( TRUE );
}

/* Simulation Timer */
void SimTime(void){
	float dtime = simTime;
	simTime = glutGet(GLUT_ELAPSED_TIME) / 500.0;
	dtime = simTime - dtime;

	if(dynamicsWorld)
	dynamicsWorld->stepSimulation(dtime, 10);
	drawGLScene();
}

int main( int argc, char **argv )
{
    int videoFlags;
    int done = FALSE;
    SDL_Event event;
    const SDL_VideoInfo *videoInfo;
    int isActive = TRUE;
    SDL_AudioSpec awant,aget,wavefmt;
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

  if(getenv("SDL_AUDIO_DRIVER")!=NULL)
  {
    if(SDL_AudioInit(getenv("SDL_AUDIO_DRIVER"))<0)
    {
      fprintf(stderr,"Couldn't init audio driver \"%s\"\n",
        getenv("SDL_AUDIO_DRIVER"));
      SDL_Quit();
      return(4);
    }
  }
  else
  {
    if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0)
    {
      fprintf(stderr,"Couldn't init audio: %s\n",SDL_GetError());
      return(5);
    }
  }

    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

    if ( videoInfo->hw_available )
	videoFlags |= SDL_HWSURFACE;
    else
	videoFlags |= SDL_SWSURFACE;

    if ( videoInfo->blit_hw )
	videoFlags |= SDL_HWACCEL;



    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT,32,
				videoFlags );

    if ( !surface )
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    Quit( 1 );
	}

    if ( ( SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL ) ) )
    {
   	    fprintf( stderr, "Setting keyboard repeat failed: %s\n",
   		     SDL_GetError( ) );
   	    Quit( 1 );
    }

    /* initialize OpenGL */
    initGL( );

    resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );
      {
    char namebuf[512];
    if(SDL_AudioDriverName(namebuf,512)==NULL)
      fprintf(stderr,"No audio driver\n");
    else
      fprintf(stderr,"Audio driver: %s\n",namebuf);
  }

  awant.freq=22050;
  awant.format=AUDIO_S16SYS;
  awant.channels=1;
  awant.samples=4096;
  awant.callback=audio_callback;
  awant.userdata=&aget;

  if(SDL_OpenAudio(&awant,&aget)!=0)
  {
    fprintf(stderr,"Couldn't open audio device: %s\n",SDL_GetError());
    SDL_Quit();
    return(1);
  }

  fprintf(stderr,"Audio initialized at %d hz\n",aget.freq);

  if(SDL_LoadWAV(WAV_FILE,&wavefmt,&audio_buf,&wav_bytes)==NULL)
  {
    fprintf(stderr,"Could not load WAV file: %s",SDL_GetError());
    SDL_CloseAudio();
    SDL_Quit();
    return(1);
  }

  fprintf(stderr,"Loaded %s\n",WAV_FILE);
  fprintf(stderr,"%d bytes going into conversion...\n",wav_bytes);
  {
    SDL_AudioCVT cvt;
    int retval;
    fprintf(stderr,"SDL_BuiltAudioCVT(%p,0x%08x,%d,%d,0x%08x,%d,%d)=",
            &cvt,wavefmt.format,wavefmt.channels,wavefmt.freq,
            aget.format,aget.channels,aget.freq);


    retval=SDL_BuildAudioCVT(&cvt,wavefmt.format,wavefmt.channels,
                             wavefmt.freq,aget.format,aget.channels,aget.freq);
    fprintf(stderr,"%d\n",retval);
    if(retval<0)
    {
      fprintf(stderr,"Couldn't create audio converter: %s\n",SDL_GetError());
      free(audio_buf);
      SDL_CloseAudio();
      SDL_Quit();
      return(2);
    }

	{ /* Cannot use realloc, since windows mungs malloc() up for you */
      Uint8 *nbuf=(Uint8 *)malloc(wav_bytes*cvt.len_mult);
      if(nbuf==NULL)
	  {
        fprintf(stderr,"Couldn't create buffer\n");
		free(audio_buf);
		SDL_CloseAudio();
		SDL_Quit();
		return(5);
	  }
	  memcpy(nbuf,audio_buf,wav_bytes);
	  SDL_FreeWAV(audio_buf);
	  audio_buf=nbuf;
	}

    cvt.len=wav_bytes;
    cvt.buf=audio_buf;
    wav_bytes=cvt.len*cvt.len_mult;
    SDL_ConvertAudio(&cvt);
  }
  fprintf(stderr,"%d bytes coming out of conversion.\n",wav_bytes);

  fprintf(stderr,"Playback test:\n");

  running=1;
  SDL_PauseAudio(0);


    while ( !done )
	{

	    while ( SDL_PollEvent( &event ) )
		{
		    switch( event.type )
			{
			case SDL_ACTIVEEVENT:
			    if ( event.active.gain == 0 )
				isActive = TRUE;
			    else
				isActive = TRUE;
			    break;
			case SDL_VIDEORESIZE:
			    surface = SDL_SetVideoMode( event.resize.w,
							event.resize.h,
							16, videoFlags );
			    if ( !surface )
				{
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    Quit( 1 );
				}
			    resizeWindow( event.resize.w, event.resize.h );
			    break;
			case SDL_KEYDOWN:
			    handleKeyPress( &event.key.keysym );
			    break;
			case SDL_QUIT:
			    done = TRUE;
			    break;
			case SDL_MOUSEMOTION:
				mouseMovement((int) event.motion.x, (int) event.motion.y, (int) event.motion.state);
				break;
			default:
			    break;
			}
		}

	    /* If rainbow coloring is turned on, cycle the colors */
	    if ( rainbow && ( delay > 25 ) )
		col = (col+1) % 12;

	    if ( isActive ){
	    	drawGLScene( );
	    	SimTime();
	    }
	    delay++;
	}
	SDL_PauseAudio(1);

  fprintf(stderr,"Playback test done\n");

  SDL_CloseAudio();

  if(audio_buf!=NULL) free(audio_buf);


    Quit( 0 );
    return( 0 );
}
