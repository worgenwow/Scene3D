#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include <model.h>
#include <objLoader.h>
#include <lights.h>
#include <shader.h>
#include <data_struct.h>

// callback for when freeglut gets an error
void logError(const char *fmt, va_list ap);

// callback for when freeglut gets a warning
void logWarning(const char *fmt, va_list ap);

// sets up glut, glut window and glew
void initialiseGLUT(int argc, char **argv);

// registers all the glut callbacks
void setGlutCallbacks(Data *data);

// handles result of user input before making render call
void idle(void *data);

// handles keydown and up events for normal keys
void normalKeyChange(unsigned char key, bool keyDown, Data *d);

// callback for when window changes size
void changeSize(int w, int h, void *data);

// callback for normal key down
void normalKeyDown(unsigned char key, int x, int y, void *data);

// callback for normal key up
void normalKeyUp(unsigned char key, int x, int y, void *data);

// callback for special key down
void specialKeyDown(int key, int x, int y, void *data);

// callback for when mouse moves in window with at least one mouse button down
void mouseMovement(int x, int y, void *data);

// callback for when a mouse button up or down (state)
void mouseClick(int button, int state, int x, int y, void *data);

/* creates OpenGL shaders from data
   and initialises some shader uniforms */
void createShaders(Data *d);

// renders the scene
void renderScene(void *data);

// loads objects into data
void loadObjects(Data *d);

int main(int argc, char **argv) {
  initialiseGLUT(argc, argv);

  Data data;
  createShaders(&data);
  loadObjects(&data);
  setGlutCallbacks(&data);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  // enter the glut event processing cycle
  data.previousTime = glutGet(GLUT_ELAPSED_TIME);
  glutMainLoop();
  
  return 0;
}

void renderScene(void *data) {
  Data *d = static_cast<Data *>(data);
  
  // clear the buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // get the view matrix
  glm::mat4 view = d->camera.getViewMatrix();
  // get time in seconds
  float time = glutGet(GLUT_ELAPSED_TIME)/1000.f;

  glm::vec3 lightPosition = glm::vec3(1*sin(time), 0.0f, 1*cos(time));

  LightProps lightProps;
  lightProps.specular = glm::vec3(1.0f, 1.0f, 1.0f);
  lightProps.diffuse = lightProps.specular * glm::vec3(0.7f);
  lightProps.ambient = lightProps.diffuse * glm::vec3(0.1f);

  LightDropOff lightDropOff;
  lightDropOff.constant = 1.0f;
  lightDropOff.linear = 0.09f;
  lightDropOff.quadratic = 0.032f;

  PointLight pointLight;
  pointLight.lightProps = lightProps;
  pointLight.lightDropOff = lightDropOff;
  pointLight.position = lightPosition;

  DirLight dirLight;
  dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
  dirLight.lightProps = lightProps;

  // dim the dirlight
  dirLight.lightProps.specular *= glm::vec3(0.1f);
  dirLight.lightProps.diffuse *= glm::vec3(0.1f);
  dirLight.lightProps.ambient *= glm::vec3(0.1f);

  Spotlight spotlight;
  spotlight.lightProps = lightProps;
  spotlight.lightDropOff = lightDropOff;
  spotlight.cutOff = cos(12.5f*M_PI/180);
  spotlight.outerCutOff = cos(17.5f*M_PI/180);
  spotlight.position = d->camera.getPosition();
  spotlight.direction = d->camera.getFrontVector();

  // set all the uniform variables for the object
  d->shaders[d->OBJECT]->use();
  d->shaders[d->OBJECT]->setMat4("projection", d->proj);
  d->shaders[d->OBJECT]->setMat4("view", view);
  d->shaders[d->OBJECT]->setVec3("viewPos", spotlight.position);
  d->shaders[d->OBJECT]->setSpotlight("spotlight", spotlight);
  d->shaders[d->OBJECT]->setDirLight("dirLight", dirLight);
  d->shaders[d->OBJECT]->setPointLight("pointLight", pointLight);

  glm::vec3 cubePositions[] = {
    glm::vec3( 2.0f,  0.0f,  0.0f)
  };

  // draw the objects
  for(int i=0;i<1;i++) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[i]);
    float angle = 20.0f * i; 
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

    // calculate the normal matrix for correcting normal vector after any transformations
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    d->shaders[d->OBJECT]->setMat4("model", model);
    d->shaders[d->OBJECT]->setMat3("normalMatrix", normalMatrix);
    d->cube.draw(d->shaders[d->OBJECT]);
  }

  glm::mat4 backpackModel = glm::mat4(1.0f);
  backpackModel = glm::scale(backpackModel, glm::vec3(0.4f));

  d->shaders[2]->use();
  d->shaders[2]->setMat4("projection", d->proj);
  d->shaders[2]->setMat4("view", view);
  d->shaders[2]->setMat4("model", backpackModel);
  d->shaders[2]->setVec3("viewPos", spotlight.position);
  d->shaders[2]->setSpotlight("spotlight", spotlight);
  d->shaders[2]->setDirLight("dirLight", dirLight);
  d->shaders[2]->setPointLight("pointLight", pointLight);

  d->backpack.draw(d->shaders[2]);

  // move light to position and make it smaller
  glm::mat4 lightModel = glm::mat4(1.0f);
  lightModel = glm::translate(lightModel, lightPosition);
  lightModel = glm::scale(lightModel, glm::vec3(0.2f)); 

  // sets all the uniform variables for the light
  d->shaders[d->LIGHT_SOURCE]->use();
  d->shaders[d->LIGHT_SOURCE]->setMat4("projection", d->proj);
  d->shaders[d->LIGHT_SOURCE]->setMat4("view", view);
  d->shaders[d->LIGHT_SOURCE]->setMat4("model", lightModel);
  d->shaders[d->LIGHT_SOURCE]->setVec3("lightColor",  pointLight.lightProps.specular);

  // draw the light source
  d->cube.draw(d->shaders[d->LIGHT_SOURCE]);

  // show drawn buffer to screen
  glutSwapBuffers();
}

