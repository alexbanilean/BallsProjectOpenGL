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

// debug
#include <iostream>

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

// play around with these
// consider that if the maximal area of the objects is close to the are of the rectagle your computer will have a bad time
const int MAX_BOUNCES = 8;
const float DECREASE_AMOUNT = 1.6f;
const float MIN_BALL_SIZE = 5.0f;
// bounding box
const glm::vec2 BOX_MIN(-250, -250);
const glm::vec2 BOX_MAX(250, 250);

// initial config
const float INITIAL_SIZE = 100.0f;
const float INITIAL_VELOCITY = 5.0f;
const GLvoid* verticesSize;

std::vector<glm::vec4> possibleColors = {
	glm::vec4(0.2f, 0.48f, 0.3f, 1.0f),
	glm::vec4(0.53f, 0.29f, 0.17f, 1.0f),
	glm::vec4(0.35f, 0.51f, 0.13f, 1.0f),
	glm::vec4(0.35f, 0.19f, 0.45f, 1.0f),
	glm::vec4(0.16f, 0.34f, 0.48f, 1.0f),
	glm::vec4(0.12f, 0.39f, 0.47f, 1.0f),
	glm::vec4(0.48f, 0.07f, 0.44f, 1.0f),
};

class Ball {
public:
	static std::vector<Ball> balls;
	static std::vector<Ball> new_balls;

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

private:
	/* 
		Elastic collisions of randomized object are slighty impredictiable especially when mass based
		This computes the reflection of walls as well as pulling the ball back in frame in case it reached aa speed that allowed it to fully pass
	
	*/
	void WallCollisionCheck() {
		if (position.x - size < BOX_MIN.x) {
			velocity.x = -velocity.x;
			position.x = BOX_MIN.x + size;
			bounceCount++;
		}
		else if (position.x + size > BOX_MAX.x) {
			velocity.x = -velocity.x;
			position.x = BOX_MAX.x - size;
			bounceCount++;
		}

		if (position.y - size < BOX_MIN.y) {
			velocity.y = -velocity.y;
			position.y = BOX_MIN.y + size;
			bounceCount++;
		}
		else if (position.y + size > BOX_MAX.y) {
			velocity.y = -velocity.y;
			position.y = BOX_MAX.y - size;
			bounceCount++;
		}
	}

	void Split() {
		// this is a workaround so it dont have to create new objects and delete old ones
		if (size > MIN_BALL_SIZE) {
			size /= DECREASE_AMOUNT;
			Ball newBall = *this;

			float randomAngle = static_cast<float>(std::rand()) / RAND_MAX * 2 * PI;
			glm::vec2 dir(std::cos(randomAngle), std::sin(randomAngle));

			newBall.velocity = dir * glm::length(velocity);
			this->velocity = -dir * glm::length(velocity);

			color = possibleColors[rand() % possibleColors.size()];
			newBall.color = possibleColors[rand() % possibleColors.size()];

			float separationDistance = size;
			this->position += dir * separationDistance;
			newBall.position -= dir * separationDistance;

			this->bounceCount = 0;
			newBall.bounceCount = 0;

			balls.push_back(newBall);
		}
	}

	static void HandleCollisions() {
		for (size_t i = 0; i < balls.size(); ++i) {
			for (size_t j = i + 1; j < balls.size(); ++j) {
				Ball& ball1 = balls[i];
				Ball& ball2 = balls[j];

				glm::vec2 diff = ball2.position - ball1.position;
				float distance = glm::length(diff);
				float radiusSum = ball1.size + ball2.size;

				// Check for collision
				if (distance < radiusSum) {
					// Calculate elastic collision response
					ElasticCollision(ball1, ball2, diff, distance, radiusSum);
				}
			}
		}
	}


