#ifndef NAME_H
#define NAME_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class Name {
private:
	unsigned int nameShader;
	unsigned int nameVAO, nameVBO, nameEBO;
	unsigned int nameTexture;

public:
	Name(const char* imagePath);
	~Name();

	void draw(float alpha = 1.0f);
	void setTexture(const char* imagePath);
};

#endif