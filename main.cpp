#include <vector>
#include <time.h>
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint
VaoId,
VboId,
EboId,
DboId,
ColorBufferId,
ProgramId,
ProgramId2,
myMatrixLocation,
colorLocation;

glm::mat4
myMatrix, resizeMatrix;

float xMin = -300.f, xMax = 300.f, yMin = -300.f, yMax = 300.f;

const float PI = 3.1415926;
const GLvoid* verticesSize;

struct Ball {
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;
	float size;
	int bounceCount;

	bool operator==(const Ball& other) const {
		return position == other.position &&
			velocity == other.velocity &&
			color == other.color &&
			size == other.size &&
			bounceCount == other.bounceCount;
	}
};

std::vector<Ball> balls;

std::vector<glm::vec4> possibleColors = {
	glm::vec4(0.2f, 0.48f, 0.3f, 1.0f),
	glm::vec4(0.53f, 0.29f, 0.17f, 1.0f),
	glm::vec4(0.35f, 0.51f, 0.13f, 1.0f),
	glm::vec4(0.35f, 0.19f, 0.45f, 1.0f),
	glm::vec4(0.16f, 0.34f, 0.48f, 1.0f),
	glm::vec4(0.12f, 0.39f, 0.47f, 1.0f),
	glm::vec4(0.48f, 0.07f, 0.44f, 1.0f),
};

void SplitBall(Ball& ball) {
	if (ball.size > 5.0f) {
		Ball newBall1 = ball;
		Ball newBall2 = ball;

		newBall1.size /= 2;
		newBall2.size /= 2;

		float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
		newBall1.velocity = glm::vec2(cos(glm::radians(randomAngle)), sin(glm::radians(randomAngle))) * 10.0f;
		newBall2.velocity = glm::vec2(cos(glm::radians(randomAngle + 180.0f)), sin(glm::radians(randomAngle + 180.0f))) * 10.0f;

		newBall1.color = possibleColors[rand() % possibleColors.size()];
		newBall2.color = possibleColors[rand() % possibleColors.size()];

		newBall1.position = glm::vec2(ball.position.x + 50, ball.position.y + 50);
		newBall2.position = glm::vec2(ball.position.x - 50, ball.position.y - 50);

		// Erase the original ball from the vector
		balls.erase(std::remove(balls.begin(), balls.end(), ball), balls.end());

		balls.push_back(newBall1);
		balls.push_back(newBall2);
	}
}

void CreateVBO(void)
{
	// varfurile 
	GLfloat Vertices[] = {
		-250.0f, -250.0f,  0.0f,  1.0f,
		 250.0f, -250.0f,  0.0f,  1.0f,
		 250.0f,  250.0f,  0.0f,  1.0f,
		-250.0f,  250.0f,  0.0f,  1.0f
	};

	// culorile, ca atribute ale varfurilor
	GLfloat Colors[] = {
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	//	Indicii care determina ordinea de parcurgere a varfurilor;
	static const GLuint Indices[] =
	{
		0, 1, 2, 3
	};

	std::vector<GLfloat> CircleVertices;
	for (int ii = 0; ii < 180; ii++) {
		float theta = 2.0f * PI * float(ii) / float(180);
		float x = cosf(theta);
		float y = sinf(theta);
		CircleVertices.push_back(x);
		CircleVertices.push_back(y);
	}

	//  Transmiterea datelor prin buffere;

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer comun pentru VARFURI - COORDONATE si CULORI;
	glGenBuffers(1, &VboId);																//  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId);													//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors) + sizeof(Vertices), NULL, GL_STATIC_DRAW);	//	Definirea bufferului, dimensiunea lui = dimensiunea(COLORS + VERTICES)
	//	Spatiul bufferului este impartit intre zona de COORDONATE si cea de VARFURI:
	//	 - prima sectiune incepe de la 0, are dimensiunea VERTICES si continele datele despre COORDONATE;
	//	 - a doua sectiune incepe de la sizeof(Vertices) - deci la finalul primei sectiuni, are dimensiunea COLORS si contine datele despre CULOARE;
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				//	COORDONATELE;
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);		//	CULORILE;

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboId);														//  Generarea bufferului si indexarea acestuia catre variabila EboId;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// buffer pentru CIRCLE VERTICES;
	glGenBuffers(1, &DboId);
	glBindBuffer(GL_ARRAY_BUFFER, DboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * CircleVertices.size(), &CircleVertices[0], GL_STATIC_DRAW);

	verticesSize = (const GLvoid*)sizeof(Vertices);
}

void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI(Coordonate + Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glDeleteBuffers(1, &DboId);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("balls_project_1.vert", "balls_project_1.frag");
	ProgramId2 = LoadShaders("circles.vert", "circles.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
	glDeleteProgram(ProgramId2);
}

void Initialize(void)
{
	glClearColor(0.26f, 0.32f, 0.41f, 1.0f); // culoarea de fond a ecranului
	CreateVBO();
	CreateShaders();

	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

	//	Dreptunghiul "decupat"; 
	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	myMatrix = resizeMatrix;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

	//	Se activeaza lucrul cu atribute;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, verticesSize);

	//	Desenarea primitivelor
	//	Functia glDrawElements primeste 4 argumente:
	//	 - arg1 = modul de desenare;
	//	 - arg2 = numarul de varfuri;
	//	 - arg3 = tipul de date al indicilor;
	//	 - arg4 = pointer spre indici (EBO): pozitia de start a indicilor;
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void*)(0));

	glUseProgram(ProgramId2);

	myMatrixLocation = glGetUniformLocation(ProgramId2, "myMatrix");
	colorLocation = glGetUniformLocation(ProgramId2, "circleColor");

	Ball ball;
	ball.position = glm::vec2(0.0f, 0.0f);
	ball.velocity = glm::vec2(0.0f, 0.0f);
	ball.color = possibleColors[0];
	ball.size = 10.0f;
	ball.bounceCount = 0;

	balls.push_back(ball);
	SplitBall(ball);

	for (const Ball& ball : balls) {
		myMatrix = resizeMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(ball.position.x, ball.position.y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(ball.size, ball.size, 1.0f));
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

		// Set the ball color
		glUniform4f(colorLocation, ball.color[0], ball.color[1], ball.color[2], ball.color[3]);

		// Bind the VBO for the circle
		glBindBuffer(GL_ARRAY_BUFFER, DboId);

		// Set the attribute pointers for the circle
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		// Draw the circle using the vertex buffer
		glDrawArrays(GL_TRIANGLE_FAN, 0, 180);
	}

	glUseProgram(ProgramId);
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

int main(int argc, char* argv[])
{
	srand(static_cast<unsigned>(time(nullptr)));
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowSize(600, 600);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(400, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Balls project");								//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);	//  Desenarea scenei in fereastra;
	glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();
}
