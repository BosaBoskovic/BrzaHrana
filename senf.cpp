#include "senf.h"
#include <cmath>

void generateSenf(float* senfVertices, int segments) {
    float radius = 0.08f;
    float cylinderHeight = 0.15f;
    float coneHeight = 0.08f;
    int idx = 0;

    // ŽUTA BOJA
    float r = 1.0f, g = 1.0f, b = 0.0f, a = 1.0f;

    // ========== VALJAK (TELO) ==========
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Gornja tacka
        senfVertices[idx++] = x;
        senfVertices[idx++] = cylinderHeight;
        senfVertices[idx++] = z;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = x; senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = z;

        // Donja tacka
        senfVertices[idx++] = x;
        senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = z;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = x; senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = z;
    }

    // Gornji poklopac
    senfVertices[idx++] = 0.0f;
    senfVertices[idx++] = cylinderHeight;
    senfVertices[idx++] = 0.0f;
    senfVertices[idx++] = r; senfVertices[idx++] = g;
    senfVertices[idx++] = b; senfVertices[idx++] = a;
    senfVertices[idx++] = 0.0f; senfVertices[idx++] = 1.0f;
    senfVertices[idx++] = 0.0f;

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        senfVertices[idx++] = cos(angle) * radius;
        senfVertices[idx++] = cylinderHeight;
        senfVertices[idx++] = sin(angle) * radius;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = 0.0f; senfVertices[idx++] = 1.0f;
        senfVertices[idx++] = 0.0f;
    }

    // Donji poklopac
    senfVertices[idx++] = 0.0f;
    senfVertices[idx++] = 0.0f;
    senfVertices[idx++] = 0.0f;
    senfVertices[idx++] = r; senfVertices[idx++] = g;
    senfVertices[idx++] = b; senfVertices[idx++] = a;
    senfVertices[idx++] = 0.0f; senfVertices[idx++] = -1.0f;
    senfVertices[idx++] = 0.0f;

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        senfVertices[idx++] = cos(angle) * radius;
        senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = sin(angle) * radius;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = 0.0f; senfVertices[idx++] = -1.0f;
        senfVertices[idx++] = 0.0f;
    }

    // ========== KUPA (GORE) ==========
    float tipX = 0.0f;
    float tipY = cylinderHeight + coneHeight;
    float tipZ = 0.0f;

    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * 3.14159f;
        float angle2 = (float)(i + 1) / segments * 2.0f * 3.14159f;

        float x1 = cos(angle1) * radius;
        float z1 = sin(angle1) * radius;
        float x2 = cos(angle2) * radius;
        float z2 = sin(angle2) * radius;

        // Vrh kupe
        senfVertices[idx++] = tipX;
        senfVertices[idx++] = tipY;
        senfVertices[idx++] = tipZ;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = tipX; senfVertices[idx++] = 1.0f;
        senfVertices[idx++] = tipZ;

        // Baza 1
        senfVertices[idx++] = x1;
        senfVertices[idx++] = cylinderHeight;
        senfVertices[idx++] = z1;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = x1; senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = z1;

        // Baza 2
        senfVertices[idx++] = x2;
        senfVertices[idx++] = cylinderHeight;
        senfVertices[idx++] = z2;
        senfVertices[idx++] = r; senfVertices[idx++] = g;
        senfVertices[idx++] = b; senfVertices[idx++] = a;
        senfVertices[idx++] = x2; senfVertices[idx++] = 0.0f;
        senfVertices[idx++] = z2;
    }
}

int getSenfVertexCount(int segments) {
    return (segments + 1) * 2 + (segments + 2) + (segments + 2) + segments * 3;
}

unsigned int setupSenfBuffers(const float* vertices, int vertexCount) {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertexCount * 10 * sizeof(float), vertices, GL_STATIC_DRAW);

    unsigned int stride = 10 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void drawSenf(int segments) {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
    int offset = (segments + 1) * 2;
    glDrawArrays(GL_TRIANGLE_FAN, offset, segments + 2);
    offset += segments + 2;
    glDrawArrays(GL_TRIANGLE_FAN, offset, segments + 2);
    offset += segments + 2;
    glDrawArrays(GL_TRIANGLES, offset, segments * 3);
}