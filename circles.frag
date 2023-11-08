#version 330 core

/* Updated color */
out vec4 out_Color;

/* Uniform variable */
uniform vec4 circleColor;

void main(void)
{
    out_Color = circleColor;
}

