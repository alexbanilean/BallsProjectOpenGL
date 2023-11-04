//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - circles.vert |
// ======================================
// 
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei; 
//

#version 330 core

//  Variabile de intrare (dinspre programul principal);
layout (location = 0) in vec4 in_Position;     //  Se preia din buffer de pe prima pozitie (0) atributul care contine coordonatele;

//  Variabile de iesire;
out vec4 gl_Position;   //  Transmite pozitia actualizata spre programul principal;

//  Variabile uniforme;
uniform mat4 myMatrix;

void main(void)
{
    gl_Position = myMatrix * in_Position;
} 
 