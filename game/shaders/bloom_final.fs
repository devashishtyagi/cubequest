uniform sampler2D texture;
#define glaresize 0.03 
#define power 0.3 

void main()
{
   vec4 sum = vec4(0);
   vec4 bum = vec4(0);
   vec2 texcoord = vec2(gl_TexCoord[0]);
   int j;
   int i;
	
	if (texcoord.x >= 0.8 || texcoord.y >= 0.8 || texcoord.x <= 0.2 || texcoord.y <= 0.2){
   	for( i= -4 ;i < 4 ; i++)
   	{
    	    for (j = -3; j < 3; j++)
        	{
            	sum += texture2D(texture, texcoord + vec2(-i, j)*glaresize) * power;
            	bum += texture2D(texture, texcoord + vec2(j, i)*glaresize) * power;            
        	}
   	}
    if (texture2D(texture, texcoord).r < 2.0)
    {
       gl_FragColor = sum*sum*sum*0.0080+bum*bum*bum*0.0080+ texture2D(texture, texcoord);
    }
    }
}
