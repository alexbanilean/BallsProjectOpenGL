#version 330 core

/* Coordinates from button */
layout (location = 0) in vec4 in_Position;

/* Updated position */
out vec4 gl_Position;

/* Uniform variable */
uniform mat4 myMatrix;

void main(void)
{
    gl_Position = myMatrix * in_Position;
} 
 