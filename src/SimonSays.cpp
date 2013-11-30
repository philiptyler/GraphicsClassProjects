/*
 *  Lab exercise 8.0 
 *  CPE 471, Computer Graphics
 *  Copyright 2005 Chris Buckalew
 *
 *  This code does simple selection on the 3-box computer
 *  we built in the first lab.  When you click on one of 
 *  the boxes, the console window tells you which one you 
 *  clicked on.  What happens when you rotate the scene so
 *  that you can pick two boxes at once?  
 *
 *  Here's what I want you to add:
 *
 *  1) When a box is selected, allow the user to enter a
 *     color for the box in the console window.  The box
 *     will then be drawn using that color.
 *
 *  2) Fix the code so that the nearest object is colored.
 *
 *  3) Add a sphere somewhere else in the scene and allow
 *     the sphere to also be selected, and its color
 *     changed in the same manner as the boxes.
 *
 *  Note that there are lots of ways for this code to go bad.
 *  Basically it works by "rendering" the entire scene for
 *  the purposes of selection - this occurs in the mouse
 *  callback.  In the process of "rendering", it has to create
 *  all the transform hierarchy and "draw" all the objects just
 *  as if it were actually rendering.  (There is one single 
 *  transform that is different.)  This is why the projection,
 *  viewing, and modeling transforms are all built up within
 *  selection.  Instead of drawing, all selection does is find
 *  the "names" of objects - integers that are assigned to the
 *  objects with the glLoadName() function.  To keep things
 *  as synchronized as possible, the same drawing functions are
 *  used with a special flag, GL_SELECT, that turns the name
 *  loading on.
 *     
 */

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
//#include "GLSL_helper.h"
#include <math.h>

#define MAX_SPHERES 7
#define SPHERE_RAD 7.0
#define BUFSIZE 512
#define MAX_PATTERN_LEN 100
#define SECOND 1000000
#define HALF_SECOND 500000

// some function prototypes
void display(void);
void normalize(float[3]);
void normCrossProd(float[3], float[3], float[3]);
void initSpheres(void);
void showPattern(int i);
void generatePattern(void);
void startGame(void);
void drawSpheres(GLenum mode);
void lightSphere(int i);
void unlightSphere(int i);
void lightAllSpheres(int numTimes);

struct sphereData {
   GLfloat color[4];
   GLfloat litColor[4];
   short lit;
};

// initial viewer position
static GLdouble modelTrans[] = {0.0, 0.0, -5.0};
// initial model angle
static GLfloat theta[] = {0.0, 0.0, 0.0};
static float thetaIncr = 5.0;

// animation transform variables
static GLdouble translate[3] = {-10.0, 0.0, 0.0};

static GLint numSpheres;
GLfloat angle;
static struct sphereData spheres[MAX_SPHERES];

static int pattern[MAX_PATTERN_LEN];
static int patternLen = 3;
static int patternIndex = 0;
static int allowHitProc = 1;
static int youLose = 0;
static int startedGuessing = 0;

void initSpheres() {
   int i, j;
   
   for (i = 0; i < MAX_SPHERES; i++) {
      spheres[i].lit = 0;
      for (j = 0; j < 3; j++) {
         spheres[i].color[j] = 0;
         spheres[i].litColor[j] = 0;
      }
      spheres[i].color[j] = 1.0;
      spheres[i].litColor[j] = 1.0;
   }
   
   spheres[0].color[0] = 0.5;                            // red
   spheres[1].color[0] = 0.5; spheres[1].color[1] = 0.5; // yellow
   spheres[2].color[1] = 0.5;                            // green
   spheres[3].color[2] = 0.5;                            // blue
   spheres[4].color[0] = 0.5; spheres[4].color[1] = 0.3; // orange
   spheres[5].color[1] = 0.5; spheres[5].color[2] = 0.5; // turquoise
   spheres[6].color[0] = 0.5; spheres[6].color[2] = 0.5; // purple
   
   spheres[0].litColor[0] = 1.0;                            // red
   spheres[1].litColor[0] = 1.0; spheres[1].litColor[1] = 1.0; // yellow
   spheres[2].litColor[1] = 1.0;                            // green
   spheres[3].litColor[2] = 1.0;                            // blue
   spheres[4].litColor[0] = 1.0; spheres[4].litColor[1] = 0.6; // orange
   spheres[5].litColor[1] = 1.0; spheres[5].litColor[2] = 1.0; // turquoise
   spheres[6].litColor[0] = 1.0; spheres[6].litColor[2] = 1.0; // purple
}

