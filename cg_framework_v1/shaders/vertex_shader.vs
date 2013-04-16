/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	This is a simple vertex shader.
	It takes as input the ATTRIBUTE vertex (a vec4), and outputs it as the vertex's position.
*/

#version 410

in vec4 vertex;

void main()
{
   gl_Position = vertex;
}
