#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>

#include "textfile.h"
#include "parser.cpp"


#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     16

#define TRUE  1
#define FALSE 0

/* Max number of particles */
#define MAX_PARTICLES 1000


using namespace std;

/* playfield sml */
char filename[] = "data/playfield.xml";

/* playfield data */
vector<wall> wallData;
vector<obstacle> obsData;
vector<holes> holesData;
vector<location> flameData, powerData;

/* Storage for display list of the playfield */
GLuint playfieldList;

/* variables specifying the camera attributes */
float eye[] = {0.0f, 3.5f, 10.0f};
float object[] = {0.0f, 0.0f, 0.0f};
float normal[] = {0.0f, 1.0f, 0.0f};

/* array specifying the screen*/
float field[][3] = {{12.0, 0.0, 3.0},{-12.0, 0.0, -15.0},
				  {12.0, 8.0, 3.0},{12.0, 0.0, -15.0},
				  {-12.0, 8.0, 3.0},{-12.0, 0.0, -15.0},
				  {-4.0, 8.0, -15.0},{ -12.0, 0.0, -15.0},
				  {12.0, 8.0, -15.0},{ 4.0, 0.0, -15.0},
				  {4.0, 3.0, -15.0},{-4.0, 0.0, -15.0},
				  {4.0, 3.0, -15.0},{-4.0, 3.0, -25.0},
				  {-4.0, 8.0, -15.0},{-4.0, 3.0, -25.0},
				  {4.0, 8.0, -15.0},{4.0, 3.0, -25.0}
				  };

int field_obj = 9;

int rainbow=TRUE;
float slowdown = 2.0f; /* Slow Down Particles                                */
float xspeed;          /* Base X Speed (To Allow Keyboard Direction Of Tail) */
float yspeed;          /* Base Y Speed (To Allow Keyboard Direction Of Tail) */
float zoom = -5.0f;   /* Used To Zoom Out                                   */

GLuint loop;           /* Misc Loop Variable                                 */
GLuint col = 0;        /* Current Color Selection                            */
GLuint delay;          /* Rainbow Effect Delay                               */
//GLuint texture[1];     /* Storage For Our Particle Texture                   */

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


/* objects specifying the shaders */
GLuint v,f,p;

/* light position */
float lpos[4] = {1,0.5,1,0};

/* This is our SDL surface */
SDL_Surface *surface;
char filelocation[] = "data/plate.bmp";

/* Storage For One Texture ( NEW ) */
GLuint texture[2];

/*angle of rotation*/
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0,xpos1=0.0,zpos1=0.0;
float delta=2;
float cRadius = 10.0f; // our radius distance from our character

float lastx, lasty;


void Quit( int returnCode )
{
    SDL_Quit( );
    exit( returnCode );
}

