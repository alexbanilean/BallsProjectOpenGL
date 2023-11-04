//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - circles.frag |
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

uniform vec4 circleColor;

void main(void)
{
    out_Color = circleColor;
}

