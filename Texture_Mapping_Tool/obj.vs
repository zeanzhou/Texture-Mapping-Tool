#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 camera_pos;
uniform float camera_dist;
uniform float max_bounding_value;

varying vec3 Normal;
varying vec3 FragPos;
varying vec3 LightPos;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;

	FragPos = vec3(view * model * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse(view * model))) * normal;
	LightPos = vec3(view * vec4(camera_pos, 1.0));
}