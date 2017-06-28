#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float camera_dist;
uniform float max_bounding_value;

varying vec3 finalcolor;
varying vec3 v_normal;
varying vec3 v_pos;

void main()
{
	mat4 VM_TI = transpose(inverse(view * model));
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;

	v_pos = vec3(view * model * vec4(position, 1.0f));
	v_normal = normalize(vec3(VM_TI * vec4(normal, 1.0f)));
	

	vec4 transformed_model = mat4(mat3(view)) * model * vec4(position, 1.0f); // mat3(x)=>clip rest data; mat4(..)=>set 1.0 to diagnol
	finalcolor = vec3(abs(transformed_model.z / max_bounding_value)/5*4 + 0.2);


	//finalcolor = vec3(position.z+0.8)/1.6;
	//finalcolor = vec3((camera_dist+transformed_model.z)/100); //w==1 *2

	//finalcolor = vec3(1.0f); 
	//finalcolor = vec3((gl_Position.y/gl_Position.w+1)/2);
}