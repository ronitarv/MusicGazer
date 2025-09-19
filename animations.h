#pragma once

#include <GLFW/glfw3.h>

extern unsigned int SCR_WIDTH; 
extern unsigned int SCR_HEIGHT;

void star_pass(GLFWwindow* window, struct data* data);
void warp(GLFWwindow* window, struct data* data);
void transwarp(GLFWwindow* window, struct data* data);
void aurora_warp(GLFWwindow* window, struct data* data);
void quantum_slipstream(GLFWwindow* window, struct data* data);