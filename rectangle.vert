#version 330 core

/* Coordinates */
layout (location = 0) in vec4 in_Position; 
/* Colors */    
layout (location = 1) in vec4 in_Color;
/* Texture coordinates */
layout (location = 2) in vec2 texCoord;


/* Updated position */
out vec4 gl_Position;
/* Send to rectangle.frag */
out vec4 ex_Color;
out vec2 tex_Coord;

/* Uniform variable */
uniform mat4 myMatrix;

void main(void)
{
    gl_Position = myMatrix * in_Position;
    ex_Color = in_Color;
    tex_Coord = vec2(texCoord.x, 1-texCoord.y);
} 
 