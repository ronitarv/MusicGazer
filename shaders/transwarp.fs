#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform vec3 particleColor;
uniform float alphaScale;

void main()
{
    vec2 uv = TexCoord - vec2(0.5);
    float dist = length(uv);
    if (dist > 0.5) {
        discard;
    }
    float coreRadius = 0.5;
    float alpha = 1.0 - smoothstep(0.1, coreRadius, dist);
    
    FragColor = vec4(particleColor, alpha * alphaScale);
}