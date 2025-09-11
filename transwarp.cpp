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
#include <random>


void transwarp(GLFWwindow* window, data* data) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    glBlendColor(0.0f, 1.0f, 0.0f, 0.1f);
    
    Shader ourShader("shader.vs", "transwarp.fs");

    float last_intensity = 0.0f;
    float tunnel_radius = 1000.0f;
    float tunnel_length = 10000.0f;

    std::random_device ran_dev;
    std::mt19937 gen(ran_dev());
    std::uniform_real_distribution<float> rand_dist(-5.0f,5.0f);
    std::uniform_real_distribution<float> rand_length(0.0f,tunnel_length);

    
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
        glm::vec3 color;
        bool isTunnel;
    };

    int num_particles = 5000;
    Particle particles[num_particles];
    for (int i = 0; i < num_particles; i++) {
        if (rand() % 2 == 0) {
            particles[i].position = glm::vec3(0.0f, 0.0f, -(rand_length(gen)));
            particles[i].velocity = glm::vec3(((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 50.0f));
            particles[i].isTunnel = false;
        } else {
            float angle = rand_dist(gen);
            float x = tunnel_radius * cos(angle);
            float y = tunnel_radius * sin(angle);
            particles[i].position = glm::vec3(x, y, -(rand_length(gen)));
            particles[i].velocity = glm::vec3(0.0f, 0.0f, ((rand() % 100) / 50.0f));
            particles[i].color = glm::vec3(0.4f, (rand() & 1000)/1000.0, 0.4f);
            particles[i].isTunnel = true;
        }
    }

    glm::vec3 particleColor = glm::vec3(0.0f, 1.0f, 0.0f);
   
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
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.0f, 10000000.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        //std::cout << "Intensity: " << intensity << std::endl;
        //std::cout << rand_dist(gen) << std::endl;
        glBindVertexArray(VAO);
        for (auto& p: particles) {
            if (p.isTunnel) {
                p.position += p.velocity * 50.0f * std::pow(intensity, 2.0f) + glm::vec3(0.0f, 0.0f, 0.1f);
                if (p.position[2] > 0.0f) {
                    float angle = rand_dist(gen);
                    float x = tunnel_radius * cos(angle);
                    float y = tunnel_radius * sin(angle);
                    p.position = glm::vec3(x, y, -tunnel_length);
                }
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, p.position);
                model = glm::scale(model, glm::vec3(500.0f));
                ourShader.setVec3("particleColor",p.color);
                ourShader.setFloat("alphaScale",intensity/5.0f);
                //ourShader.setVec3("particleColor",glm::vec3(((intensity-0.2f)+(last_intensity-0.2f))/2,
                //                                            ((intensity-0.2f)+(last_intensity-0.2f))/2,
                //                                            (intensity+last_intensity/2)));
                ourShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            else {
                p.position += p.velocity * 10.0f * std::pow(intensity, 2.0f) + glm::vec3(0.0f, 0.0f, 0.1f);
                if (p.position[0] < -1000.0f || p.position[0] > 1000.0f || p.position[1] < -1000.0f || p.position[1] > 1000.0f || p.position[2] > 0.0f) {
                    p.position = glm::vec3(rand() % 200 - 100, rand() % 200 - 100, -tunnel_length);
                }
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, p.position);
                //model = glm::scale(model, glm::vec3(1.0f));
                ourShader.setVec3("particleColor",glm::vec3(0.2f, 0.2f, 0.2f) + particleColor * glm::vec3((intensity+last_intensity)/2,
                                                            (intensity+last_intensity)/2,
                                                            (intensity+last_intensity)/2));
                ourShader.setFloat("alphaScale",1.0f);
                //ourShader.setVec3("particleColor",glm::vec3(((intensity-0.2f)+(last_intensity-0.2f))/2,
                //                                            ((intensity-0.2f)+(last_intensity-0.2f))/2,
                //                                            (intensity+last_intensity/2)));
                ourShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        last_intensity = intensity;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}