/* function to load in bitmap as a GL texture */
int LoadGLTextures( )
{
    int Status = FALSE;
    SDL_Surface *TextureImage[2]; 
    
    if ( ( TextureImage[0] = SDL_LoadBMP( filelocation) ) )
        {
	    Status = TRUE;
	    glGenTextures( 1, &texture[0] );
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[0]->w,
			  TextureImage[0]->h, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
        
    if ( TextureImage[0] )
	    SDL_FreeSurface( TextureImage[0] );

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


    return Status;
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;
 
    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;
    glViewport( 0, 0, ( GLint )width, ( GLint )height );
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    glMatrixMode( GL_MODELVIEW );

    glLoadIdentity( );

    return( TRUE );
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
    particles[num].fade = ( float )( rand( ) %100 ) / 1000.0f + 0.003f;
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
    particles[num].yg = -0.8f;
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
	 float xrotrad, yrotrad;
   
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
			   	yrotrad = (yrot / 180 * 3.141592654f);
			   	xpos1 -= mod(float(cos(yrotrad))) *delta*0.01;
			    	zpos1 -= mod(float(sin(yrotrad))) *delta*0.01;
				    /* Left arrow key was pressed
				     * this decreases the particles' x movement
				     */
				    if ( xspeed > -200.0f )
					xspeed--;

			    	break;
	        case SDLK_RIGHT:
				yrotrad = (yrot / 180 * 3.141592654f);
			    	xpos1 += mod(float(cos(yrotrad))) *delta*0.01;
    				zpos1 += mod(float(sin(yrotrad))) *delta*0.01;
				    /* Right arrow key was pressed
				     * this increases the particles' x movement
				     */
				    if ( xspeed < 200.0f )
					xspeed++;
		
	          		break;
	        case SDLK_UP:
	               		yrotrad = (yrot / 180 * 3.141592654f);
				xrotrad = (xrot / 180 * 3.141592654f); 
				xpos1 += mod(float(sin(yrotrad)))*delta*0.01;
				zpos1 -=mod(float(cos(yrotrad)))*delta*0.01;
				ypos -= float(sin(xrotrad));
			/*for the particle effect*/
			 	if ( yspeed < 200.0f )
					yspeed++;
	   			break;

	        case SDLK_DOWN:
	            		yrotrad = (yrot / 180 * 3.141592654f);
	            		xrotrad = (xrot / 180 * 3.141592654f);
	            		xpos1 -= mod(float(sin(yrotrad)))*delta*0.01;
	            		zpos1 += mod(float(cos(yrotrad)))*delta*0.01;;
	            		ypos += float(sin(xrotrad));
				/* Down arrow key was pressed
					     * this decreases the particles' y movement
					     */
				if ( yspeed > -200.0f )
					yspeed--;

	          		break;
	case SDLK_KP8:
	    /* NumPad 8 key was pressed
	     * increase particles' y gravity
	     */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		if ( particles[loop].yg < 1.5f )
		    particles[loop].yg += 0.01f;
	    break;
	case SDLK_KP2:
	    /* NumPad 2 key was pressed
	     * decrease particles' y gravity
	     */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		if ( particles[loop].yg > -1.5f )
		    particles[loop].yg -= 0.01f;
	    break;
	case SDLK_KP6:
	    /* NumPad 6 key was pressed
	     * this increases the particles' x gravity
	     */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		if ( particles[loop].xg < 1.5f )
		    particles[loop].xg += 0.01f;
	    break;
	case SDLK_KP4:
	    /* NumPad 4 key was pressed
	     * this decreases the particles' y gravity
	     */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		if ( particles[loop].xg > -1.5f )
		    particles[loop].xg -= 0.01f;
	    break;
	case SDLK_TAB:
	    /* Tab key was pressed
	     * this resets the particles and makes them re-explode
	     */
	    for ( loop = 0; loop < MAX_PARTICLES; loop++ )
		{
		   int color = ( loop + 1 ) / ( MAX_PARTICLES / 12 );
		   float xi, yi, zi;
		   xi = ( float )( ( rand( ) % 50 ) - 26.0f ) * 10.0f;
		   yi = zi = ( float )( ( rand( ) % 50 ) - 25.0f ) * 10.0f;

		   ResetParticle( loop, color, xi, yi, zi );
		}
	    break;
	case SDLK_RETURN:
	    /* Return key was pressed
	     * this toggles the rainbow color effect
	     */
	    rainbow = !rainbow;
	    delay = 25;
	    break;
	case SDLK_SPACE:
	    /* Spacebar was pressed
	     * this turns off rainbow-ing and manually cycles through colors
	     */
	    rainbow = FALSE;
	    delay = 0;
	    col = ( ++col ) % 12;
	    break;

	        default:
	          		break;
	  }

    return;
}

/* handling mouse event */
void mouseMovement(int x,int y,int z) {
if(z==1)
{
    int diffx=x-lastx; //check the difference between the current x and the last x position
    int diffy=y-lasty; //check the difference between the  current y and the last y position
    lastx=x; //set lastx to the current x position
    lasty=y; //set lasty to the current y position
    xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
    yrot += (float) diffx;    //set the xrot to yrot with the addition of the difference in the x position
}
   }


/* setting up shaders for the program */

void setShaders(){
	char *vs = NULL, *fs = NULL;
	
	vs = textFileRead("shaders/fxaa.vs");
	fs = textFileRead("shaders/fxaa.fs");
	
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	

	const char * ff = fs;
	const char * vv = vs;


	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

		
	glCompileShader(v);
	glCompileShader(f);
	
	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);

	glLinkProgram(p);
	glUseProgram(p);
}

void genDisplayList(){

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
		
	setShaders();
	
    //glEnable( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
    glClearDepth( 1.0f );
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

    genDisplayList();

    return( TRUE );
}