void showPattern(int i) {
   if (i == patternLen) {
      allowHitProc = 1;
      return;
   }
   lightSphere(pattern[i++]);
   glutTimerFunc(1000, showPattern, i);
}

void generatePattern() {
   for (int i = 0; i < patternLen; i++) {
      pattern[i] = rand() % numSpheres;
   }
}

void beginPatternSequence() {
   patternIndex = 0;
   allowHitProc = 0;
   patternLen++;
   startedGuessing = 1;
   generatePattern();
   lightAllSpheres(3);
}

void startGame() {
   printf("+==========================================+\n");
   printf("|  WELCOME TO SIMON SAYS!! OPENGL EDITION  |\n");
   printf("+==========================================+\n");
   
   printf("Enter number of spheres to play with: ");
   scanf("%d", &numSpheres);
   if (numSpheres <= 2 || numSpheres > MAX_SPHERES) {
      printf("invalid sphere number, defaulting to 4...\n\n");
      numSpheres = 4;
   }
   angle  = 360.0 / numSpheres;
   
   unsigned int seconds;
   seconds = (unsigned int)time(NULL);
   srand(seconds);
   
   initSpheres();
   startedGuessing = 0;
   patternLen = 3;
}

//---------------------------------------------------------
//   Set up the view

void setUpView() {
   // this code initializes the viewing transform
   glLoadIdentity();
   
   // moves viewer along coordinate axes
   gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
   
   // move the view back some relative to viewer[] position
   glTranslatef(0.0f,0.0f, -8.0f);
   
   // rotates view
   glRotatef(0, 1.0, 0.0, 0.0);
   glRotatef(0, 0.0, 1.0, 0.0);
   glRotatef(0, 0.0, 0.0, 1.0);
   
   return;
}

//---------------------------------------------------------
//   Set up the projection

void setUpProjection(int w, int h) {
   // this code initializes the projection transform
   glViewport(0,0,w,h);
   
   if (w < h) {
      glFrustum(-2.0, 2.0, -2.0*(GLfloat) h / (GLfloat) w,
                2.0*(GLfloat) h / (GLfloat) w, 2.0, 200.0);
   }
   else {
      glFrustum(-2.0, 2.0, -2.0*(GLfloat) w / (GLfloat) h,
                2.0*(GLfloat) w / (GLfloat) h, 2.0, 200.0);
   }
   
   glMatrixMode(GL_MODELVIEW);
   
   return;
}

//----------------------------------------------------------
//  Set up model transform

void setUpModelTransform() {
   
   // moves model along coordinate axes
   glTranslatef(modelTrans[0], modelTrans[1], modelTrans[2]);
   
   // rotates model
   glRotatef(theta[0], 1.0, 0.0, 0.0);
   glRotatef(theta[1], 0.0, 1.0, 0.0);
   glRotatef(theta[2], 0.0, 0.0, 1.0);
   
   
}

//----------------------------------------------------------
//  Set up the light

