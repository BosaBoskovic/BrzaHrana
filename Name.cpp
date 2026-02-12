#include "Name.h"
#include "Util.h"
#include <iostream>

unsigned int loadNameTexture(const char* filepath) {
	unsigned int texture = loadImageToTexture(filepath);
	glBindTexture(GL_TEXTURE_2D, texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}

Name::Name(const char* imagePath) {
	nameShader = createShader("name.vert", "name.frag");
	nameTexture = loadNameTexture(imagePath);

	float nameVertices[] = {
		//x, y				s, t
		 0.6f, -1.0f,        0.0f, 0.0f,
		 1.0f, -1.0f,        1.0f, 0.0f,
		 1.0f, -0.6f,        1.0f, 1.0f,
		 0.6f, -0.6f,        0.0f, 1.0f
	};

	unsigned int nameIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &nameVAO);
	glGenBuffers(1, &nameVBO);
	glGenBuffers(1, &nameEBO);

	glBindVertexArray(nameVAO);

	glBindBuffer(GL_ARRAY_BUFFER, nameVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(nameVertices), nameVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nameEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(nameIndices), nameIndices, GL_STATIC_DRAW);

	// Pozicija
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture koordinate
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

Name::~Name() {
	glDeleteBuffers(1, &nameVBO);
	glDeleteBuffers(1, &nameEBO);
	glDeleteVertexArrays(1, &nameVAO);
	glDeleteProgram(nameShader);
	glDeleteTextures(1, &nameTexture);
}

void Name::draw(float alpha) {
	GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);

	glDisable(GL_DEPTH_TEST);

	glUseProgram(nameShader);
	glBindVertexArray(nameVAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nameTexture);
	glUniform1i(glGetUniformLocation(nameShader, "uTexture"), 0);
	glUniform1f(glGetUniformLocation(nameShader, "uAlpha"), alpha);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	if (depthTestWasEnabled) {
		glEnable(GL_DEPTH_TEST);
	}
}

void Name::setTexture(const char* imagePath) {
	glDeleteTextures(1, &nameTexture);
	nameTexture = loadNameTexture(imagePath);
}