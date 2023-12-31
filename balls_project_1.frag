//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_02_Shader.frag |
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

uniform vec4 circleColor;
uniform int codCol;

void main(void)
{
    	switch(codCol)
	{
		case 0:
			out_Color = ex_Color;
			break;
		case 1:
			out_Color = circleColor;
			break;
		default:
			break;
	}
}

