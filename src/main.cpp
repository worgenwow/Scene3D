#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <map>
#include <vector>
#include <imageLoader.h>

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

// genderates the framebuffer
void genFramebuffer(GLuint &framebuffer, GLuint &textureColorBuffer, GLuint &RBO);

// handles all the rendering
void render(void *data);

// renders the scene
void renderScene(Data *d, int shaderIndex);

// loads objects into data
void loadObjects(Data *d);

// loads a cubemap image based on vector of paths
GLuint loadCubemap(std::vector<std::string> faces);

GLuint createSkybox();

void setupUBO(Data *d);

int main(int argc, char **argv) {
  initialiseGLUT(argc, argv);

  Data data;
  createShaders(&data);
  loadObjects(&data);
  setGlutCallbacks(&data);
  setupUBO(&data);
  genFramebuffer(data.framebuffers[0], data.textureColorBuffers[0], data.RBOs[0]);
  std::vector<std::string> faces{
    "./images/skybox/right.jpg",
    "./images/skybox/left.jpg",
    "./images/skybox/top.jpg",
    "./images/skybox/bottom.jpg",
    "./images/skybox/front.jpg",
    "./images/skybox/back.jpg"
  };
  data.cubemap = loadCubemap(faces);
  data.skyboxVAO = createSkybox();

  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);

  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  // enter the glut event processing cycle
  data.previousTime = glutGet(GLUT_ELAPSED_TIME);
  glutMainLoop();
  
  return 0;
}

void render(void *data) {
  Data *d = static_cast<Data *>(data);
  
  if(d->wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // render to the framebuffer's texture
  glBindFramebuffer(GL_FRAMEBUFFER, d->framebuffers[0]);
  glEnable(GL_DEPTH_TEST);
  
  // clear the buffer
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderScene(d, d->SCENE);
  // renderScene(d, d->NORMALS_DEBUG);

  // bind the default framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // draw the framebuffer texture to a quad
  d->shaders[d->VIEW_QUAD]->use();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, d->textureColorBuffers[0]);
  d->shaders[d->VIEW_QUAD]->setInt("screenTexture", 0);
  d->quad.draw(d->shaders[d->VIEW_QUAD]);

  // show drawn buffer to screen
  glutSwapBuffers();
}

void renderScene(Data *d, int shaderIndex) {
  // get the view matrix
  glm::mat4 view = d->camera.getViewMatrix();

  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
  glm::vec3 viewPos = d->camera.getPosition();

  glDepthFunc(GL_LEQUAL);
  d->shaders[d->SKYBOX]->use();
  d->shaders[d->SKYBOX]->setMat4("projection", d->proj);
  d->shaders[d->SKYBOX]->setMat4("skyboxView", skyboxView);
  glBindVertexArray(d->skyboxVAO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, d->cubemap);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthFunc(GL_LESS);

  // set all the uniform variables for the object
  d->shaders[shaderIndex]->use();

  // glm::mat4 planeModel = glm::mat4(1.0f);
  // glm::mat3 planeNormalMat = glm::transpose(glm::inverse(glm::mat3(planeModel)));
  // d->shaders[d->SCENE]->setMat4("model", planeModel);
  // d->shaders[d->SCENE]->setMat3("normalMatrix", planeNormalMat);
  // d->plane.draw(d->shaders[d->SCENE]);

  glBindTexture(GL_TEXTURE_CUBE_MAP, d->cubemap);
  d->shaders[shaderIndex]->setVec3("viewPos", viewPos);
  glm::vec3 cubePositions[] = {
    glm::vec3(-1.0f, 0.0f, -1.0f),
    glm::vec3(4.0f, 0.0f, 0.0f)
  };
  
  // draw the objects
  for(unsigned int i=0;i<2;i++) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[i]);

    // calculate the normal matrix for correcting normal vector after any transformations
    glm::mat3 normalMatrix;
    if(shaderIndex == d->NORMALS_DEBUG) {
      normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
    } else {
      normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    }

    d->shaders[shaderIndex]->setMat4("model", model);
    d->shaders[shaderIndex]->setMat3("normalMatrix", normalMatrix);
    d->cube.draw(d->shaders[shaderIndex]);
  }

  // glm::mat4 model = glm::mat4(1.0f);
  // model = glm::translate(model, glm::vec3(0,-0.13f,0));
  // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  // model = glm::scale(model, glm::vec3(0.2f));
  // glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
  // d->shaders[d->SCENE]->setMat4("model", model);
  // d->shaders[d->SCENE]->setMat3("normalMatrix", normalMatrix);
  // d->backpack.draw(d->shaders[d->SCENE]);
}

void setupUBO(Data *d) {
  glGenBuffers(1, &d->matricesUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  // allocate memory to buffer
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // bind buffer to binding point 0
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, d->matricesUBO, 0, 2 * sizeof(glm::mat4));
}

void genFramebuffer(GLuint &framebuffer, GLuint &textureColorBuffer, GLuint &RBO) {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &textureColorBuffer);
  glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

  glGenRenderbuffers(1, &RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void setGlutCallbacks(Data *data) {
  glutDisplayFuncUcall(render, data);
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
  d->camera.flipDirection(false);

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

  glViewport(0, 0, w, h); // sets viewport to be entire window
  d->proj = glm::perspective(glm::radians(45.f), ratio, 0.1f, 100.0f); // sets the perspective
  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(d->proj));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void normalKeyDown(unsigned char key, int x, int y, void *data) {
  Data *d = static_cast<Data *>(data);

  int mod = glutGetModifiers();
  if(key == 27) {
    glutLeaveMainLoop();
  }

  if(key == 'r') {
    if(mod == GLUT_ACTIVE_ALT) {
      d->wireframe = true;
    } else {
      d->wireframe = false;
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
  d->shaders[d->SCENE] = new Shader("./shaders/object.vs", "./shaders/reflection.fs");
  d->shaders[d->VIEW_QUAD] = new Shader("./shaders/view_quad.vs", "./shaders/post_processing.fs");
  d->shaders[d->SKYBOX] = new Shader("./shaders/skybox.vs", "./shaders/skybox.fs");
  d->shaders[d->NORMALS_DEBUG] = new Shader("./shaders/debug_normals.vs", "./shaders/debug_normals.fs",
                                            "./shaders/debug_normals.gs");

  d->shaders[d->SCENE]->bindUniformBlock("Matrices", 0);
  d->shaders[d->SKYBOX]->bindUniformBlock("Matrices", 0);
  d->shaders[d->NORMALS_DEBUG]->bindUniformBlock("Matrices", 0);
}

void loadObjects(Data *d) {
  ObjLoader loader;
  loader.loadObj("./objects/plane/plane.obj", d->plane);
  loader.loadObj("./objects/cube/cube.obj", d->cube);
  loader.loadObj("./objects/quad/quad.obj", d->quad);
  // loader.loadObj("./objects/backpack/backpack.obj", d->backpack);
}

GLuint loadCubemap(std::vector<std::string> faces) {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for(unsigned int i=0; i<faces.size(); i++) {
    unsigned char *data = ImageLoader::loadImage(faces[i].c_str(), &width, &height, &nrChannels);
    if(data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      ImageLoader::freeImage(data);
    } else {
      printf("WARNING::IMAGE_LOADER: failed to load cubemap image at path {%s}\n", faces[i].c_str());
      ImageLoader::freeImage(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

GLuint createSkybox() {
  float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
  };

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW); 

  glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return VAO;
}