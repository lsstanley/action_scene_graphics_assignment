/*
Author: Luke Stanley
Matriculation No: 190010293
Date: 4/Nov/2024
Title: Bike pedal... unicycle to be potentially...
*/



/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <vector>
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#else
#pragma comment(lib, "glfw3.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

// We'll use the STD stack class to make our stack or matrices
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>


// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
 

GLuint program;		/* Identifier for the shader program */
GLuint vao;			/* Vertex array (Container) object. This is the index of the VAO that will be the container for
					our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					I've included this to show you how to pass in an unsigned integer into
					your vertex shader. */

GLuint lightmode; // Uniform to switch the lightmode in the vertex shader
GLuint emissive;

/* Position and view globals */
GLfloat x, y, z, light_x, light_y, light_z;
GLfloat angle_x, angle_inc_x, angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat model_scale; 
GLfloat paused_inc_x, paused_inc_y, paused_inc_z; // for storing the paused values
GLfloat light_intensity, is_attenuation; 
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere light_intensity    GLuint is_attenuation; 

GLboolean isPaused;
GLboolean shifting; 
 /* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID, lightmodeID, emissiveID, lightposID, attenuationID, lightintensityID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
// Sphere aSphere;
//Cube aCube;
Cylinder aCylinder;
using namespace std;
using namespace glm;

Cylinder wheelHub[4]; // all the different parts required for the wheel hub. 0 is the middle, 1 is the left [| 2 is |] 3 is left = 4 is right =  =[|==|]= 

static void createWheelHub()
{
  for (int i = 0; i<=4 ; i++) {
    wheelHub[i].makeCylinder();
  }
}


/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = y =	z = 0;
	// light_x = 1.6f;
	// light_y = 5.2f;
	// light_z = 14.2f;
  light_x = .0f;
  light_y = .0f; 
  light_z = .0f;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	paused_inc_x = paused_inc_y = paused_inc_z = 0; 
	model_scale = 1.f;
	aspect_ratio = 1024.f / 768.f;	// Initial aspect ratio from window size (variables would be better!)
	colourmode = 1;		// 0=use colour attrib buffer, 1=colour defined in shaders
	lightmode = 0;
	emissive = 0;
	numlats = 80;		// Number of latitudes in our sphere
	numlongs = 80;		// Number of longitudes in our sphere
  is_attenuation = 1; 
  light_intensity = .5f; 
	isPaused = GL_FALSE;
  shifting = false;
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("action_scene.vert", "action_scene.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	lightmodeID = glGetUniformLocation(program, "lightmode");
	lightposID = glGetUniformLocation(program, "lightpos");

	cout << "lightposID:" << lightposID << endl; 
	emissiveID = glGetUniformLocation(program, "is_emissive");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
  lightintensityID = glGetUniformLocation(program, "light_intensity");
  attenuationID = glGetUniformLocation(program, "is_attenuation");

	/* create our sphere and cube objects */
	// aSphere.makeSphere(numlats, numlongs, vec4(1.f, 0, 0.5f, 1.f));
  createWheelHub();  
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	// glClearColor(0.23f,0.24f, 0.25f, 1.0f);
	glClearColor(0.0f,0.0f, 0.0f, 1.0f);
	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Projection matrix : 30� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define our transformations that apply to all our objects 
	// by modifying the matrix at the top of the stack
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

  vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(lightmodeID, lightmode);
  glUniform1f(lightintensityID, light_intensity);
  glUniform1ui(attenuationID, is_attenuation);
 //	Push the current value of the top of the matrix onto the stack
