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

#define INVALID_VAL -1024
// some function prototypes
void display(void);
void normalize(float[3]);
void normCrossProd(float[3], float[3], float[3]);
GLfloat yMidRand(GLfloat, GLfloat, GLfloat);
void findNormal(GLfloat[3], GLfloat[3], GLfloat[3], GLfloat[3]);
void findHighestY(void);

// initial viewer position
static GLdouble modelTrans[] = {-0.5, 0.0, 10.0};
// initial model angle
static GLfloat theta[] = {30.0, 0.0, 0.0};
static float thetaIncr = 5.0;

// animation transform variables
static GLdouble translate[3] = {-10.0, 0.0, 0.0};

static GLfloat *yVals;
static GLfloat ruggedness = 0.25;
static int itDepth = 6;
static int arraySize = 0;
static int totArraySize = 0;
static GLfloat highestY = 0;
//---------------------------------------------------------
//   Set up the view

void setUpView() {
   // this code initializes the viewing transform
   glLoadIdentity();

   // moves viewer along coordinate axes
   gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

   // move the view back some relative to viewer[] position
   glTranslatef(0.0f,0.0f, -12.0f);

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
   // a directional light source from over the right shoulder
   GLfloat lightDir[] = {0.0, 5.0, 1.0, 0.0};
   GLfloat diffuseComp[] = {1.0, 1.0, 1.0, 1.0};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseComp);

   return;
}

//--------------------------------------------------------
//  Set up the objects

void recursiveMount(GLfloat x1, GLfloat x2, GLfloat z1,
                    GLfloat z2, int itLvl) {
   // Find corresponding yVals array indicies to store
   // new y values.  Based on x and z coordinate.
   int row1 = floor(x1*arraySize+0.5)*(arraySize+1);
   int row2 = floor(x2*arraySize+0.5)*(arraySize+1);
   int col1 = floor(z1*arraySize+0.5);
   int col2 = floor(z2*arraySize+0.5);
   
   // Base case
   if (itLvl > itDepth) return;

   // Find midpoint yVals indicies
   int colOff = pow(2,itDepth-itLvl);
   int rowOff = (arraySize+1)*colOff;

   // midpoint y x z values
   GLfloat mid1, mid2, mid3, mid4;
   GLfloat midx = (x1+x2)/2.0;
   GLfloat midz = (z1+z2)/2.0;

   // generate y values for midpoints with degenerated ruggedness
   GLfloat rug = ruggedness / pow(2,itLvl-1);
   if (yVals[row1+col1+colOff] == INVALID_VAL)
      yVals[row1+col1+colOff] = mid1 = yMidRand(yVals[row1+col1],yVals[row1+col2],rug);
   else mid1 = yVals[row1+col1+colOff]; 
   if (yVals[row1+rowOff+col1] == INVALID_VAL)
      yVals[row1+rowOff+col1] = mid2 = yMidRand(yVals[row1+col1],yVals[row2+col1],rug);
   else mid2 = yVals[row1+rowOff+col1];
   if (yVals[row2+col1+colOff] == INVALID_VAL)
      yVals[row2+col1+colOff] = mid3 = yMidRand(yVals[row2+col1],yVals[row2+col2],rug);
   else mid3 = yVals[row2+col1+colOff];
   if (yVals[row1+rowOff+col2] == INVALID_VAL)
      yVals[row1+rowOff+col2] = mid4 = yMidRand(yVals[row1+col2],yVals[row2+col2],rug);
   else mid4 = yVals[row1+rowOff+col2];

   // generate center point y value
   mid1 = (mid1+mid2+mid3+mid4)/4.0;
   yVals[row1+rowOff+col1+colOff] = yMidRand(mid1,mid1,rug);

   // recurse into 4 new polygons with generated midpoints at verticies
   itLvl++;
   recursiveMount(x1, midx, z1, midz, itLvl);
   recursiveMount(midx, x2, z1, midz, itLvl);
   recursiveMount(x1, midx, midz, z2, itLvl);
   recursiveMount(midx, x2, midz, z2, itLvl);
}

