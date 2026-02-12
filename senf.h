#ifndef SENF_H
#define SENF_H

#include <GL/glew.h>

void generateSenf(float* senfVertices, int segments);
int getSenfVertexCount(int segments);
unsigned int setupSenfBuffers(const float* vertices, int vertexCount);
void drawSenf(int segments);

#endif