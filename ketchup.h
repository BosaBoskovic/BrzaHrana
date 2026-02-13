#pragma once
#include <GL/glew.h>

// Generiše geometriju za ke?ap (valjak + kupa)
void generateKetchup(float* ketchupVertices, int segments = 30);

// Vraca broj verteksa potreban za kecap
int getKetchupVertexCount(int segments = 30);

// Kreira VAO/VBO za kecap i vraca VAO
unsigned int setupKetchupBuffers(const float* vertices, int vertexCount);

// Crta kecap (poziva odgovarajuce glDrawArrays)
void drawKetchup(int segments = 30);

void drawKetchupWithTransform(unsigned int ketchupVAO, int segments);
