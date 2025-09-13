#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
extern "C" {
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
}
#include "audio_capture.h"
#include "animations.h"

unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}



int main(int argc, char* argv[]) {

    std::string source = "";
    std::string animation = "transwarp";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            std::cout << "Usage: ./music_gazer [--source <audio-device-id> --animation star_pass|warp|transwarp|aurora_warp]\n";
            std::cout << "--source <audio_device-id>  : ID to PipeWire Interface Node\n";
            std::cout << "--animation NAME : Animation to use [default: transwarp] [possible values: star_pass, warp, transwarp, aurora_warp]\n";
            return 0;
        } else if (arg == "--source") {
            source = argv[i+1];
            ++i;
        } else if (arg == "--animation") {
            animation = argv[i+1];
            ++i;
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            return 1;
        }
    }

    if (source != "") {
        std::cout << "Using audio source: " << source << std::endl;
    } else {
        std::cout << "No audio source specified, using default." << std::endl;
    }

    data* data = init_pipewire(argc, argv, source);

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        fprintf(stdout, "Error in glfwinit\n");
    }
    
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;  

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "MusicGazer", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        fprintf(stdout, "Error in window start\n");
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    if (animation == "star_pass") {
        star_pass(window, data);
    } else if (animation == "warp") {
        warp(window, data);
    } else if (animation == "transwarp") {
        transwarp(window, data);
    } else if (animation == "aurora_warp") {
        aurora_warp(window, data);
    } else {
        std::cerr << "Unknown animation: " << animation << std::endl;
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}