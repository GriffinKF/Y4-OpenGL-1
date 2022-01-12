/**
 * CylinderRed.h
 * Example of a cylinderRed. This was created by David Ogle in 2015 and updated and tidied up
 * by Iain Martin in November 2017.
 * Provided to the AC41001/AC51008 Graphics class to help debug their own cylinderRed objects or to
 * used in their assignment to provide another flexible 
 */

#ifndef CYLINDERRED_H
#define CYLINDERRED_H

#include "wrapper_glfw.h"
#include <glm/glm.hpp>

class CylinderRed
{
private:
	glm::vec3 colour;
	GLfloat radius, length;
	GLuint definition;
	GLuint cylinderRedBufferObject, cylinderRedNormals, cylinderRedColours, cylinderRedElementbuffer;
	GLuint num_pvertices;
	GLuint isize;
	GLuint numberOfvertices;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	void defineVertices();

public:
	CylinderRed();
	CylinderRed(glm::vec3 c);
	~CylinderRed();
	void makeCylinderRed();
	void drawCylinderRed(int drawmode);
};

#endif