void setGlutCallbacks(Data *data) {
  glutDisplayFuncUcall(renderScene, data);
  glutReshapeFuncUcall(changeSize, data);
  glutIdleFuncUcall(idle, data);
  glutKeyboardFuncUcall(normalKeyDown, data);
  glutKeyboardUpFuncUcall(normalKeyUp, data);
  glutSpecialFuncUcall(specialKeyDown, data);
  glutMotionFuncUcall(mouseMovement, data);
  glutMouseFuncUcall(mouseClick, data);
}

void logError(const char *fmt, va_list ap) {
  printf("freeglut - ");
  vprintf(fmt, ap);
  printf("\n");

  glutExit();
  exit(1);
}

void logWarning(const char *fmt, va_list ap) {
  printf("freeglut - ");
  vprintf(fmt, ap);
  printf("\n");
}

void initialiseGLUT(int argc, char **argv) {
  /* sets error logging functions to catch
     any intialisation errors */
  glutInitErrorFunc(logError);
  glutInitWarningFunc(logWarning);

  // initialises GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 3); // use opengl version 3.3
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // creates window
  glutInitWindowPosition(-1, -1);
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Test");

  // intialises GLEW
  glewExperimental = GL_FALSE; 
  GLenum glewError = glewInit();
  if(glewError != GLEW_OK) {
    printf("Error initializing GLEW. %s\n", glewGetErrorString(glewError));
  }
}

void idle(void *data) {
  Data *d = static_cast<Data *>(data);

  float deltaTime = (glutGet(GLUT_ELAPSED_TIME) - d->previousTime)/1000.f;

  d->camera.updatePos(&d->keyData, deltaTime);
  d->camera.updateAngle(&d->mouseChange, deltaTime);
  d->mouseChange = glm::vec2(0,0);

  d->previousTime = glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay();
}

void normalKeyChange(unsigned char key, bool keyDown, Data *d) {
  switch(key) {
    case 'w':
      d->keyData.wKeyDown = keyDown;
      break;
    case 'a':
      d->keyData.aKeyDown = keyDown;
      break;
    case 's':
      d->keyData.sKeyDown = keyDown;
      break;
    case 'd':
      d->keyData.dKeyDown = keyDown;
      break;
    case SPACEBAR:
      d->keyData.spaceKeyDown = keyDown;
      break;
    case 'c':
      d->keyData.cKeyDown = keyDown;
      break;
  }
}

void changeSize(int w, int h, void *data) {
  Data *d = static_cast<Data *>(data);

  if(h == 0) // don't want to divide by 0
    h = 1;
  float ratio = (float) w / (float) h;

  d->screenHeight = h;
  d->screenWidth  = w;

  glViewport(0, 0, w, h);             // sets viewport to be entire window
  d->proj = glm::perspective(glm::radians(45.f), ratio, 0.1f, 100.0f); // sets the perspective
}

void normalKeyDown(unsigned char key, int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);

  int mod = glutGetModifiers();
  if(key == 27) {
    glutLeaveMainLoop();
  }

  if(key == 'r') {
    if(mod == GLUT_ACTIVE_ALT) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   
    }
  }

  normalKeyChange(key, true, d);
}

void normalKeyUp(unsigned char key, int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);

  normalKeyChange(key, false, d);
}

// callback for special key presses
void specialKeyDown(int key, int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);

  switch(key) {
    case GLUT_KEY_F1:
      break;
    case GLUT_KEY_DOWN:
      break;
    case GLUT_KEY_UP:
      break;
  }
}

void mouseMovement(int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);
  if(d->mouse1Down) {
    d->mouseChange = glm::vec2(x - d->screenWidth/2, y - d->screenHeight/2);

    glutWarpPointer(d->screenWidth/2, d->screenHeight/2);
  }
}

void mouseClick(int button, int state, int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);

  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    glutWarpPointer(d->screenWidth/2, d->screenHeight/2);
    d->mouse1Down = true;
  } else {
    d->mouse1Down = false;
  }
}

void createShaders(Data *d) {
  d->shaders[d->OBJECT]       = new Shader("./shaders/object.vs", "./shaders/backpack.fs");
  d->shaders[d->LIGHT_SOURCE] = new Shader("./shaders/object.vs", "./shaders/light_source.fs");
  d->shaders[2]               = new Shader("./shaders/object.vs", "./shaders/simple.fs");
}

void loadObjects(Data *d) {
  ObjLoader loader;
  loader.loadObj("./objects/backpack/backpack.obj", d->backpack);
  loader.loadObj("./objects/cube/cube.obj", d->cube);
}