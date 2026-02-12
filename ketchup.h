#pragma once
#include <GL/glew.h>

// Generiše geometriju za ke?ap (valjak + kupa)
void generateKetchup(float* ketchupVertices, int segments = 30);

// Vra?a broj verteksa potreban za ke?ap
int getKetchupVertexCount(int segments = 30);

// Kreira VAO/VBO za ke?ap i vra?a VAO
unsigned int setupKetchupBuffers(const float* vertices, int vertexCount);

// Crta ke?ap (poziva odgovaraju?e glDrawArrays)
void drawKetchup(int segments = 30);

void drawKetchupWithTransform(unsigned int ketchupVAO, int segments);
