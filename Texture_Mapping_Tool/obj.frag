#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;

varying vec3 finalcolor;
varying vec3 v_normal;
varying vec3 v_pos;

void main()
{
	//vec3 ka = vec3(0.1f, 0.1f, 0.1f);
	//vec3 kd = vec3(1.0f, 1.0f, 1.0f);

	//color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //color = vec4(finalcolor, 1.0f);
	//color = vec4(texture(texture_diffuse1, TexCoords));

	//vec3 lightDir = normalize(vec3(0.0, 0.0, 100000.0) - v_pos);
	//float NdotL = max(dot(v_normal, lightDir), 0.0);

	//color = vec4(ka + kd * NdotL, 1.0f);
	color = vec4(finalcolor, 1.0f);
}