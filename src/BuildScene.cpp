/* Author: Philip Tyler
 * For: CPE471 @ Cal Poly SU
 * Instructor: Dr. Chris Buckalew
 *
 * The following code models and draws a simple toilet
 * using OpenGL primitives that have been scaled,
 * rotated and translated.
 *------------------------------------------------------------*/
 
#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// some function prototypes
void display(void);
void normalize(float[3]);
void normCrossProd(float[3], float[3], float[3]);

// initial viewer position
static GLdouble modelTrans[] = {0.0, 0.0, -5.0};
// initial model angle
static GLfloat theta[] = {0.0, 0.0, 0.0};
static float thetaIncr = 5.0;

// animation transform variables
static GLdouble translate[3] = {-10.0, 0.0, 0.0};

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
   // a directional light source from directly behind
   GLfloat lightDir[] = {0.0, 0.0, 5.0, 0.0};
   GLfloat diffuseComp[] = {1.0, 1.0, 1.0, 1.0};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseComp);

   return;
}

//--------------------------------------------------------
//  Set up the objects

/* Draws Bowl of toilet */
void drawCone() {

   // save the transformation state
   glPushMatrix();

      // set the material
      GLfloat diffuseColor[] = {1.0, 1.0, 1.0, 1.0};
      glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

      // locate it in the scene
      glMatrixMode(GL_MODELVIEW);
      // Adjust the translate and rotation to make the cone the right
      //    shape and in the right place
      glTranslatef(0,0,0);	 // x, y, and z
      glRotatef(90, 1, 0, 0);  // angle and axis (x, y, z components)
      // draw the cone - parameters are bottom radius, height, and number
      // of slices horizontally and radially
      glutSolidCone(4.0, 4.0, 10, 10);

   // recover the transform state
   glPopMatrix();

   return;
}

/* Draws flat sphere used at "water" and green sphere
 * used as the flush button */
void drawSphere() {
   glPushMatrix();

      // set the material
      GLfloat diffuseColor[] = {0, 0, 1.0, 1.0};
      glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

      // locate it in the scene
      glMatrixMode(GL_MODELVIEW);
      // Adjust the translate and rotation to make the water flat
      // and in the right place
      glTranslatef(0, 0.1, 0);	 // x, y, and z
      glScalef(1.0,0,1.0);
      glutSolidSphere(3.5,10,10);

   glPopMatrix();
   glPushMatrix();

      // set the material
      GLfloat greenColor[] = {0, 1.0, 0, 1.0};
      glMaterialfv(GL_FRONT, GL_DIFFUSE, greenColor);

      // locate it in the scene
      glMatrixMode(GL_MODELVIEW);
      // Adjust the translate and rotation to make the flush button
      // to the right shape and in the right place
      glTranslatef(-2.5, 3.5, -5);	 // x, y, and z
      glutSolidSphere(.5,5,5);

   glPopMatrix();

   return;
}

/* Draws both boxes used in Toilet Model */
void drawBox() {
   glPushMatrix();

      // set the material
      GLfloat diffuseColor[] = {1.0, 1.0, 1.0, 1.0};
      glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

      // locate it in the scene
      glMatrixMode(GL_MODELVIEW);
      // Adjust the translate and rotation to make the upper box
      // to the right shape and in the right place
      glTranslatef(0, 2, -7);
      glScalef(8, 6, 4);	 // x, y, and z
      glutSolidCube(1.0);

   glPopMatrix();

   glPushMatrix();

      glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

      // locate it in the scene
      glMatrixMode(GL_MODELVIEW);
      // Adjust the translate and rotation to make the lower box
      // to the right shape and in the right place
      glTranslatef(0, -3, -4);
      glScalef(2, 4, 10);	 // x, y, and z
      glutSolidCube(1.0);

   glPopMatrix();
   
   return;
}

/* Draws seat of Toilet */
void drawTorus() {
   glPushMatrix();

   // set the material
   GLfloat diffuseColor[] = {1.0, 1.0, 1.0, 1.0};
   glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // Adjust the translate and rotation to make the torus the right
   //    shape and in the right place
   glTranslatef(0, 0.2, 0);
   glScalef(0.6,0.1,0.6);
   glRotatef(90,-1,0,0);
   glutSolidTorus(1.2, 6, 16, 10);

   glPopMatrix();

   return;
}

/* Draws all parts of toilet, used in display() funtion */
void drawToilet() {
   drawCone();
   drawSphere();
   drawTorus();
   drawBox();
}

//-----------------------------------------------------------
//  Callback functions

void reshapeCallback(int w, int h) {
   // from Angel, p.562

   glViewport(0,0,w,h);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w < h) {
      glFrustum(-2.0, 2.0, -2.0*(GLfloat) h / (GLfloat) w,
                2.0*(GLfloat) h / (GLfloat) w, 2.0, 200.0);
   }
   else {
      glFrustum(-2.0, 2.0, -2.0*(GLfloat) w / (GLfloat) h,
                2.0*(GLfloat) w / (GLfloat) h, 2.0, 200.0);
   }

   glMatrixMode(GL_MODELVIEW);
}

void mouseCallback(int button, int state, int x, int y) {
   // rotate camera
   GLint axis = 3;
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) axis = 0;
   if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) axis = 1;
   if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) axis = 2;
   if (axis < 3) {  // button ups won't change axis value from 3
      theta[axis] += thetaIncr;
      if (theta[axis] > 360.0) theta[axis] -= 360.0;
      display();
   }
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
   // this code executes whenever the window is redrawn (when opened,
   //   moved, resized, etc.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // set the viewing transform
   setUpView();

   // set up light source
   setUpLight();

   // start drawing objects
   setUpModelTransform();
   drawToilet();
   glutSwapBuffers();
}

// create a double buffered 500x500 pixel color window
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Building a Scene: Lab 2");
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
   glutReshapeFunc(reshapeCallback);
   glutKeyboardFunc(keyCallback);
   glutMouseFunc(mouseCallback);
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