void drawRect(float* min, float* max){
	if (min[0] == max[0]){
		int x = min[0], y, z;
		for(y = min[1]; y < max[1]; y++){
			for(z = min[2]; z< max[2]; z++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+1, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y+1, z+1);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z+1);
				glEnd();
			}
		}
	}
	else if(min[1] == max[1]){
		int y = min[1], x, z;
		for(x = min[0]; x < max[0]; x++){
			for(z = min[2]; z < max[2]; z++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x+1, y, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1, y, z+1);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z+1);
				glEnd(); 
			}
		}	
	}
	else{
		int z = min[2], y, x;
		for(x = min[0]; x < max[0]; x++){
			for(y = min[1]; y < max[1]; y++){
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
					glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y+1, z);
					glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1, y+1, z);
					glTexCoord2f(0.0f, 0.0f); glVertex3f(x+1, y, z);
				glEnd();
			}
		}
	}
}


int drawGLScene( void )
{
    /* These are to calculate our fps */
    static GLint T0     = 0;
    static GLint Frames = 0;

	glClearColor(0.0f, 0.0f, 0.0f ,1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity( );
    gluLookAt(eye[0], eye[1], eye[2], object[0], object[1], object[2], normal[0], normal[1], normal[2]);

    glActiveTexture(GL_TEXTURE0);
    int texture_location = glGetUniformLocation(p, "texture");
    glUniform1i(texture_location, 0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);	
  	
    /*for(int i=0; i<field_obj; i++){
    	drawRect(field[2*i+1], field[2*i]);
    }*/

    int len = (int) wallData.size();
    for(int j=0; j<len; j++){
    location vertexData[12];
    (wallData.at(j)).generateRect(vertexData);
  	for(int i = 0; i < 6; i++){
  		drawRect((vertexData[2*i]).v, (vertexData[2*i+1]).v);
  	}
    }

    xpos=xpos+xpos1;
    zpos=zpos+zpos1;
    xpos1*=0.98;
    zpos1*=0.98;

    glTranslatef(xpos, 0.5f, zpos);
    glRotatef(xrot,1.0,0.0,0.0);
    glRotatef(yrot,0.0,0.0,1.0);
    /* Drawing the moving cube */
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidCube(1.0f);

 glEnable( GL_BLEND );
    /* Type Of Blending To Perform */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    /* Really Nice Point Smoothing */
   
    /* Select Our Texture */
    glBindTexture( GL_TEXTURE_2D, texture[1] );

    glLoadIdentity( );
    //glTranslatef(10.0f,0.0f,0.0f);
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
		      glVertex3f( x + 0.1f, y + 0.1f, z );
		      /* Top Left */
		      glTexCoord2d( 0, 1 );
		      glVertex3f( x - 0.1f, y + 0.1f, z );
		      /* Bottom Right */
		      glTexCoord2d( 1, 0 );
		      glVertex3f( x + 0.1f, y - 0.1f, z );
		      /* Bottom Left */
		      glTexCoord2d( 0, 0 );
		      glVertex3f( x - 0.1f, y - 0.1f, z );
		    glEnd( );

		    /* Move On The X Axis By X Speed */
		    particles[loop].x += particles[loop].xi /
			( slowdown * 1000 );
		    /* Move On The Y Axis By Y Speed */
		    particles[loop].y += particles[loop].yi /
			( slowdown * 1000 );
		    /* Move On The Z Axis By Z Speed */
		    particles[loop].z += particles[loop].zi /
			( slowdown * 1000 );

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

glDisable(GL_BLEND);


    /* Draw it to the screen */
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
	    Frames = 0;
	}
    }


    return( TRUE );
}

int main( int argc, char **argv )
{
    int videoFlags;
    int done = FALSE;
    SDL_Event event;
    const SDL_VideoInfo *videoInfo;
    int isActive = TRUE;
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
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
	
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
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
				mouseMovement( event.motion.xrel,event.motion.yrel,event.motion.state);
				break;
			default:
			    break;
			}
		}

	    /* If rainbow coloring is turned on, cycle the colors */
	    if ( rainbow && ( delay > 25 ) )
		col = ( ++col ) % 12;

	    if ( isActive )
		drawGLScene( );
	    delay++;

	}

    Quit( 0 );
    return( 0 );
}
