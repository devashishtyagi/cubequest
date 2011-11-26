//------------------------
//Fragment shader
#version 110
uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float BlendFactor;
//------------------------
varying vec2 TexCoord0;
varying vec2 TexCoord1;    //Or just use TexCoord0
//------------------------
void main()
 {
    vec4 texel0 = vec4(0.0,0.0,0.0,0.0); 
    vec4 texel1 = vec4(0.0,0.0,0.0,0.0); 
    gl_FragColor = mix(texel0, texel1, BlendFactor);
 }