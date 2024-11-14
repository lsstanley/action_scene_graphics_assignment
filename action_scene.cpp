/*
Author: Luke Stanley
Matriculation No: 190010293
Date: 4/Nov/2024
Title:  Unicycle
*/



/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/vector_float3.hpp"
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
 
using namespace std;
using namespace glm;


GLuint program;		/* Identifier for the shader program */
GLuint vao;			/* Vertex array (Container) object. This is the index of the VAO that will be the container for
					our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					I've included this to show you how to pass in an unsigned integer into
					your vertex shader. */

GLuint lightmode; // Uniform to switch the lightmode in the vertex shader
GLuint emissive;

/* Position and view globals */

GLfloat cam_x, cam_y, cam_z; 
GLfloat x, y, z, light_x, light_y, light_z;
GLfloat angle_x, angle_inc_x, angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat wheel_rotation, wheel_rotation_inc, paused_wheel_rotation_inc; 
GLfloat pedal_rotation, pedal_rotation_inc, paused_pedal_rotation_inc;
GLfloat model_scale; 
GLfloat paused_inc_x, paused_inc_y, paused_inc_z; // for storing the paused values
GLfloat light_intensity, is_attenuation; 
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere light_intensity    GLuint is_attenuation; 

const GLfloat WHEEL_RADIUS = 2.5f;
const GLint WHEEL_COUNT = 20; // the amount of connecting cubes there will be for the wheel
const GLint SPOKE_AMT = 10; // for each side of the wheel

// length of each connecting cube
// circumference / amount of cubes
const GLfloat WHEEL_LENGTH = (2*pi<float>()*WHEEL_RADIUS)/WHEEL_COUNT;

const vec3 FRAME_COLOUR = vec3(0,0,0); //vec3 as cylinder requires colours to be vec3 
const float MAX_SEAT_HEIGHT = 1.3f; 

GLboolean isPaused;
GLboolean shifting; 
GLboolean controlPressed;
 /* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID, lightmodeID, emissiveID, lightposID, attenuationID, lightintensityID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

// frame parts
Cube fork[2];// | | on each end of the wheelHub leading up to the wheel. 
Cube forkBridge; // I'm not sure what to call it but I want to use a cylinder connecting the fork pieces together rather than just using cubes for it all
//
GLfloat seat_height; // adjustable variable for the height of the chair, can't exceed MAX_SEAT_HEIGHT
Cylinder seatAdjuster; // the part of the frame that is from the fork bridge to the seat
Cylinder seatAdjustOuter(FRAME_COLOUR); 

// wheel parts
Cylinder wheelHub[5]; // all the different parts required for the wheel hub. 0 is the middle, 1 is the left [| 2 is |] 3 is left = 4 is right =  =[|==|]= 
Cube tyre[WHEEL_COUNT];
Cube rim[WHEEL_COUNT];
Cube spokes[SPOKE_AMT];

//pedal parts
Cube pedalCrank[2]; // the part connecting the pedal and the wheel hub 
Cylinder pedalCrankBolt[2]; // side view of pedal [ o ] the o is a pedal crank bolt, just for higher detail.
Cube pedal[2]; // the foot pedals


// ------------------------------------------------------------------- // 
// seat parts

static void createSeatAdjuster()
{
  seatAdjustOuter.makeCylinder(); 
  seatAdjuster.makeCylinder();
}

//  ------------------------------------------------------------------ // 
// frame parts

static void createFork()
{
  for(int i=0; i<2; i++){
    // fork[i].makeCube(vec4(1,0.2706f,0,1)); // making them orange
    fork[i].makeCube(vec4(FRAME_COLOUR,1)); 
  }
  forkBridge.makeCube(vec4(FRAME_COLOUR,1)); 
}

// ------------------------------------------------------------------ // 
// wheel parts
static void createTyre()
{
  for (int i=0; i<=WHEEL_COUNT; i++){
    tyre[i].makeCube(vec4(0,0,0,1));
  }
}

static void createRim()
{
  for (int i=0; i<=WHEEL_COUNT; i++){
    rim[i].makeCube(vec4(1,1,1,1));
  }
}

static void createSpokes()
{
  for (int i=0; i<SPOKE_AMT; i++){
    spokes[i].makeCube(vec4(1,1,1,1));
  }
}

static void createWheelHub()
{
  for (int i = 0; i<=4; i++) {
    wheelHub[i].makeCylinder();
  }
}

// ---------------------------------------------------------------------- // 
// pedal parts
static void createPedalCrank()
{
  for (int i = 0; i<=1; i++){
    pedalCrank[i].makeCube(vec4(1,1,1,1));
  }
}

static void createPedalCrankBolt()
{
  for (int i = 0; i<=1; i++){
    pedalCrankBolt[i].makeCylinder(); 
  }
}

static void createPedals()
{
  for (int i = 0; i < 2; i++){
    pedal[i].makeCube(vec4(0,0,0,1));
  }
}



/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */ 
  cout << WHEEL_LENGTH << endl;
	x = y =	z = 0;
  wheel_rotation = wheel_rotation_inc = paused_wheel_rotation_inc = 0.f;  
  pedal_rotation = pedal_rotation_inc = paused_pedal_rotation_inc = 0.f;
  // light_x = 1.6f;
	// light_y = 5.2f;
	// light_z = 14.2f;
  cam_x = 0.f; 
  cam_y = 5.f; 
  cam_z = 16.f;
  seat_height = 0.f;
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
  controlPressed = false;
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

  /* creating the seat parts */
  createSeatAdjuster();
  /* creating frame objects */ 
  createFork(); 

	/* creating wheel objects */
  createWheelHub(); 
  createTyre();
  createRim();
  createSpokes();

  /* creating the pedal parts */ 
  createPedalCrank();
  createPedalCrankBolt(); 
  createPedals();
}


