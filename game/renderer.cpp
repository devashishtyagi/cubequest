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

/* objects specifying the shaders */
GLuint v,f,p;

/* light position */
float lpos[4] = {1,0.5,1,0};

/* This is our SDL surface */
SDL_Surface *surface;
char filelocation[] = "data/plate.bmp";

/* Storage For One Texture ( NEW ) */
GLuint texture[1];

/*angle of rotation*/
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0,xpos1=0.0,zpos1=0.0;
float delta=1;
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
    SDL_Surface *TextureImage[1]; 
    
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
			case SDLK_LEFT:
				printf("IN left\n");
				printf("%f\n",yrot);
				yrotrad = (yrot / 180 * 3.141592654f);
				xpos1 -= mod(float(cos(yrotrad))) *delta*0.01;
				zpos1 -= mod(float(sin(yrotrad))) *delta*0.01;
				break;

	        case SDLK_RIGHT:
	        	printf("IN right\n");
	        	yrotrad = (yrot / 180 * 3.141592654f);
	        	xpos1 += mod(float(cos(yrotrad))) *delta*0.01;
	        	zpos1 += mod(float(sin(yrotrad))) *delta*0.01;
	        	break;
	        case SDLK_UP:
	        	printf("IN up\n");
	        	yrotrad = (yrot / 180 * 3.141592654f);
			    xrotrad = (xrot / 180 * 3.141592654f);
			    xpos1 += mod(float(sin(yrotrad)))*delta*0.01;
			    zpos1 -=mod(float(cos(yrotrad)))*delta*0.01;
			    ypos -= float(sin(xrotrad));
			    break;
	        case SDLK_DOWN:
	        	printf("IN down\n");
	            yrotrad = (yrot / 180 * 3.141592654f);
	            xrotrad = (xrot / 180 * 3.141592654f);
	            xpos1 -= mod(float(sin(yrotrad)))*delta*0.01;
	            zpos1 += mod(float(cos(yrotrad)))*delta*0.01;;
	            ypos += float(sin(xrotrad));
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
    int diffx=x-lastx; 			//check the difference between the current x and the last x position
    int diffy=y-lasty; 			//check the difference between the  current y and the last y position
    lastx=x; 					//set lastx to the current x position
    lasty=y; 					//set lasty to the current y position
    xrot += (float) diffy; 	   //set the xrot to xrot with the addition of the difference in the y position
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
	
	glClearColor(0.0f, 0.0f, 0.0f ,1.0f);
    glEnable( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

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

	object[0] += xpos1;
	object[2] += zpos1;

	eye[0] += xpos1;
	eye[2] += zpos1;

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

    /* Draw it to the screen */
    SDL_GL_SwapBuffers( );


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

	    if ( isActive )
		drawGLScene( );
	}

    Quit( 0 );
    return( 0 );
}
