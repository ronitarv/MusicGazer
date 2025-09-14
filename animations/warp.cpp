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


void warp(GLFWwindow* window, data* data) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    Shader ourShader("shader.vs", "shader.fs");

    float last_intensity = 0.0f;

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    };

    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
    };

    int num_particles = 1000;
    Particle particles[num_particles];
    for (int i = 0; i < num_particles; i++) {
        particles[i].position = glm::vec3(0.0f, 0.0f, -100.0f);
        particles[i].velocity = glm::vec3(((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 50.0f));
    }

    glm::vec3 particleColor = glm::vec3(0.4f, 0.4f, 1.0f);
   
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 
    while (!glfwWindowShouldClose(window)) {
        pw_loop_iterate(data->pwloop, 0);
        //std::cout << "Intensity: " << intensity << std::endl;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ourShader.use();
        
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 100000.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glBindVertexArray(VAO);
        for (auto& p: particles) {
            p.position += p.velocity * 6.0f * std::pow(intensity, 2.0f) + glm::vec3(0.0f, 0.0f, 0.1f);
            if (p.position[0] < -1000.0f || p.position[0] > 1000.0f || p.position[1] < -1000.0f || p.position[1] > 1000.0f || p.position[2] > 0.0f) {
                p.position = glm::vec3(rand() % 200 - 100, rand() % 200 - 100, -1000.0f);
            }
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.position);
            ourShader.setVec3("particleColor",glm::vec3(0.2f, 0.2f, 0.2f) + particleColor * glm::vec3((intensity+last_intensity)/2,
                                                        (intensity+last_intensity)/2,
                                                        (intensity+last_intensity)/2));
            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        last_intensity = intensity;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}