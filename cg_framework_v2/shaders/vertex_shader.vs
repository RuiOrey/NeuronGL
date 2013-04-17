/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	The directive layout(location=?) "hard-codes" the location of the attributes
	
*/

#version 330

layout (location=0) in vec4 vertex;
layout (location=1) in vec4 color;
out vec4 color_vf;

//1.uniform mat4 pMatrix;
//2.uniform mat4 vMatrix;

void main()
{
	color_vf = color;
	gl_Position = vertex;
	//3.gl_Position = pMatrix * vertex;
	//4.gl_Position = pMatrix * vMatrix * vertex;
}

