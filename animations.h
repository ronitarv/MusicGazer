#pragma once

#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 1920; 
const unsigned int SCR_HEIGHT = 1080;

void star_pass(GLFWwindow* window, struct data* data);
void warp(GLFWwindow* window, struct data* data);
void transwarp(GLFWwindow* window, struct data* data);
void aurora_warp(GLFWwindow* window, struct data* data);