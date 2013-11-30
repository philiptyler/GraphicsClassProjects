/*
 *  Lab exercise 6.0 
 *  CPE 471, Computer Graphics
 *  Copyright 2005 Chris Buckalew
 *
/*--------------------------------------------------------------
 *  This code contains examples of using two different types
 *  of lights in OpenGL.  The usual setUpLight() function call
 *  has been commented out, and the two lights in the scene are
 *  activated in the drawScene() function.
 *  Arrange so that there are two spotlights on the right side,
 *  at the same location, both pointing up and a little off to
 *  the side at an angle.  One spotlight should be very narrow 
 *  and the other larger.  One should be white and the other green.
 *  Why is the spotlight jagged ("jaggies" is a technical term
 *  in CG)?  Fix it.
 *  You'll need to look in the OpenGL documentation to get the
 *  syntax for a spotlight.
 *
 *  Place four point lights near the front of the sphere, all different
 *  colors, one near each quadrant.  
 *
 *  Change the directional light so it shines on the bottom of
 *  the sphere and make it red.
 *
 *------------------------------------------------------------
 */
#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
//#include "GLSL_helper.h"
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
   // a directional light source to serve as a headlight
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

void drawSphere() {

   // save the transformation state
   glPushMatrix();

   // set the material
   GLfloat diffuseColor[] = {1.0, 1.0, 1.0, 1.0};
   glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // note that center of cube is at origin
   glTranslatef(0, 0, 0);	 

   // draw the sphere - the parameters are the radius, slices, and stacks
   glutSolidSphere(5.0, 100, 100);   // lots of polygons

   // recover the transform state
   glPopMatrix();

   return;
}

void drawScene() {

   // light positions can be controlled with transforms just like objects
   glPushMatrix();

      // here's a white directional light
      // the last 0.0 in the lightDir vector indicates directional light
      GLfloat lightDir[] = {0.0, 0.0, 5.0, 0.0};
      GLfloat diffuseComp[] = {1.0, 1.0, 1.0, 1.0};

      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);

      glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseComp);

      // here's a red point light
      // the last 1.0 in the lightDir vector indicates point source
      GLfloat lightLoc[] = {5.0, 5.0, 5.0, 1.0};
      GLfloat pointDiffuse[] = {1.0, 0.0, 0.0, 1.0};

      glEnable(GL_LIGHT1);

      glLightfv(GL_LIGHT1, GL_POSITION, lightLoc);
      glLightfv(GL_LIGHT1, GL_DIFFUSE, pointDiffuse);


      drawSphere();
   glPopMatrix();    

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
   //setUpLight();

   // start drawing objects
   setUpModelTransform();
   drawScene();

   glutSwapBuffers();
}

// create a double buffered 500x500 pixel color window
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Lights: Lab 6");
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
