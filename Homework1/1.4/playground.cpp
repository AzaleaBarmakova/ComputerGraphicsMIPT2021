#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Homework 1", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;  // Needed for core profile - don't work without this line
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// celeste (= pretty sky blue) background 
	glClearColor(0.6f, 0.93f, 1.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint ProgramID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// Camera matrix - initial
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, -3), // Camera is at (0,0,5), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	
	//triangular pyramid = tetrahedron, vertices (+-1, 0, -1/sqrt(2)), (0, +-1, 1/sqrt(2))
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0, 0.0, -1/sqrt(2),  // triangle 1 : begin
		1.0, 0.0, -1/sqrt(2),
		0.0,  -1.0, 1/sqrt(2), // triangle 1 : end

		-1.0, 0.0, -1 / sqrt(2), // triangle 2 : begin
		1.0, 0.0, -1 / sqrt(2),
		0.0,  1.0, 1 / sqrt(2), // triangle 2 : end

		-1.0, 0.0, -1 / sqrt(2), // triangle 3 : begin
		0.0,  1.0, 1 / sqrt(2),
		0.0,  -1.0, 1 / sqrt(2), // triangle 3 : end

		1.0, 0.0, -1 / sqrt(2), // triangle 4 : begin
		0.0,  1.0, 1 / sqrt(2),
		0.0,  -1.0, 1 / sqrt(2), // triangle 5 : end
	};

	//// One color for each vertex. 
	static const GLfloat g_color_buffer_data[] = {
		1.0f, 0.0f, 0.0f,  // triangle 1 : begin
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, // triangle 1 : end

		0.0f, 1.0f, 0.0f, // triangle 2 : begin
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, // triangle 2 : end

		0.0f, 0.0f, 1.0f, // triangle 3 : begin
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, // triangle 3 : end

		1.0f, 0.0f, 0.0f, // triangle 4 : begin
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, // triangle 4 : end
	};

	//starting parameters for circle movement
	double rad = 0.0;
	double pi = 3.1416;
	double radius = 4.0f;

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	do{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Use our shader
		glUseProgram(ProgramID);

		//Updating movement
		rad += pi / 360;
		
		// Camera matrix update
		glm::mat4 View = glm::lookAt(
			glm::vec3(radius * cos(rad), radius * cos(rad), radius * sin(rad)), 
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 1)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		
		// Our ModelViewProjection : multiplication of our 3 matrices 
		glm::mat4 MVP = Projection * View * Model; //  Remember, matrix multiplication is the other way around

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3*4); // 3 indices starting at 0 -> 1 triangle
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(ProgramID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

