varying float V;                    // generic varying
varying float LightIntensity;

uniform float Frequency;            // Stripe frequency = 6

void main()
{
    float sawtooth = fract(V * Frequency);
    float triangle = abs(2.0 * sawtooth - 1.0);
    float dp = length(vec2(dFdx(V), dFdy(V)));
    float edge = dp * Frequency * 2.0;
    float square = smoothstep(0.5 - edge, 0.5 + edge, triangle);
    gl_FragColor = vec4(vec3(square), 1.0);
}
