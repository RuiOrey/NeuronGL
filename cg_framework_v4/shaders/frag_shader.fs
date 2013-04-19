/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Verónica Orvalho, Bruno Oliveira, 2012

	This is a simple fragment shader.
*/

#version 330

out vec4 fragColor;
in vec2 texCoord_vf;
uniform sampler2D tex;

void main(void)
{
	
	fragColor = texture(tex, texCoord_vf);
}
