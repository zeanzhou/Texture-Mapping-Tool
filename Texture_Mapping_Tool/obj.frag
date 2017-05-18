#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;

varying vec3 finalcolor;

void main()
{    
    color = vec4(finalcolor, 1.0f); //vec4(texture(texture_diffuse1, TexCoords));
}