#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
struct CookingState {
    float progress = 0.0f;
    float cookSpeed = 0.1f; 

    glm::vec3 startColor = glm::vec3(1.0f, 0.7f, 0.75f); // Roze (kao u generatePatty)
    glm::vec3 endColor = glm::vec3(0.35f, 0.16f, 0.14f);  // Tamno smedja

    void update(float deltaTime) {
        if (progress < 1.0f) {
            progress += cookSpeed * deltaTime;
        }
    }

    glm::vec3 getCurrentColor() {
        return glm::mix(startColor, endColor, progress);
    }
};