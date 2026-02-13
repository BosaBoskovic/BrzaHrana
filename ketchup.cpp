#include "ketchup.h"
#include <cmath>

void generateKetchup(float* ketchupVertices, int segments) {
    float radius = 0.08f;
    float cylinderHeight = 0.15f;
    float coneHeight = 0.08f;
    int idx = 0;

    // CRVENA BOJA
    float r = 1.0f, g = 0.0f, b = 0.0f, a = 1.0f;

    // ========== VALJAK (TELO) ==========
    // 1. OMOTAC VALJKA 
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Gornja ta?ka
        ketchupVertices[idx++] = x;
        ketchupVertices[idx++] = cylinderHeight;
        ketchupVertices[idx++] = z;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = x; ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = z; // Normal

        // Donja tacka
        ketchupVertices[idx++] = x;
        ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = z;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = x; ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = z;
    }

    // 2. GORNJI POKLOPAC VALJKA (ravan krug - baza kupe)
    ketchupVertices[idx++] = 0.0f;
    ketchupVertices[idx++] = cylinderHeight;
    ketchupVertices[idx++] = 0.0f; // Centar
    ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
    ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
    ketchupVertices[idx++] = 0.0f; ketchupVertices[idx++] = 1.0f;
    ketchupVertices[idx++] = 0.0f;

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        ketchupVertices[idx++] = cos(angle) * radius;
        ketchupVertices[idx++] = cylinderHeight;
        ketchupVertices[idx++] = sin(angle) * radius;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = 0.0f; ketchupVertices[idx++] = 1.0f;
        ketchupVertices[idx++] = 0.0f;
    }

    // 3. DONJI POKLOPAC VALJKA
    ketchupVertices[idx++] = 0.0f;
    ketchupVertices[idx++] = 0.0f;
    ketchupVertices[idx++] = 0.0f;
    ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
    ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
    ketchupVertices[idx++] = 0.0f; ketchupVertices[idx++] = -1.0f;
    ketchupVertices[idx++] = 0.0f;

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        ketchupVertices[idx++] = cos(angle) * radius;
        ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = sin(angle) * radius;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = 0.0f; ketchupVertices[idx++] = -1.0f;
        ketchupVertices[idx++] = 0.0f;
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
        ketchupVertices[idx++] = tipX;
        ketchupVertices[idx++] = tipY;
        ketchupVertices[idx++] = tipZ;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = tipX; ketchupVertices[idx++] = 1.0f;
        ketchupVertices[idx++] = tipZ;

        // Baza 1
        ketchupVertices[idx++] = x1;
        ketchupVertices[idx++] = cylinderHeight;
        ketchupVertices[idx++] = z1;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = x1; ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = z1;

        // Baza 2
        ketchupVertices[idx++] = x2;
        ketchupVertices[idx++] = cylinderHeight;
        ketchupVertices[idx++] = z2;
        ketchupVertices[idx++] = r; ketchupVertices[idx++] = g;
        ketchupVertices[idx++] = b; ketchupVertices[idx++] = a;
        ketchupVertices[idx++] = x2; ketchupVertices[idx++] = 0.0f;
        ketchupVertices[idx++] = z2;
    }
}

int getKetchupVertexCount(int segments) {
    
    return (segments + 1) * 2 + (segments + 2) + (segments + 2) + segments * 3;
}

unsigned int setupKetchupBuffers(const float* vertices, int vertexCount) {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertexCount * 10 * sizeof(float), vertices, GL_STATIC_DRAW);

    unsigned int stride = 10 * sizeof(float);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void drawKetchup(int segments) {
    // Omotac valjka
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);

    // Gornji poklopac
    int offset = (segments + 1) * 2;
    glDrawArrays(GL_TRIANGLE_FAN, offset, segments + 2);

    // Donji poklopac
    offset += segments + 2;
    glDrawArrays(GL_TRIANGLE_FAN, offset, segments + 2);

    // Kupa
    offset += segments + 2;
    glDrawArrays(GL_TRIANGLES, offset, segments * 3);
}

void drawKetchupWithTransform(unsigned int ketchupVAO, int segments) {
    glBindVertexArray(ketchupVAO);
    drawKetchup(segments);
    glBindVertexArray(0);
}