/* Called to update the display. Note that this function is called in the event loop in the wrapper
class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	// glClearColor(0.23f,0.24f, 0.25f, 1.0f);
	// glClearColor(0.0f,0.0f, 0.0f, 1.0f);
  glClearColor(0.227f, 0.227f, 0.227f, 1.0f);
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
		vec3(0, 0, cam_z), // Camera is at (0,0,8), in World Space
		vec3(cam_x, cam_y, 0), // and looks at the origin
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

 


// ----------------------------------------------------------------------------------------------------------------------
// drawing the seat objects
  //seat adjuster
  model.push(model.top());
  {
    model.top() = translate(model.top(), vec3(0,3.71f+seat_height,0));
    model.top() = scale(model.top(), vec3(model_scale/6, model_scale*1.3, model_scale/6));
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
    seatAdjuster.drawCylinder(drawmode);
  }
  model.pop();


// ----------------------------------------------------------------------------------------------------------------------
// drawing the frame objects

  // frame for the seat adjuster (i am calling it outer seat adjuster)
  model.push(model.top());
  {
    model.top() = translate(model.top(), vec3(0,3.705f,0));
    model.top() = scale(model.top(), vec3(model_scale/5, model_scale*1.3, model_scale/5));
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
    seatAdjustOuter.drawCylinder(drawmode);
  }
  model.pop();

  // the fork
  for(int i = 0; i<3; i++){
    if(i==0 || i==1){ // the cubes on both sides of the wheel 
      model.push(model.top());
      {
        GLfloat DISTANCE_APART = .54f;
        GLfloat distanceApart = (i%2==0)?-DISTANCE_APART:DISTANCE_APART;
        
        model.top() = translate(model.top(), vec3(0, 1.45f,distanceApart)); 
        model.top() = scale(model.top(), vec3(model_scale, model_scale/0.15f, model_scale/4.8f));
      
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        fork[i].drawCube(drawmode);
      }
      model.pop();
    }
    if(i==2){ // the fork bridge
      model.push(model.top());
      {
        model.top() = translate(model.top(), vec3(0, 3.0325f, 0)); 
        model.top() = scale(model.top(), vec3(model_scale, model_scale/3, model_scale*2));
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        forkBridge.drawCube(drawmode);
      }
      model.pop();
    }
  }

// ----------------------------------------------------------------------------------------------------------------------
// drawing the wheel tyre and rim


  model.top() = rotate(model.top(), -radians(wheel_rotation), vec3(0,0,1)); // because a unicycle the pedals directly impact the movement of the wheel as there is no chain, the whole wheelHub/pedals all would move together
  for(int i = 0; i < WHEEL_COUNT; i++){

    // creating the tyre
    model.push(model.top());
    {
     // https://stackoverflow.com/a/13608420 converted from java to c++ for getting the points on a circumference
 
      GLfloat cx =  WHEEL_RADIUS * cos(2*pi<float>()*(float(i)/WHEEL_COUNT));
      GLfloat cy =  WHEEL_RADIUS * sin(2*pi<float>()*(float(i)/WHEEL_COUNT));
      GLfloat ang = 360.0/WHEEL_COUNT; 
      model.top() = translate(model.top(), vec3(cx,cy,0));
      model.top() = rotate(model.top(), radians(ang*i), vec3(0,0,1));
      // the .73 is a number i found from testing to make the cubes look connected. 
      model.top() = scale(model.top(), vec3(model_scale, model_scale/(WHEEL_LENGTH*.73), model_scale));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      tyre[i].drawCube(drawmode);
    }
    model.pop();

    // creating the rim, follows the same structure as above code except from a slight tweak to the width and cx 
    model.push(model.top());
    {
      // offset of -.13 for the rims, number i liked the result for.....
      GLfloat cx =  (WHEEL_RADIUS-.13f) * cos(2*pi<float>()*(float(i)/WHEEL_COUNT));
      GLfloat cy =  (WHEEL_RADIUS-.13f) * sin(2*pi<float>()*(float(i)/WHEEL_COUNT));
      GLfloat ang = 360.0/WHEEL_COUNT; 
      model.top() = translate(model.top(), vec3(cx,cy,0));
      model.top() = rotate(model.top(), radians(ang*i), vec3(0,0,1));
      model.top() = scale(model.top(), vec3(model_scale/1.5, model_scale/(WHEEL_LENGTH*.73), model_scale/1.5));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      rim[i].drawCube(drawmode);
    }
    model.pop();
  }

  // creating the spokes
  for (int i = 0; i<SPOKE_AMT; i++){
    model.push(model.top());
    {
      const GLfloat SPOKE_RADIUS = 1/2.85f; // as the radius of a cylinder is defaulted at 1. I am using the same calculation that is being used for scaling of the wheel hub
      
      // getting the points similarly to the rims and wheels but around the spoke_radius instead
      GLfloat cx = SPOKE_RADIUS * cos(2*pi<float>()*(float(i)/SPOKE_AMT));
      GLfloat cy = SPOKE_RADIUS * sin(2*pi<float>()*(float(i)/SPOKE_AMT));
      GLfloat ang = 360.0/SPOKE_AMT;
      model.top() = translate(model.top(), vec3(cx,cy,0));
      // model.top() = rotate(model.top(), radians(5.0f), vec3(1,0,0));
      model.top() = rotate(model.top(), radians(ang*i), vec3(0,0,1));
      model.top() = scale(model.top(), vec3(model_scale/15, model_scale*9, model_scale/15));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      spokes[i].drawCube(drawmode);
    }
    model.pop();
  }

// ----------------------------------------------------------------------------------------------------------------------
// drawing the wheel hub components
 //0 is the middle, 1 is the left [| 2 is |] 3 is left = 4 is right =  =[|==|]= 
  for (int i = 0; i<5; i++){
    if(i == 0){ // middle 
      model.push(model.top());
      {                                  
        model.top() = rotate(model.top(), radians(90.0f), vec3(1,0,0)); // makes the top/bottom of the cylinder face the same side as the camera
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
        float DISTANCE_APART = .44f;
        float distanceApart = (i==1)?-DISTANCE_APART:DISTANCE_APART;
        // cout << "Distance apart: " << distanceApart << endl; 
        //couldn't get radians() working for some reason with a constant 90 degrees
        model.top() = rotate(model.top(), radians(90.0f), vec3(1,0,0)); // makes the top/bottom of the cylinder face the same side as the camera
        model.top() = translate(model.top(), vec3(0,distanceApart,0));
        model.top() = scale(model.top(), vec3(model_scale/1.9f, model_scale/12.f, model_scale/1.9f));
        
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        wheelHub[i].drawCylinder(drawmode);
      } 
      model.pop();
    }
    if(i == 3 || i == 4){ // =  = at each end of [| |]
      model.push(model.top());
      {
        float DISTANCE_APART = .59f;
        float distanceApart = (i==3)?-DISTANCE_APART:DISTANCE_APART;
        // cout << "distance apart: " << distanceApart << endl << "model scale" << model_scale << endl;
        model.top() = rotate(model.top(), radians(90.0f), vec3(1,0,0));
        model.top() = translate(model.top(), vec3(0,distanceApart,0));
        model.top() = scale(model.top(), vec3(model_scale/10.f, model_scale/4.35f, model_scale/10.f));

        glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
        wheelHub[i].drawCylinder(drawmode);
      }
      model.pop();
    }
  }

// ----------------------------------------------------------------------------------------------------------//
  // drawing the pedal components
  for (int i=0; i<2; i++) {
    // pedal cranks
    model.push(model.top());
    {
      float DISTANCE_APART = .645f;
      float distanceApart = (i==0)?-DISTANCE_APART:DISTANCE_APART; // this ternary is just making each object at opposite ends
      float invert = (i==1)?-1:1; //this is to make the cranks opposite directions from each other
      // cout << distanceApart << endl;
      model.top() = translate(model.top(), vec3(0, invert * .55f,distanceApart)); 
      model.top() = scale(model.top(), vec3(model_scale/2.f, model_scale/0.35f, model_scale/4.8f));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      pedalCrank[i].drawCube(drawmode);
    }
    model.pop();
    
    // pedal crank bolts, uses a similar set up to the pedal cranks
    model.push(model.top());
    {
      float DISTANCE_APART = .892f;
      float distanceApart = (i==0)?-DISTANCE_APART:DISTANCE_APART;
      float invert = (i==1)?-1:1; //this is to make the cranks opposite directions from each other
      // cout << distanceApart << endl;
      model.top() = translate(model.top(), vec3(0, invert * 1.175f,distanceApart));
      model.top() = rotate(model.top(), radians(90.f), vec3(1,0,0));
      model.top() = scale(model.top(), vec3(model_scale/15.f, model_scale/1.65f, model_scale/15.f));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      pedalCrankBolt[i].drawCylinder(drawmode);
    }
    model.pop();

    // pedals drawing
    model.push(model.top());
    {
      // need a similar set up to the pedal cranks as they are attached to the end of each pedal crank
      float DISTANCE_APART = .942f; 
      float distanceApart = (i==0)?-DISTANCE_APART:DISTANCE_APART;
      float invert = (i==1)?-1:1;
      
      model.top() = translate(model.top(), vec3(0, invert * 1.175f, distanceApart)); 
      model.top() = rotate(model.top(), -radians(pedal_rotation), vec3(0,0,1)); // rotating the pedals by the input given by the user
      model.top() = rotate(model.top(), radians(wheel_rotation), vec3(0,0,1));  // rotate the pedal in the opposite direction from the main wheel rotation so that if you do not add a pedal rotation they are always flat as if you are riding it 
      model.top() = scale(model.top(), vec3(model_scale, model_scale/3.f, model_scale));
      glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
      pedal[i].drawCube(drawmode);
    }
    model.pop();
  }
	glDisableVertexAttribArray(0);
	glUseProgram(0);

  // pedal rotation
  pedal_rotation += pedal_rotation_inc; 
  //wheel rotation
  wheel_rotation += wheel_rotation_inc; 
  //camera rotations 
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

  
  if(key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL){
    if(action == GLFW_PRESS){
      controlPressed = true; 
      cout << "control pressed" << endl; 
    }else if (action == GLFW_RELEASE) {
      controlPressed = false;
      cout << "control released" << endl; 
    }
  }

  if(key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT){
    if(action == GLFW_PRESS){
      shifting = true; 
      cout << "Shift pressed" << endl; 
    }else if (action == GLFW_RELEASE) {
      shifting = false;
      cout << "shift released" << endl; 
    }
  }

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS && !shifting && !controlPressed){ // stops the camera rotations
		angle_inc_x = .0f; 
		angle_inc_y = .0f;
		angle_inc_z = .0f;
	} 
  if(key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS && shifting && !controlPressed){ // stops wheel speed
    wheel_rotation_inc = .0f; 
  }
  if(key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS && !shifting && controlPressed){ // stops pedal speed
    pedal_rotation_inc = .0f;
  }

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		isPaused = !isPaused;
		if(isPaused){
      paused_wheel_rotation_inc = wheel_rotation_inc; 
      paused_pedal_rotation_inc = pedal_rotation_inc;
			paused_inc_x = angle_inc_x; 
			paused_inc_y = angle_inc_y;
			paused_inc_z = angle_inc_z;
      wheel_rotation_inc = 0.f;
      pedal_rotation_inc = 0.f;
			angle_inc_x = 0.f;
			angle_inc_y = 0.f; 
			angle_inc_z = 0.f; 
		}else{
			angle_inc_x = paused_inc_x; 
			angle_inc_y = paused_inc_y; 
			angle_inc_z = paused_inc_z;
      wheel_rotation_inc = paused_wheel_rotation_inc;
      pedal_rotation_inc = paused_pedal_rotation_inc;
			paused_inc_x = 0.f; 
			paused_inc_y = 0.f;
			paused_inc_z = 0.f;
      paused_wheel_rotation_inc = 0.f; 
      paused_pedal_rotation_inc = 0.f;
		}
	}
  if(!shifting && !controlPressed){
    if(key == 'P' && action == GLFW_PRESS) emissive = 1 - emissive;
    if(!isPaused){
      if (key == 'W') angle_inc_x -= 0.05f;
      if (key == 'S') angle_inc_x += 0.05f;
      if (key == 'D') angle_inc_y += 0.05f;
      if (key == 'A') angle_inc_y -= 0.05f;
      if (key == 'R') angle_inc_z -= 0.05f;
      if (key == 'T') angle_inc_z += 0.05f;
      if (key == 'Z') x -= 0.05f;
      if (key == 'X') x += 0.05f;
      if (key == 'C') y -= 0.05f;
      if (key == 'V') y += 0.05f;
      if (key == 'B') z -= 0.05f;
      if (key == 'N') z += 0.05f;
    }
  }
  if(shifting){ // when right shift is being held
    if(!isPaused){
      // rotate the wheel
      if(key == 'W' && action != GLFW_RELEASE) wheel_rotation_inc += 0.1f; 
      if(key == 'S' && action != GLFW_RELEASE) wheel_rotation_inc -= 0.1f; 
      if (key == 'J') cam_x -= 0.5f;
      if (key == 'L') cam_x += 0.5f;
      if (key == 'K') cam_y -= 0.5f; 
      if (key == 'I') cam_y += 0.5f; 
      if (key == 'U') cam_z += 0.5f; 
      if (key == 'O') cam_z -= 0.5f; 

    }
  }
  if(controlPressed){
    if(!isPaused){
      //rotate the pedals
      if(key == 'W' && action != GLFW_RELEASE) pedal_rotation_inc += .1f;
      if(key == 'S' && action != GLFW_RELEASE) pedal_rotation_inc -= .1f; 
    }
  }
  if (key == GLFW_KEY_UP && action != GLFW_RELEASE && seat_height+0.025 <= MAX_SEAT_HEIGHT) seat_height += .025f;
  if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE && seat_height-0.025 > 0) seat_height -= .025f; 
  if (key == 'J') light_x -= 0.2f;
  if (key == 'L') light_x += 0.2f;
  if (key == 'K') light_y -= 0.2f; 
  if (key == 'I') light_y += 0.2f; 
  if (key == 'O') light_z -= 0.2f;
  if (key == 'U') light_z += 0.2f;
  
  // > key, couldn't find it in GLFW documentation
  if (key == '.' && shifting && light_intensity -.1 > .1){
    light_intensity -= .1f;
    cout << light_intensity << endl; 
  } // < key
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
