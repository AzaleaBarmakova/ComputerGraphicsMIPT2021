// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/initProgram.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>


class LoadModel {
public:
	GLuint Texture;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	LoadModel(const std::string& obj_file, const std::string& texture_file) {
		Texture = loadBMP_custom(texture_file.data());
		loadOBJ(obj_file.data(), vertices, uvs, normals);
	}

	~LoadModel() {
		glDeleteTextures(1, &Texture);
	}

	void Draw(GLuint TextureID, GLuint vertexbuffer, GLuint uvbuffer) {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
};

int main(void)
{
	startInitialization(window);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture and .obj file
	LoadModel* enemy = new LoadModel("UFO_Empty.obj", "ufo_spec.bmp");
	//LoadModel* bullet = new LoadModel("bullet.obj", "rivetsGold.bmp");
	LoadModel* bullet = new LoadModel("bullet.obj", "water.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Load it into a VBO
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, enemy->vertices.size() * sizeof(glm::vec3), &enemy->vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, enemy->uvs.size() * sizeof(glm::vec2), &enemy->uvs[0], GL_STATIC_DRAW);

	GLuint vertexbufferBullet;
	glGenBuffers(1, &vertexbufferBullet);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferBullet);
	glBufferData(GL_ARRAY_BUFFER, bullet->vertices.size() * sizeof(glm::vec3), &bullet->vertices[0], GL_STATIC_DRAW);

	GLuint uvbufferBullet;
	glGenBuffers(1, &uvbufferBullet);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferBullet);
	glBufferData(GL_ARRAY_BUFFER, bullet->uvs.size() * sizeof(glm::vec2), &bullet->uvs[0], GL_STATIC_DRAW);

	std::vector<std::vector<float>> enemyCoordinates;
	int enemyCount = 0;
	int enemyMax = 11;
	const float enemySpawnRadius = 15.0f;
	double RespawnTime = 3;
	double spawnTime = glfwGetTime();

	std::vector<std::vector<glm::vec3>> bulletCoordinates;
	float bulletSpeed = 30.0f;

	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//It's about time = we want enemies to spawn evenly
		// glfwGetTime is called only once, the first time this function is called
		static double lastTime = glfwGetTime();
		// Compute time difference between current and last frame
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		if (currentTime - spawnTime > RespawnTime && enemyCount < enemyMax) {
			std::vector<float> newEnemy = std::vector<float>({
				(float)std::rand() / RAND_MAX * 2 * enemySpawnRadius - enemySpawnRadius,
				(float)std::rand() / RAND_MAX * 2 * enemySpawnRadius - enemySpawnRadius,
				(float)std::rand() / RAND_MAX * 2 * enemySpawnRadius - enemySpawnRadius,
				(float)std::rand() / RAND_MAX,
				(float)std::rand() / RAND_MAX,
				(float)std::rand() / RAND_MAX,
				((float)std::rand() / RAND_MAX) * 360.0f,
				});
			enemyCoordinates.push_back(newEnemy);
			enemyCount++;
			spawnTime = currentTime;
		}

		for (auto & enemyCoordinate : enemyCoordinates) {
			ModelMatrix = glm::mat4(1.0);

			glm::vec3 myRotationAxis(enemyCoordinate[3], enemyCoordinate[4], enemyCoordinate[5]);
			ModelMatrix = glm::rotate(enemyCoordinate[6] + (float)currentTime, myRotationAxis) * ModelMatrix;

			glm::vec3 myTranslationVector(enemyCoordinate[0], enemyCoordinate[1], enemyCoordinate[2]);
			ModelMatrix = glm::translate(glm::mat4(), myTranslationVector) * ModelMatrix;
			
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			enemy->Draw(TextureID, vertexbuffer, uvbuffer);
		}

		//Create bullet if mouse was released
		static int oldState = GLFW_RELEASE;
		int newState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if (newState == GLFW_RELEASE && oldState == GLFW_PRESS) {
			std::vector<glm::vec3> newBullet = std::vector<glm::vec3>({
				getPosition() + getDirection() * 5.0f,
				getDirection()
				});
			bulletCoordinates.push_back(newBullet);
		}
		oldState = newState;

		//Draw bullet
		for (auto & bulletCoordinate : bulletCoordinates) {
			ModelMatrix = glm::mat4(0.7f);

			glm::vec3 myTranslationVector(bulletCoordinate[0]);
			ModelMatrix = glm::translate(glm::mat4(), myTranslationVector) * ModelMatrix;
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			bullet->Draw(TextureID, vertexbufferBullet, uvbufferBullet);

			//Update bullet coordinates
			bulletCoordinate[0] += bulletCoordinate[1] * bulletSpeed * deltaTime;
		}
		
		
		for (size_t i = 0; i < enemyCoordinates.size(); i++) {
			glm::vec3 enemyPosition = glm::vec3(enemyCoordinates[i][0], enemyCoordinates[i][1], enemyCoordinates[i][2]);
			for (size_t j = 0; j < bulletCoordinates.size(); j++) {
				if (glm::distance(enemyPosition, bulletCoordinates[j][0]) < 3.0f) {
					enemyCoordinates.erase(enemyCoordinates.begin() + i);
					bulletCoordinates.erase(bulletCoordinates.begin() + j);
					enemyCount--;
				}
			}
		}

		// For the next frame, the "last time" will be "now"
		lastTime = currentTime;

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

