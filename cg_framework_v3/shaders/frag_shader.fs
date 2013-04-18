/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	This is a simple fragment shader.
*/

#version 330

out vec4 fragColor;
in vec4 color_vf;

void main(void)
{
   fragColor = color_vf;
}
