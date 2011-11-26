#version 330

smooth out vec2 vertOutTexCoords;

uniform mat4 modelviewProj;

void main(void)
{
	vertOutTexture = gl_MultiTexCoord0;
    gl_Position = modelviewProj * gl_Vertex;
}
