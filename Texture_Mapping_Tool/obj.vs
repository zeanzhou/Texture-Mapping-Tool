#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float camera_dist;
varying vec3 finalcolor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;

	vec4 transformed_model = view * model * vec4(position, 1.0f);
	//finalcolor = vec3(position.z+0.8)/1.6;
	finalcolor = vec3((camera_dist+transformed_model.z+0.5)); //w==1
	//finalcolor = vec3((transformed_model.y+1)/2);
}