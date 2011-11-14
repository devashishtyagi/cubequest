uniform sampler2D texture;
#define glaresize 0.03 
#define power 0.4

void main()
{
   vec4 sum = vec4(0);
   vec4 bum = vec4(0);
   vec2 texcoord = vec2(gl_TexCoord[0]);
   int j;
   int i;

   for( i= -3 ;i <3 ; i++)
   {
        for (j = -4; j < 4; j++)
        {
            sum += texture2D(texture, texcoord + vec2(-i, j)*glaresize) * power;          
        }
   }
    	if (texture2D(texture, texcoord).r < 2.0)
    {
       gl_FragColor = sum*sum*sum*0.0080*2.0+ texture2D(texture, texcoord);
    }
}
