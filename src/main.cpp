#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <map>
#include <vector>
#include <imageLoader.h>

#include <openglMaths.h>
#include <model.h>
#include <objLoader.h>
#include <light_structs.h>
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

// creates the skybox VAO
GLuint createSkybox();

// sets up the uniform buffers for passing variables to multiple shaders
void setupUBO(Data *d);

// calculate the normal matrix for correcting normal vector after any transformations
oglm::mat3 calcNormalMatrix(oglm::mat4 model, oglm::mat4 view, bool debugNormals);

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

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  

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
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
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
  oglm::mat4 view = d->camera.getViewMatrix();

  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(oglm::mat4), sizeof(oglm::mat4), &view.columns[0].x);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  oglm::mat4 skyboxView = oglm::mat4(oglm::mat3(view));
  oglm::vec3 viewPos = d->camera.getPosition();
  bool debugNormals = (shaderIndex == d->NORMALS_DEBUG);

  // set all the uniform variables for the object
  d->shaders[shaderIndex]->use();
  d->shaders[shaderIndex]->setVec3("viewPos", viewPos);

  // draws the plane
  oglm::mat4 planeModel = oglm::mat4(1.0f);
  oglm::mat3 planeNormalMat = calcNormalMatrix(planeModel, view, debugNormals);
  d->shaders[shaderIndex]->setMat4("model", planeModel);
  d->shaders[shaderIndex]->setMat3("normalMatrix", planeNormalMat);
  d->plane.draw(d->shaders[shaderIndex]);  
  
  // draws the cubes
  oglm::mat4 cubeModel = oglm::mat4(1.0f);
  oglm::mat3 normalMatrix = calcNormalMatrix(cubeModel, view, debugNormals);
  d->shaders[shaderIndex]->setMat4("model", cubeModel);
  d->shaders[shaderIndex]->setMat3("normalMatrix", normalMatrix);

  // draw the objects
  d->cube.drawInstanced(d->shaders[shaderIndex], 2000);

  // backpack transformations
  oglm::mat4 backpackModel = oglm::mat4(1.0f);
  backpackModel = oglm::translate(backpackModel, oglm::vec3(0,-0.13f,0));
  backpackModel = oglm::rotate(backpackModel, oglm::radians(90.0f), oglm::vec3(0.0f, 1.0f, 0.0f));
  backpackModel = oglm::scale(backpackModel, oglm::vec3(0.2f));

  // draws the backpack
  oglm::mat3 bpNormalMatrix = calcNormalMatrix(backpackModel, view, debugNormals);
  d->shaders[shaderIndex]->setMat4("model", backpackModel);
  d->shaders[shaderIndex]->setMat3("normalMatrix", bpNormalMatrix);
  d->backpack.draw(d->shaders[shaderIndex]);

  // draws the skybox
  glDepthFunc(GL_LEQUAL);
  d->shaders[d->SKYBOX]->use();
  d->shaders[d->SKYBOX]->setMat4("projection", d->proj);
  d->shaders[d->SKYBOX]->setMat4("skyboxView", skyboxView);
  glBindVertexArray(d->skyboxVAO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, d->cubemap);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthFunc(GL_LESS);
}

void setupUBO(Data *d) {
  glGenBuffers(1, &d->matricesUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  // allocate memory to buffer
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(oglm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // bind buffer to binding point 0
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, d->matricesUBO, 0, 2 * sizeof(oglm::mat4));
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
  glutCreateWindow("3D Scene");

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
  d->mouseChange = oglm::vec2(0,0);

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
  d->proj = oglm::perspective(oglm::radians(45.f), ratio, 0.1f, 100.0f); // sets the perspective
  glBindBuffer(GL_UNIFORM_BUFFER, d->matricesUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(oglm::mat4), &d->proj.columns[0].x);
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
    d->mouseChange = oglm::vec2(x - d->screenWidth/2, y - d->screenHeight/2);

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
  d->shaders[d->SCENE] = new Shader("./shaders/instanced.vs", "./shaders/simple.fs");
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
  loader.loadObj("./objects/backpack/backpack.obj", d->backpack);

  unsigned int amount = 2000;
  oglm::vec3 cubePositions[amount];
  float divisor = 100/(2*M_PI);
  float multiplier = 3 + divisor;
  for (int j = 0; j < 20; j++) {
    for (unsigned int i = 0; i < 100; i++) {
      cubePositions[i+(100*j)] = oglm::vec3(multiplier * sin(i/divisor), j-10, multiplier * cos(i/divisor));
    }
  }
  
  d->cube.enableInstancing(cubePositions, amount);
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

oglm::mat3 calcNormalMatrix(oglm::mat4 model, oglm::mat4 view, bool debugNormals) {
  oglm::mat3 normalMatrix;

  if(debugNormals) {
    normalMatrix = oglm::transpose(oglm::inverse(oglm::mat3(model * view)));
  } else {
    normalMatrix = oglm::transpose(oglm::inverse(oglm::mat3(model)));
  }
  return normalMatrix;
}