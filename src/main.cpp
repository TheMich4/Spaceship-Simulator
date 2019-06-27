#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // camera forward vector
glm::vec3 cameraSide; // camera up vector
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureAsteroid;
GLuint textureHit;

const int maxNumberOfPlanets = 100;
int numberOfPlanets = 0;

const float planetRadius = 40.0;

std::vector<glm::vec3> planetPositions;

int xPrev;
int yPrev;

float moveSpeed = 0.2f;

void renderScene();
void checkForHit();
void shoot();

void keyboard(unsigned char key, int x, int y) {

	float angleSpeed = 0.1f;
	switch(key) {
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	case 'e': 
		std::cout << "E!" << std::endl;
		shoot();
		break;
	}
}

void mouse(int x, int y) {
	float angleSpeed = 0.01f;

	int xDiff = xPrev - x;
	int yDiff = yPrev - y;

	cameraAngle += xDiff * angleSpeed;

	//cameraPos.y += yDiff * angleSpeed;

	xPrev = x;
	yPrev = y;
}

void generatePlanets() {
	while (numberOfPlanets < maxNumberOfPlanets) {
		glm::vec3 planet = glm::ballRand(planetRadius);
		
		if (planet == cameraPos) {
			continue;
		}
		
		if (planet.y < -5 || planet.y > 5) {
			continue;
		}

		planetPositions.push_back(planet);
		numberOfPlanets += 1;
	}
}

void checkForColision() {
	int size = 1.9;

	for (int i = 0; i < maxNumberOfPlanets; i++) {
		// check for colision on x axis
		if (cameraPos.x > planetPositions[i].x - size && cameraPos.x < planetPositions[i].x + size) {
			// check for colisions on y axis
			if (cameraPos.y > planetPositions[i].y - size && cameraPos.y < planetPositions[i].y + size) {
				// check for colisions on z axis
				if (cameraPos.z > planetPositions[i].z - size && cameraPos.z < planetPositions[i].z + size) {
					std::cout << "colision with " << i << std::endl;

					// have to figure out how to remove a planet form the screen
					// and how to calculate colission based on model position rather than camera position
				}
			}
		}
	}
}

glm::mat4 createCameraMatrix() {
	cameraDir = glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0.0f, sinf(cameraAngle - glm::radians(90.0f)));
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color) {
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId) {
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

// ignore this for now - WORK IN PROGRESS

void checkForHit() {
	//std::cout << "Angle: " << cameraAngle << ", Dir: " << cameraDir.x << ", x: " << cameraPos.x << ", y:" << cameraPos.y << std::endl";

	for (int i = 0; i < numberOfPlanets; i++) {
		//std::cout << i + 1 << ": " << planetPositions[i].x << ", " << planetPositions[i].y;

		// Can only shoot at the planet if it is at the same hight as a spaceship
		// Planet has a height of around 1.0 (have to check exact number)
		if (cameraPos.y > planetPositions[i].y - 1.0 && cameraPos.y < planetPositions[i].y + 1.0) {

			// Here find how to calculate if spaceship is facing the planet

			std::cout << "Hit: " << i << std::endl;

			glm::vec3 pos = planetPositions[i];
			pos.x += 1;
			pos.y += 1;

			drawObjectTexture(&sphereModel, glm::translate(pos), textureHit);
		}

		//std::cout << std::endl;
	}
}

void shoot() {
	checkForHit();
}

void renderScene() {
	std::cout << "renderScene called now!" << std::endl;

	// Update of camera and perspective matrices
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0,-0.25f,0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::rotate(-cameraAngle, glm::vec3(0,1,0)) * shipInitialTransformation;
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(1.0f));

	for (int i = 0; i < numberOfPlanets; i++) {
		drawObjectTexture(&sphereModel, glm::translate(planetPositions[i]), textureAsteroid);
	}

	// check if after spaceship is coliding with any planet
	checkForColision();

	glutSwapBuffers();
}

void init() {
	std::cout << "init called now!" << std::endl;

	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	textureAsteroid = Core::LoadTexture("textures/asteroid.png");
	textureHit = Core::LoadTexture("textures/asteroidHit.png");

	generatePlanets();
}

void shutdown() {
	std::cout << "shutdown called now!" << std::endl;

	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle() {
	//std::cout << "idle called now!" << std::endl;

	glutPostRedisplay();
}

int main(int argc, char ** argv) {
	std::cout << "main called now!" << std::endl;

	srand(time(0));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Spaceship Simulator");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
