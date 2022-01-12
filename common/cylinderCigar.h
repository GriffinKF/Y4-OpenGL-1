/**
 * CylinderCigar.h
 * Example of a cylinderCigar. This was created by David Ogle in 2015 and updated and tidied up
 * by Iain Martin in November 2017.
 * Provided to the AC41001/AC51008 Graphics class to help debug their own cylinderCigar objects or to
 * used in their assignment to provide another flexible 
 */

#ifndef CYLINDERCIGAR_H
#define CYLINDERCIGAR_H

#include "wrapper_glfw.h"
#include <glm/glm.hpp>

class CylinderCigar
{
private:
	glm::vec3 colour;
	GLfloat radius, length;
	GLuint definition;
	GLuint cylinderCigarBufferObject, cylinderCigarNormals, cylinderCigarColours, cylinderCigarElementbuffer;
	GLuint num_pvertices;
	GLuint isize;
	GLuint numberOfvertices;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	void defineVertices();

public:
	CylinderCigar();
	CylinderCigar(glm::vec3 c);
	~CylinderCigar();
	void makeCylinderCigar();
	void drawCylinderCigar(int drawmode);
};

#endif