void setUpLight() {
   // set up the light sources for the scene
   // a directional light source from over the right shoulder
   GLfloat lightDir1[] = {2.0, 0.0, 0, 0.0};
   GLfloat lightDir2[] = {-2.0, 0.0, 0, 0.0};
   GLfloat lightDir3[] = {0.0, 2.0, 0, 0.0};
   GLfloat lightDir4[] = {0.0, -2.0, 0, 0.0};
   GLfloat frontLight[] = {0.0, 0.0, 5.0, 0.0};
   GLfloat diffuseComp[] = {1.0, 1.0, 1.0, 1.0};
   
   
   glEnable(GL_LIGHTING);
   
   if (youLose) {
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0, GL_POSITION, frontLight);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseComp);
      return;
   }
   
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, lightDir1);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseComp);
   
   glEnable(GL_LIGHT1);
   glLightfv(GL_LIGHT1, GL_POSITION, lightDir2);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseComp);
   
   glEnable(GL_LIGHT2);
   glLightfv(GL_LIGHT2, GL_POSITION, lightDir3);
   glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseComp);
   
   glEnable(GL_LIGHT3);
   glLightfv(GL_LIGHT3, GL_POSITION, lightDir4);
   glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuseComp);
   
   return;
}

//--------------------------------------------------------
//  Set up the objects

void drawSphere(GLenum mode, int i) {
   glPushMatrix();
   if (mode == GL_SELECT) glLoadName(i+1);
   if (spheres[i].lit)
      glMaterialfv(GL_FRONT, GL_DIFFUSE, spheres[i].litColor);
   else
      glMaterialfv(GL_FRONT, GL_DIFFUSE, spheres[i].color);
   glMatrixMode(GL_MODELVIEW);
   glRotatef(angle * i, 0, 0, 1.0);
   glTranslatef(numSpheres, 0, 0);
   glutSolidSphere(SPHERE_RAD, 20, 20);
   glPopMatrix();
}

void drawX(int angle) {
   GLfloat red[4] = {1.0, 0, 0, 1.0};
   glPushMatrix();
   glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
   glMatrixMode(GL_MODELVIEW);
   glRotatef((angle+=1), 0.0, 1.0, 0.0);
   
      glPushMatrix();
      glRotatef(45, 0, 0, 1.0);
      glScalef(16.0, 2.0, 2.0);
      glutSolidCube(2.0);
      glPopMatrix();
   
      glPushMatrix();
      glRotatef(135, 0, 0, 1.0);
      glScalef(16.0, 2.0, 2.0);
      glutSolidCube(2.0);
      glPopMatrix();
      
      glTranslatef(numSpheres, 0, 0);
   glPopMatrix();
   if (youLose) glutTimerFunc(100, drawX, angle);
}

void drawSpheres(GLenum mode) {
   
   for (int i = 0; i < numSpheres; i++) {
      drawSphere(mode, i);
   }
   
   return;
}

void unlightSphere(int i) {
   spheres[i].lit = 0;
   display();
}

void lightSphere(int i) {
   spheres[i].lit = 1;
   display();
   glutTimerFunc(500, unlightSphere, i);
}

void lightAllSpheres(int numTimes) {
   if (numTimes == 0) glutTimerFunc(800, showPattern, 0);
   else {
      for (int i = 0; i < numSpheres; i++) {
         lightSphere(i);
      }
      glutTimerFunc(1000, lightAllSpheres, numTimes-1);
   }
}

void processHits(GLint hits, GLuint buffer[])
{
   // this function goes through the selection hit list of object names
   
   int i, j;
   GLuint *ptr, minZ, names, closestItem;
   
   if (hits==0) return;
   
   ptr = (GLuint *) buffer;
   minZ = 0xffffffff;
   closestItem = 0;
   
   for(i = 0; i < hits; i++) {
      names = *ptr;
      ptr++;
      if (*ptr < minZ) {
         minZ = *ptr;
         closestItem = *(ptr+2);
      }
      ptr += names+2;
   }
   
   lightSphere(closestItem-1);
   if ((closestItem - 1) != pattern[patternIndex++]) {
      youLose = 1;
      printf("\nYOU LOSE\n\n");
   }
   else if (patternIndex == patternLen) {
      beginPatternSequence();
      printf("\nYOU WIN ROUND %d\n\n", patternLen-4);
   }
   
   return;
}

//-----------------------------------------------------------
//  Callback functions

void reshapeCallback(int w, int h) {
   
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   setUpProjection(w, h);
}

