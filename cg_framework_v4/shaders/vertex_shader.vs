/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	The directive layout(location=?) "hard-codes" the location of the attributes
	
*/

#version 330

layout (location=0) in vec4 vertex;
layout (location=1) in vec2 texCoord;
out vec2 texCoord_vf;

uniform mat4 pMatrix;
uniform mat4 vMatrix;
uniform mat4 mMatrix;

void main()
{
	texCoord_vf = texCoord;
	gl_Position = pMatrix * vMatrix * mMatrix * vertex;
}

