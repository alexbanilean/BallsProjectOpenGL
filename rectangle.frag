#version 330 core

in vec4 ex_Color;
in vec2 tex_Coord;

/* Updated color */
out vec4 out_Color;

/* Uniform variables */
uniform vec4 circleColor;
uniform sampler2D backgroundTexture;
uniform int codCol;

void main(void)
{
    switch(codCol)
	{
		case 0:
			out_Color = ex_Color;
			break;
		case 1:
			out_Color = texture(backgroundTexture, tex_Coord);
			break;
		default:
			break;
	}
}

