#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec3 inNormal;

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

out vec4 channelCol;
out vec2 channelTex;
out vec3 FragPos;
out vec3 Normal;


void main()
{
	gl_Position = uP * uV * uM * vec4(inPos, 1.0); //Zbog nekomutativnosti mnozenja matrica, moramo mnoziti MVP matrice i tjemena "unazad"
	FragPos = vec3(uM * vec4(inPos, 1.0));
	Normal = mat3(transpose(inverse(uM))) * inNormal;

	if (inTex != vec2(0.0)) {
		channelCol = vec4(1.0);
	} else {
		channelCol = inCol;
	}

	channelTex = inTex;
}