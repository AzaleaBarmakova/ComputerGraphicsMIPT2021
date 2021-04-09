#ifndef INITPROGRAM_HPP
#define INITPROGRAM_HPP

// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
#include "common/shader.hpp"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

int startInitialization(GLFWwindow*& window);

#endif 