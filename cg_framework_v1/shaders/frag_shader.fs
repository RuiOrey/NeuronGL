/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	This is a simple fragment shader.
	It paints all pixels white, represented by the vec4(1.0f, 1.0f, 1.0f, 1.0f) [RGBA]
*/

#version 410

out vec4 fragColor;

void main(void)
{
   fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
