/*
 poslight.cpp
 Demonstrates a positional light with attenuation
 Displays a cube and a sphere and a small sphere to show the light position
 Includes controls to move the light source and rotate the view.
 This is a solution to lab4: Phong lighting (with the vertex and fragemknt shaders).
 Iain Martin October 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include "cylinderCigar.h"
#include "cylinderRed.h"

// Including headers for Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace glm;

const int NUM_PROGRAMS = 3;

GLuint program[NUM_PROGRAMS];		/* Identifiers for the shader prgorams */
GLuint current_program;
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat openLid, openLid_inc;

GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID[NUM_PROGRAMS], viewID[NUM_PROGRAMS], projectionID[NUM_PROGRAMS], lightposID[NUM_PROGRAMS], normalmatrixID[NUM_PROGRAMS];
GLuint colourmodeID[NUM_PROGRAMS], emitmodeID[NUM_PROGRAMS], attenuationmodeID[NUM_PROGRAMS];

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

Cube aCube;
Cube brownCube;
Cube darkBrownCube;
Sphere aSphere;
Cylinder aCylinder;
CylinderCigar aCylinderCigar;
CylinderRed aCylinderRed;


/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 0;
	light_y = 0.3; light_z = 0; light_x = 0.9;
	angle_x = angle_y = angle_z = openLid = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = openLid = 0;const float roughness = 0.8;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; 
	emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere


	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		//program[0] = glw->LoadShader("..\\..\\shaders\\poslight.vert", "..\\..\\shaders\\poslight.frag");
		program[1] = glw->LoadShader("..\\..\\shaders\\fraglight.vert", "..\\..\\shaders\\fraglight.frag");
		//program[2] = glw->LoadShader("..\\..\\shaders\\fraglight.vert", "..\\..\\shaders\\fraglight_oren_nayar.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define the same uniforms to send to both shaders */
	for (int i = 0; i < NUM_PROGRAMS; i++)
	{
		glUseProgram(program[i]);
		modelID[i] = glGetUniformLocation(program[i], "model");
		colourmodeID[i] = glGetUniformLocation(program[i], "colourmode");
		emitmodeID[i] = glGetUniformLocation(program[i], "emitmode");
		attenuationmodeID[i] = glGetUniformLocation(program[i], "attenuationmode");
		viewID[i] = glGetUniformLocation(program[i], "view");
		projectionID[i] = glGetUniformLocation(program[i], "projection");
		lightposID[i] = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID[i] = glGetUniformLocation(program[i], "normalmatrix");
	}
	// Define the index which represents the current shader (i.e. default is gouraud)
	current_program = 1;

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	aCube.makeCube(0);
	brownCube.makeCube(1);
	darkBrownCube.makeCube(2);
	aCylinder.makeCylinder();
	aCylinderCigar.makeCylinderCigar();
	aCylinderRed.makeCylinderRed();

	cout << "The Key Controls attempt to follow a logical control:" << endl;
	cout << "" << endl;
	cout << "Camera Controls:" << endl;
	cout << "W: Move Camera Up " << endl;
	cout << "S: Move Camera Down " << endl;
	cout << "A: Move Camera Right " << endl;
	cout << "D: Move Camera Left " << endl;
	cout << ",: Zoom Out (This key is selected because it is '<' on my keyboard" << endl;
	cout << ".: Zoom Out (This key is selected because it is '>' on my keyboard" << endl;
	cout << "" << endl;
	cout << "Lighting Controls" << endl;
	cout << "T: Move Light Up" << endl;
	cout << "G: Move Light Down" << endl;
	cout << "F: Move Light Right" << endl;
	cout << "H: Move Light Left" << endl;
	cout << "R: Move Light Towards You" << endl;
	cout << "Y: Move Light Away From You" << endl;
	cout << "" << endl;
	cout << "Object Controls" << endl;
	cout << "I: Rotate Object Up" << endl;
	cout << "K: Rotate Object Down" << endl;
	cout << "J: Rotate Object Left" << endl;
	cout << "L: Rotate Object Right" << endl;
	cout << "U: Tilt Object Left" << endl;
	cout << "O: Tilt Object Right" << endl;
	cout << "" << endl;
	cout << "Animation Controls" << endl;
	cout << "Arrow Key Up: Open The Box" << endl;
	cout << "Arrow Key Down: Close The Box" << endl;
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.75, 0.75, 0.75, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program[current_program]);


	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get applied to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[current_program], 1, &lightpos[0]);
	glUniform1ui(attenuationmodeID[current_program], attenuationmode);

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f));// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID[current_program], emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID[current_program], emitmode);
	}
	model.pop();

	// Define the global model transformations (rotate and scale). Note, we're not modifying the light source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// This block of code draws the base of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0, -0.5f, 0));
		model.top() = scale(model.top(), vec3(3, 0.2, 3));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		brownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the left of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.75f, -0.3f, 0));
		model.top() = scale(model.top(), vec3(0.2, 1, 3));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		brownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the right of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0.75f, -0.3f, 0));
		model.top() = scale(model.top(), vec3(0.2, 1, 3));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		brownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the back of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0, -0.3f, -0.7f));
		model.top() = scale(model.top(), vec3(3, 1, 0.2));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		brownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the front of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(0, -0.3f, 0.7f));
		model.top() = scale(model.top(), vec3(3, 1, 0.2));
	
		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		brownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws the top of the cigar box
	model.push(model.top());
	{
		// Define the model transformations for the cube
		//model.top() = translate(model.top(), vec3(0, -0.05, -0.75));
		//model.top() = rotate(model.top(), radians(openLid), vec3(1, 0, 0));
		//model.top() = translate(model.top(), vec3(0, 0.05, 0.75));
		model.top() = scale(model.top(), vec3(3.2, 0.2, 3));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		
		/* Draw our cube*/
		darkBrownCube.drawCube(drawmode);
	}
	model.pop();

	// This block of code draws cylinder that acts as the hinges of the cigar box on the left side
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.75, -0.05, -0.75));
		model.top() = rotate(model.top(), radians(90.0f), vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.1, 0.05, 0.1));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws cylinder that acts as the hinges of the cigar box on the right side
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.75, -0.05, -0.75));
		model.top() = rotate(model.top(), radians(90.0f), vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.1, 0.05, 0.1));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinder.drawCylinder(drawmode);
	}
	model.pop();

	//Cigar 1
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0, -0.4, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 1
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0, -0.4, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 2
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.1, -0.4, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 2
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.1, -0.4, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 3
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.2, -0.4, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();
	
	//Cigar Band 3
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.2, -0.4, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 4
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.1, -0.4, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 4
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.1, -0.4, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 5
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.2, -0.4, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 5
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.2, -0.4, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 6
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.05, -0.315, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));
		

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 6
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.05, -0.315, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));


		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 7
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.15, -0.315, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));
		

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 7
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(0.15, -0.315, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));


		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 8
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.05, -0.315, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));
		

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 8
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.05, -0.315, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));


		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	//Cigar 9
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.15, -0.315, 0));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.05, 0.5, 0.05));
		

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderCigar.drawCylinderCigar(drawmode);
	}
	model.pop();

	//Cigar Band 9
	model.push(model.top());
	{
		// Define the model transformations for the cylinder
		model.top() = translate(model.top(), vec3(-0.15, -0.315, 0.15));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.051, 0.05, 0.051));


		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCylinderRed.drawCylinderRed(drawmode);
	}
	model.pop();

	///* Draw a small strip light */
	//model.push(model.top());
	//{
	//	model.top() = translate(model.top(), vec3(0, -0.05, 0.55));
	//	model.top() = translate(model.top(), vec3(0, -0.05, -1.25));
	//	model.top() = rotate(model.top(), radians(openLid), vec3(1, 0, 0));
	//	model.top() = translate(model.top(), vec3(0, 0.05, 1.25));
	//	model.top() = scale(model.top(), vec3(2.6, 0.05, 0.05));// Recalculate the normal matrix and send to the vertex shader
	//	glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
	//	normalmatrix = transpose(inverse(mat3(view * model.top())));
	//	glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

	//	/* Draw our lightposition strip  with emit mode on*/
	//	emitmode = 1;
	//	glUniform1ui(emitmodeID[current_program], emitmode);
	//	aCube.drawCube(drawmode);
	//	emitmode = 0;
	//	glUniform1ui(emitmodeID[current_program], emitmode);
	//}
	//model.pop();
	
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	//Prevents the lid from opening more than logically allowed
	openLid += openLid_inc;
	if (openLid < -90) {
		openLid = -90;
		openLid_inc = 0;
	}
	//Prevents the lid from closing more than logically allowed
	if (openLid > 0) {
		openLid = 0;
		openLid_inc = 0;
	}
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
//	if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	//Continuous Rotating Controls
	if (key == 'W') angle_inc_x -= 0.05f;
	if (key == 'S') angle_inc_x += 0.05f;
	if (key == 'A') angle_inc_y -= 0.05f;
	if (key == 'D') angle_inc_y += 0.05f;
	if (key == 'Q') angle_inc_z -= 0.05f;
	if (key == 'E') angle_inc_z += 0.05f;
	if (key == ',') model_scale -= 0.02f;
	if (key == '.') model_scale += 0.02f;
	
	//Animation Control
	if (key == GLFW_KEY_UP) openLid_inc -= 0.05f;
	if (key == GLFW_KEY_DOWN) openLid_inc += 0.05f;

	/*if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;*/

	//Light Controls
	if (key == 'F') light_x -= 0.05f;
	if (key == 'H') light_x += 0.05f;
	if (key == 'G') light_y -= 0.05f;
	if (key == 'T') light_y += 0.05f;
	if (key == 'Y') light_z -= 0.05f;
	if (key == 'R') light_z += 0.05f;
	
	//Single Press Rotation
	if (key == 'K') vx -= 1.f;
	if (key == 'I') vx += 1.f;
	if (key == 'L') vy -= 1.f;
	if (key == 'J') vy += 1.f;
	if (key == 'U') vz -= 1.f;
	if (key == 'O') vz += 1.f;

	/* Turn attenuation on and off */
	if (key == '.' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
	}
}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Assignment 1: Cigar Box");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}