	// WARNING physics
	/* This functions calculates perfect elastic collision of 2 objects using conservation of kinetic energy and momentum*/
	static void ElasticCollision(Ball& ball1, Ball& ball2, const glm::vec2& diff, float distance,float radiusSum) {
		float mass1 = PI * ball1.size * ball1.size;
		float mass2 = PI * ball2.size * ball2.size;
		float totalMass = mass1 + mass2;

		glm::vec2 normal = glm::normalize(diff);
		glm::vec2 tangent(-normal.y, normal.x);

		float dotNormal1 = glm::dot(normal, ball1.velocity);
		float dotNormal2 = glm::dot(normal, ball2.velocity);
		float dotTangent1 = glm::dot(tangent, ball1.velocity);
		float dotTangent2 = glm::dot(tangent, ball2.velocity);

		glm::vec2 tangentVelocity1 = tangent * dotTangent1;
		glm::vec2 tangentVelocity2 = tangent * dotTangent2;

		float newDotNormal1 = (dotNormal1 * (mass1 - mass2) + 2 * mass2 * dotNormal2) / totalMass;
		float newDotNormal2 = (dotNormal2 * (mass2 - mass1) + 2 * mass1 * dotNormal1) / totalMass;

		glm::vec2 newNormalVelocity1 = normal * newDotNormal1;
		glm::vec2 newNormalVelocity2 = normal * newDotNormal2;

		ball1.velocity = newNormalVelocity1 + tangentVelocity1;
		ball2.velocity = newNormalVelocity2 + tangentVelocity2;

		// Move balls apart if they're overlapping to prevent sticking
		float overlap = (ball1.size + ball2.size) - distance;
		if (overlap > 0) {
			glm::vec2 separation = normal * overlap;
			ball1.position -= separation * (mass2 / totalMass);
			ball2.position += separation * (mass1 / totalMass);
		}

		ball1.bounceCount++;
		ball2.bounceCount++;
	}

	auto inline get_position() const
	{
		return glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
	}
	auto inline get_scale() const
	{
		return glm::scale(glm::mat4(1.0f), glm::vec3(size, size, 1.0f));
	}
public:

	void Update() {
		position += velocity; // move

		WallCollisionCheck();

		// Check for split condition
		if (bounceCount >= MAX_BOUNCES) {
			Split();
		}
	}

	auto inline transform() const
	{
		return get_position() * get_scale();
	}

	static inline void SceneUpdate()
	{
		HandleCollisions();
		const int cnt = balls.size();
		// Ensure we only call update on the balls we have, and dont recalculate for freshly spawend ones
		for (int i=0;i<cnt;i++)
			balls[i].Update();
	}
};

std::vector<Ball> Ball::balls;
std::vector<Ball> Ball::new_balls;

void CreateVBO(void)
{
	// varfurile 
	GLfloat Vertices[] = {
		BOX_MIN.x, BOX_MIN.y,  0.0f,  1.0f,
		BOX_MAX.x, BOX_MIN.y,  0.0f,  1.0f,
		BOX_MAX.x, BOX_MAX.y,  0.0f,  1.0f,
		BOX_MIN.x, BOX_MAX.y,  0.0f,  1.0f
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
	Ball ball;
	ball.position = glm::vec2(0.0f, 0.0f);

	float randomAngle = static_cast<float>(std::rand()) / RAND_MAX * 2 * PI;
	glm::vec2 dir(std::cos(randomAngle), std::sin(randomAngle));

	ball.velocity = dir * INITIAL_VELOCITY;
	ball.color = possibleColors[0];
	ball.size = INITIAL_SIZE;
	ball.bounceCount = 0;
	Ball::balls.push_back(ball);
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

	//std::cerr << "balls in scene: " << Ball::balls.size() << std::endl;
	/* Balls rendering */
	for (const Ball& ball : Ball::balls) {
		myMatrix = resizeMatrix * ball.transform();
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

	// Compute geometry for the next frame
	Ball::SceneUpdate();

	glutSwapBuffers();
	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

void TimerFunction(int value)
{
	// fix 60 fps since computation is done on every frame we want it to look similar on different speed machine and dont consume excesive CPU time
	glutPostRedisplay(); 
	glutTimerFunc(1000 / 60, TimerFunction, 0); 
}

int main(int argc, char* argv[])
{
	srand(static_cast<unsigned>(time(nullptr)));
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);					//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowSize(600, 600);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(400, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Balls project");								//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	glutTimerFunc(0, TimerFunction, 0); // Register the timer function
	Initialize();						//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);	//  Desenarea scenei in fereastra;
	glutCloseFunc(Cleanup);				//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	
}