void mouseCallback(int button, int state, int x, int y) {
   
   int w, h;
   
   if ((button == GLUT_LEFT_BUTTON)&&(state == GLUT_DOWN)) {
      
      if (startedGuessing == 0) beginPatternSequence();
      // here is where selection occurs
      GLuint selectBuf[BUFSIZE];
      GLint hits;
      GLint viewport[4];
      
      glGetIntegerv(GL_VIEWPORT, viewport);
      
      glSelectBuffer(BUFSIZE, selectBuf);
      (void) glRenderMode(GL_SELECT);
      
      glInitNames();
      glPushName(0);
      
      glPushMatrix();
      // now "render" everything for selection
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      // this is the only transform difference in the hierarchy
      gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y),
                    5.0, 5.0, viewport);
      w = viewport[2];
      h = viewport[3];
      setUpProjection(w, h);
      
      // the rest is the same as in display(), except the 
      // mode flag is different (GL_SELECT)
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      setUpView();
      setUpModelTransform();
      drawSpheres(GL_SELECT);
      glPopMatrix();
      
      glFlush();
      
      hits = glRenderMode(GL_RENDER);
      if (allowHitProc) processHits(hits,selectBuf);
      
      // re-render everything again for display
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      setUpProjection(w, h);
   }
   
   display();
}   

void keyCallback(unsigned char key, int x, int y) {
   // move viewer with x, y, and z keys
   // capital moves in + direction, lower-case - direction
   if (key == 'x') modelTrans[0] -= 1.0;
   if (key == 'X') modelTrans[0] += 1.0;
   if (key == 'y') modelTrans[1] -= 1.0;
   if (key == 'Y') modelTrans[1] += 1.0;
   if (key == 'z') modelTrans[2] -= 1.0;
   if (key == 'Z') modelTrans[2] += 1.0;
   if (key == 'r') {
      theta[0] = 0.0; theta[1] = 0.0; theta[2] = 0.0;
   }
   if (key == '-') {
      thetaIncr = -thetaIncr;
   }
   if (key == '+') {
      if (thetaIncr < 0) thetaIncr = thetaIncr - 1.0;
      else               thetaIncr = thetaIncr + 1.0;
   }
   
   display();
}


//---------------------------------------------------------
//  Main routines

void display (void) {
   char userInput[100];
   // this code executes whenever the window is redrawn (when opened,
   //   moved, resized, etc.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   // set the viewing transform
   setUpView();
   
   // set up light source
   setUpLight();
   
   // start drawing objects
   setUpModelTransform();
   if (youLose){
      drawX(0);
      glutSwapBuffers();
      printf("Would you like to play again? y/n: ");
      scanf("%s", userInput);
      youLose = 0;
      setUpLight();
      if (userInput[0] == 'y') startGame();
      else exit(0);
   }
   else drawSpheres(GL_RENDER);
   
   glutSwapBuffers();
}

// create a double buffered 500x500 pixel color window
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simon Says");
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
   glutReshapeFunc(reshapeCallback);
   glutKeyboardFunc(keyCallback);
   glutMouseFunc(mouseCallback);
   startGame();
	glutMainLoop();
	return 0;
}

//---------------------------------------------------------
//  Utility functions

void normalize(float v[3]) {
   // normalize v[] and return the result in v[]
   // from OpenGL Programming Guide, p. 58
   GLfloat d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
   if (d == 0.0) {
      printf("zero length vector");
      return;
   }
   v[0] = v[0]/d; v[1] = v[1]/d; v[2] = v[2]/d;
}

void normCrossProd(float v1[3], float v2[3], float out[3]) {
   // cross v1[] and v2[] and return the result in out[]
   // from OpenGL Programming Guide, p. 58
   out[0] = v1[1]*v2[2] - v1[2]*v2[1];
   out[1] = v1[2]*v2[0] - v1[0]*v2[2];
   out[2] = v1[0]*v2[1] - v1[1]*v2[0];
   normalize(out);
}


