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
#include <chrono>
#include <complex>
#include <numbers>
#define _USE_MATH_DEFINES
#include<cmath>

using namespace std::complex_literals;

void quantum_slipstream(GLFWwindow* window, data* data) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    
    Shader ourShader("shader.vs", "transwarp.fs");

    float last_intensity = 0.0f;
    float tunnel_radius = 500.0f;
    float tunnel_length = 5000.0f;
    float norm_intensity = 0.0f;

    std::random_device ran_dev;
    std::mt19937 gen(ran_dev());
    std::uniform_real_distribution<float> rand_dist(-50.0f,50.0f);
    std::uniform_real_distribution<float> rand_circle_angle(0.0f,359.0f);
    std::uniform_real_distribution<float> rand_length(0.0f,tunnel_length);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f 
    };

    unsigned int indices[] = {
        0, 1, 2, 
        2, 3, 0 
    };

    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 color;
        bool isTunnel;
    };

    int num_particles = 20000;
    Particle particles[num_particles];
    for (int i = 0; i < num_particles; i++) {
        if (rand() % 16 == 0) {
            particles[i].position = glm::vec3(0.0f, 0.0f, -(rand_length(gen)));
            particles[i].velocity = glm::vec3(((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 5000.0f - 0.01f), ((rand() % 100) / 50.0f));
            particles[i].color = glm::vec3(0.1f, 0.7f, 0.8f);
            particles[i].isTunnel = false;
        } else {
            float angle = rand_dist(gen);
            float x = tunnel_radius * cos(angle);
            float y = tunnel_radius * sin(angle);
            particles[i].position = glm::vec3(x, y, -(std::pow(rand_length(gen)/tunnel_length,10.0f)*tunnel_length));
            particles[i].velocity = glm::vec3(0.0f, 0.0f, 5.0f);
            float darkness = (rand() % 1000)/1000.0f;
            particles[i].color = glm::vec3(0.2f*darkness, 0.7f*darkness, 0.8f*darkness);
            particles[i].isTunnel = true;
        }
    }

    glm::vec3 particleColor = glm::vec3(0.8f, 0.8f, 1.0f);

    const std::complex<double> I(0.0, 1.0);
    double r = 0.99;

    int num_complex = 360;
    std::complex<double> complex_z[num_complex];
    for (int i = 0; i < num_complex; ++i) {
        complex_z[i] = std::exp(I * ((static_cast<double>(i)/num_complex)*2.0f*M_PI)); //std::complex<double>(0.99, (i/1000.0f)*2*M_PI);
    }

    int num_wi = 8;
    std::vector<double> complex_wi;
    for (int i = 0; i < num_wi; ++i) {
        complex_wi.push_back((static_cast<double>(i)/num_wi)*2.0f*M_PI);
    }

    std::vector<std::complex<double>> complex_zi;
    complex_zi.reserve(complex_wi.size());
    for (int i = 0; i < num_wi; ++i) {
        complex_zi.push_back(r * std::exp(I * complex_wi[i]));
    }

    double complex_spec[num_complex];

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);
    auto start_norm = std::chrono::high_resolution_clock::now();
    float max_intensity = 0.1f;
    float peak_intensity = 0.1f;
    while (!glfwWindowShouldClose(window)) {

        for (auto& wi : complex_wi) {
            float angle = rand_dist(gen)/50.0f * 0.05f*norm_intensity;
            /*for (auto& wj : complex_wi) {
                if (std::abs(wi - wj) < 5.0f && &wi != &wj) {
                    angle += (wi > wj) ? 0.05f : -0.05f;
                }
            }*/
            wi += angle;
        }

        for (int i = 0; i < num_wi; ++i) {
            complex_zi[i] = r * std::exp(I * complex_wi[i]);
        }
        
        for (int i = 0; i < num_complex; ++i) {
            complex_spec[i] = std::abs(complex_z[i] - complex_zi[0]);
            for (auto j = 1; j < num_wi; ++j) {
                complex_spec[i] *= std::abs(complex_z[i]-complex_zi[j]); 
            }
            complex_spec[i] = 1.0/complex_spec[i];
        }
        
        if (intensity > peak_intensity) {
            peak_intensity = std::max(intensity,0.01f);
            max_intensity = std::max(peak_intensity,max_intensity);
        }

        auto stop_norm = std::chrono::high_resolution_clock::now();
        auto duration_norm = std::chrono::duration_cast<std::chrono::seconds>(stop_norm - start_norm);

        if (duration_norm.count() > 1) {
            max_intensity += (peak_intensity-max_intensity)/300;
            peak_intensity = 0.1f;
            start_norm = std::chrono::high_resolution_clock::now();
        }
        
        norm_intensity = intensity / max_intensity;

        pw_loop_iterate(data->pwloop, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ourShader.use();
        
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.0f, 10000000.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glBindVertexArray(VAO);

        for (auto& p: particles) {
            if (p.isTunnel) {
                p.position += p.velocity * 100.0f * std::pow((norm_intensity+9*last_intensity)/10.0f, 5.0f) + glm::vec3(0.0f, 0.0f, 0.1f);
                if (p.position[2] > 0.0f) {

                    int angle = rand_circle_angle(gen);
                    
                    float x = tunnel_radius * cos(angle*M_PI/180.0f);
                    float y = tunnel_radius * sin(angle*M_PI/180.0f); 
                    p.position = glm::vec3(x, y, -(std::pow(rand_length(gen)/tunnel_length,10.0f)*tunnel_length));
                    
                    float angle_factor = complex_spec[angle] / 60.0f; 

                    p.color = glm::vec3(0.0f, 
                        0.7f*(angle_factor+0.006f/*+complex_spec[static_cast<int>((std::abs(p.position[2])/tunnel_length)*360)]/10000.0f*/), 
                        0.8f*(angle_factor+0.006f/*+complex_spec[static_cast<int>((std::abs(p.position[2])/tunnel_length)*360)]/10000.0f*/));
                }
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, p.position);
                model = glm::scale(model, glm::vec3(200.0f));
                ourShader.setFloat("alphaScale",0.02f + std::pow((norm_intensity+9*last_intensity)/10.0f,3.0f)/1.0f);
                ourShader.setVec3("particleColor",p.color);
                ourShader.setMat4("model", model);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            else {
                p.position += p.velocity * 50.0f * std::pow(norm_intensity, 2.0f) + glm::vec3(0.0f, 0.0f, 0.1f);
                if (p.position[0] < -1000.0f || p.position[0] > 1000.0f || p.position[1] < -1000.0f || p.position[1] > 1000.0f || p.position[2] > 0.0f) {
                    p.position = glm::vec3(rand() % 200 - 100, rand() % 200 - 100, -tunnel_length);
                }
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, p.position);
                ourShader.setVec3("particleColor",glm::vec3(0.2f, 0.2f, 0.2f) + particleColor * glm::vec3((norm_intensity+last_intensity)/2,
                                                            (norm_intensity+last_intensity)/2,
                                                            (norm_intensity+last_intensity)/2));
                model = glm::scale(model, glm::vec3(0.6f));
                ourShader.setFloat("alphaScale",2.0f);
                ourShader.setMat4("model", model);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        last_intensity = (norm_intensity+9*last_intensity)/10;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}