//	I like to add a code block here to emphasise  the section between push and pop
//	This block of code draws the sphere

	// model.push(model.top());
	// {
	// 	model.top() = translate(model.top(), vec3(x, 0, 0));
	// 	model.top() = scale(model.top(), vec3(model_scale / 3.f, model_scale / 3.f, model_scale / 3.f));//scale equally in all axis
	//
	// 	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
	//
	// 	/* Draw our sphere */
	//   aSphere.drawSphere(drawmode);
	// }
	// model.pop();
  
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0, 0, 0));
		model.top() = scale(model.top(), vec3(model_scale / 3.f, model_scale/1.25f, model_scale / 3.f));//scale equally in all axis

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		/* Draw our sphere */
		aCylinder.drawCylinder(drawmode);
	}
	model.pop();

 //0 is the middle, 1 is the left [| 2 is |] 3 is left = 4 is right =  =[|==|]= 
  for (int i = 0; i<5; i++){
    if(i == 0){ // middle 
      model.push(model.top());
      {                                   //couldn't get radians() working for some reason for a constant 90 degrees
        model.top() = rotate(model.top(), 1.5708f, vec3(1,0,0)); // makes the top/bottom of the cylinder face the same side as the camera
        model.top() = translate(model.top(), vec3(0,0,0));
        model.top() = scale(model.top(), vec3(model_scale / 3.f, model_scale/1.25f, model_scale / 3.f));

        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        wheelHub[i].drawCylinder(drawmode);
      }
      model.pop();
    }
    if(i == 1 || i == 2) { // 1 is[|  2 is |]
      model.push(model.top());
      { 
        float DISTANCE_APART = .5f;
        float distanceApart = (i==1)?-DISTANCE_APART:DISTANCE_APART;
        // cout << "Distance apart: " << distanceApart << endl; 
        //couldn't get radians() working for some reason with a constant 90 degrees
        model.top() = rotate(model.top(), 1.5708f, vec3(1,0,0)); // makes the top/bottom of the cylinder face the same side as the camera
        model.top() = translate(model.top(), vec3(0,distanceApart,0));
        model.top() = scale(model.top(), vec3(model_scale / 2.5f, model_scale/5.f, model_scale/2.5f));
        
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        wheelHub[i].drawCylinder(drawmode);
      }
      model.pop();
    }
  }

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	// Store aspect ratio to use for our perspective projection
	aspect_ratio = float(w) / float(h);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS){
		angle_inc_x = .0f; 
		angle_inc_y = .0f;
		angle_inc_z = .0f;
	}

  if(key == GLFW_KEY_RIGHT_SHIFT){
    if(action == GLFW_PRESS){
      shifting = true; 
      cout << "Shift pressed" << endl; 
    }else if (action == GLFW_RELEASE) {
      shifting = false;
      cout << "shift released" << endl; 
    }
  }

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		isPaused = !isPaused;
		if(isPaused){
			paused_inc_x = angle_inc_x; 
			paused_inc_y = angle_inc_y;
			paused_inc_z = angle_inc_z; 
			angle_inc_x = 0.f;
			angle_inc_y = 0.f; 
			angle_inc_z = 0.f; 
		}else{
			angle_inc_x = paused_inc_x; 
			angle_inc_y = paused_inc_y; 
			angle_inc_z = paused_inc_z;
			paused_inc_x = 0.f; 
			paused_inc_y = 0.f;
			paused_inc_z = 0.f; 
		}
	}
  if(!shifting){
    if(key == 'P' && action == GLFW_PRESS) emissive = 1 - emissive;
    if(!isPaused){
      if (key == 'W') angle_inc_x -= 0.05f;
      if (key == 'S') angle_inc_x += 0.05f;
      if (key == 'D') angle_inc_y += 0.05f;
      if (key == 'A') angle_inc_y -= 0.05f;
      if (key == 'R') angle_inc_z -= 0.05f;
      if (key == 'T') angle_inc_z += 0.05f;
      if (key == 'Q') model_scale -= 0.02f;
      if (key == 'E') model_scale += 0.02f;
      if (key == 'Z') x -= 0.05f;
      if (key == 'X') x += 0.05f;
      if (key == 'C') y -= 0.05f;
      if (key == 'V') y += 0.05f;
      if (key == 'B') z -= 0.05f;
      if (key == 'N') z += 0.05f;
    }
  }
  if (key == 'J') light_x -= 0.2f;
  if (key == 'L') light_x += 0.2f;
  if (key == 'K') light_y -= 0.2f; 
  if (key == 'I') light_y += 0.2f; 
  if (key == 'O') light_z -= 0.2f;
  if (key == 'U') light_z += 0.2f;
  if (key == '.' && shifting && light_intensity -.1 > .1){
    light_intensity -= .1f;
    cout << light_intensity << endl; 
  } 
  if (key == ',' && shifting){ 
    light_intensity += .1;
    cout << light_intensity << endl;
  }
	if (key == 'M' && action != GLFW_PRESS)	colourmode = !colourmode;
	if (key == '.' && action != GLFW_PRESS && !shifting)
	{
		lightmode++;
		if (lightmode > 1) lightmode = 0; 
		cout << lightmode << endl;

	}

  if(key == 'P' && action == GLFW_PRESS && shifting) cout << light_x << "," << light_y << "," << light_z << endl;
  if (key == ';' && action != GLFW_PRESS){
    is_attenuation++;
    if (is_attenuation > 1) is_attenuation = 0;
  }
	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS && !shifting)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}

}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab3 start example");;

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD. Exiting." << endl;
		return -1;
	}

	// Register the callback functions
	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);
	
	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}