void drawMounts() {
   GLfloat x1, x2, z, norm[3], vertInc = 1.0 / arraySize;
   // temporary verticy storage (for calculating normals)
   GLfloat lastx1Vert[3], lastx2Vert[3], newx1Vert[3], newx2Vert[3];

   // colors for drawing
   glPushMatrix();
   GLfloat color[] = {0.8,0.4,0,1.0};
   GLfloat lakeColor[] = {0, 0.2, 0.8, 1.0};
   GLfloat snowColor[] = {1.0, 1.0, 1.0, 1.0};
   glMaterialfv(GL_FRONT, GL_DIFFUSE, color);

   // find highest Y value generated, base snow caps on this value
   findHighestY();
   highestY = 0.6*highestY;

   // replace any negative y values with 0 for lakes
   for (int h = 0; h < totArraySize; h++) if (yVals[h] < 0) yVals[h] = 0;

   // generate quad strips along z axis
   for (int i = 0; i < arraySize; i++){
      x1 = i*vertInc;
      x2 = (i+1)*vertInc;
      z = 0.0;

      glBegin(GL_QUAD_STRIP);
      lastx1Vert[0]=x1; lastx1Vert[1]=yVals[i*(arraySize+1)]; lastx1Vert[2]=z;
      lastx2Vert[0]=x2; lastx2Vert[1]=yVals[(i+1)*(arraySize+1)]; lastx2Vert[2]=z;

      for (int j = 0; j <= arraySize; j++) {
         z += vertInc;
         newx1Vert[0]=x1; newx1Vert[1]=yVals[i*(arraySize+1)+j]; newx1Vert[2]=z;
         newx2Vert[0]=x2; newx2Vert[1]=yVals[(i+1)*(arraySize+1)+j]; newx2Vert[2]=z;

         // if the y value is zero, color the verticy like a lake         
         if (lastx1Vert[1] <= 0) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, lakeColor);
         }
         // if the y value is above a calculated threshhold, color white
         else if (lastx1Vert[1] >= highestY)
            glMaterialfv(GL_FRONT, GL_DIFFUSE, snowColor);
         // otherwise color as rocks
         else glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
         findNormal(lastx1Vert, newx1Vert, lastx2Vert, norm);
         glNormal3fv(norm);
         glVertex3fv(lastx1Vert);

         // repeat for other verticy
         if (lastx2Vert[1] <= 0) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, lakeColor);
         }
         else if (lastx2Vert[1] >= highestY)
            glMaterialfv(GL_FRONT, GL_DIFFUSE, snowColor);
         else glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
         findNormal(lastx2Vert, lastx1Vert, newx2Vert, norm);
         glNormal3fv(norm);
         glVertex3fv(lastx2Vert);

         lastx1Vert[0]=newx1Vert[0]; lastx1Vert[1]=newx1Vert[1]; lastx1Vert[2]=newx1Vert[2];
         lastx2Vert[0]=newx2Vert[0]; lastx2Vert[1]=newx2Vert[1]; lastx2Vert[2]=newx2Vert[2];
      }
      glNormal3fv(norm);
      glVertex3fv(lastx1Vert);
      glNormal3fv(norm);
      glVertex3fv(lastx2Vert);

      glEnd();
   }
   glPopMatrix();
} 

void makeMounts() {
   // funtion that calls recursive y-generating fucntions
   arraySize = pow(2,itDepth);
   totArraySize = (arraySize+1)*(arraySize+1);
   // initialize yVals array, fill with very low value
   yVals = (GLfloat *)malloc(totArraySize*sizeof(GLfloat));
   for (int i = 0; i < totArraySize; i++) yVals[i] = INVALID_VAL;
   
   // initialize corners of array with y values.
   yVals[0] = yMidRand(0,0,ruggedness);
   yVals[arraySize] = yMidRand(0,0,ruggedness);
   yVals[(arraySize+1)*arraySize] = yMidRand(0,0,ruggedness);
   yVals[(arraySize+1)*arraySize+arraySize] = yMidRand(0,0,ruggedness);
   
   // call recursive function to fill array
   recursiveMount(0,1.0,0,1.0,1);
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
   if (key == 'k') {
      ruggedness = ruggedness * 0.5;
      makeMounts();
   }
   if (key == 'l') {
      ruggedness = ruggedness * 2;
      makeMounts();
   }
   if (key == 'h') {
      itDepth--;
      makeMounts();
   }
   if (key == 'j') {
      itDepth++;
      makeMounts();
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

   drawMounts();

   glutSwapBuffers();
}

// create a double buffered 500x500 pixel color window
int main(int argc, char* argv[]) {
	makeMounts();
   glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 900);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Fractal Landscapes");
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

void findNormal(GLfloat v[3], GLfloat v1[3], GLfloat v2[3], GLfloat out[3]) {
   if (v[1] == 0) {
      out[0] = 0; out[1] = 1.0; out[2] = 0;
      return;
   }
   GLfloat vect1[3] = {v1[0]-v[0], v1[1]-v[1], v1[2]-v[2]};
   GLfloat vect2[3] = {v2[0]-v[0], v2[1]-v[1], v2[2]-v[2]};
   normCrossProd(vect1, vect2, out);
}

void findHighestY() {
   highestY = 0;
   for (int i = 0; i < totArraySize; i++) {
     if (yVals[i] > highestY) highestY = yVals[i]; 
   }
}

GLfloat yMidRand(GLfloat y1, GLfloat y2, GLfloat r) {
   return ((y1+y2)/2.0) + ((((float)(rand())/RAND_MAX*2)-1.0)*r);
}
