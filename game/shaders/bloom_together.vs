//Vertex shader
 #version 110
 attribute vec4 InVertex;
 attribute vec2 InTexCoord0;
 attribute vec2 InTexCoord1;
 uniform mat4 ProjectionModelviewMatrix;
 varying vec2 TexCoord0;
 varying vec2 TexCoord1;    //Or just use TexCoord0
 //------------------------
 void main()
 {
   gl_Position = ProjectionModelviewMatrix * InVertex;
   TexCoord0 = InTexCoord0;
   TexCoord1 = InTexCoord1;
 }