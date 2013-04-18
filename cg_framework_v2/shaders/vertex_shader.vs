/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	The directive layout(location=?) "hard-codes" the location of the attributes
	
*/

#version 330

layout (location=0) in vec4 vertex;
layout (location=1) in vec4 color;
out vec4 color_vf;

uniform mat4 pMatrix;
uniform mat4 vMatrix;

void main()
{
	color_vf = color;
	//gl_Position = vertex;
	//gl_Position = pMatrix * vertex;
	gl_Position = pMatrix * vMatrix * vertex;
}

