#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;

varying vec3 Normal;
varying vec3 FragPos;
varying vec3 LightPos;
void main()
{
	vec3 lightColor = vec3(1.0);

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	color = vec4(ambient + diffuse, 1.0f);
}