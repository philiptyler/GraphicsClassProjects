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
 *

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
static GLfloat cpuColor[] = {1.0, 0.8, 0.8, 1.0};
static GLfloat keyboardColor[] = {0.8, 1.0, 0.8, 1.0};
static GLfloat monitorColor[] = {0.8, 0.8, 1.0, 1.0};
static GLfloat sphereColor[] = {1.0, 0, 0, 1.0};

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

void drawCPU(GLenum mode) {  // mode turns selection on or off

   // this models the CPU
   // use the GLUT cube primitive and scale it
   
   // save the transformation state
   glPushMatrix();

   // if doing a SELECT "render", here is the name for
   // this object (the integer 1)
   if (mode == GL_SELECT) glLoadName(1);

   // set the material
   glMaterialfv(GL_FRONT, GL_DIFFUSE, cpuColor);

   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // Adjust the translate and scale to make the monitor the right
   //    shape and in the right place
   glTranslatef(-4, 1, 0);	 
   glScalef(0.5, 1.0, 1.0);  

   // draw the cube - the parameter is the length of the sides
   glutSolidCube(5.0);

   // recover the transform state
   glPopMatrix();

   return;
}

void drawSphere(GLenum mode) {  // mode turns selection on or off
   
   // this models the CPU
   // use the GLUT cube primitive and scale it
   
   // save the transformation state
   glPushMatrix();
   
   // if doing a SELECT "render", here is the name for
   // this object (the integer 1)
   if (mode == GL_SELECT) glLoadName(4);
   
   // set the material
   glMaterialfv(GL_FRONT, GL_DIFFUSE, sphereColor);
   
   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // Adjust the translate and scale to make the monitor the right
   //    shape and in the right place
   glTranslatef(2, 1, 3); 
   
   // draw the cube - the parameter is the length of the sides
   glutSolidSphere(2.0, 20, 20);
   
   // recover the transform state
   glPopMatrix();
   
   return;
}

void drawKeyboard(GLenum mode) {

   // this models the keyboard
   // use individual rectangular polygons to model it
   
   int i, j, k;
   float d1[3], d2[3], normal[3];

   // save the transformation state
   glPushMatrix();

   if (mode == GL_SELECT) glLoadName(2);

   glMaterialfv(GL_FRONT, GL_DIFFUSE, keyboardColor);

   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // Adjust the translate and scale to make the monitor the right
   //    shape and in the right place
   glTranslatef(0, -3, 4);	 

   // vertices for keyboard - must outline each face
   GLfloat keyboardVerts[] = {
      // front face
      0, 0, 4,   8, 0, 4,    8, 1, 4,     0, 1, 4,
      // back face
      0, 0, 0,    0, 1, 0,    8, 1, 0,    8, 0, 0,
      // left face
      0, 0, 0,    0, 0, 4,     0, 1, 4,     0, 1, 0,
      // right face
      8, 0, 0,  8, 1, 0,    8, 1, 4,    8, 0, 4,
      // top face
      0, 1, 0,    0, 1, 4,    8, 1, 4,    8, 1, 0,
      // bottom face
      0, 0, 0,    0, 0, 4,    8, 0, 4,    8, 0, 0
   };

   // colors for the faces
   GLfloat diffuseColors[] = {
      // front face
      1, 0, 0,    
      // back face
      0, 1, 0,   
      // left face
      0, 0, 1,  
      // right face
      1, 1, 0, 
      // top face
      0, 1, 1,  
      // bottom face
      1, 0, 1
   };


   // now load the vertices into OpenGL Quads
   glBegin(GL_QUADS);

      for (i=0, k=0; k<6; k++, i+=12) {
         // each quad consists of 12 numbers, in this order:
         //   the x,y,z of the first vertex, x,y,z of the second,
         //   and so on 

         // compute the normal - the same for all four corners
         for (j=0; j<3; j++) {
            // i is the beginning index of the 12 numbers, and
            //   j is 0 for an x coordinate, 1 for y, 2 for z
            d1[j] = keyboardVerts[i+3+j] - keyboardVerts[i+j];
            d2[j] = keyboardVerts[i+6+j] - keyboardVerts[i+j];
         }
         
         // compute normalized cross product of edge vectors
         normCrossProd(d1, d2, normal);

         // this is the normal
         glNormal3fv(normal);

         // now a different color for each face
         glMaterialfv(GL_FRONT, GL_DIFFUSE, keyboardColor);

         // four points per quad
         glVertex3fv(&keyboardVerts[i]);
         glVertex3fv(&keyboardVerts[i+3]);
         glVertex3fv(&keyboardVerts[i+6]);
         glVertex3fv(&keyboardVerts[i+9]);

      }
   glEnd();
      
   // recover the transform state
   glPopMatrix();

   return;
}

void drawMonitor(GLenum mode) {

   // this models just the rectangular portion of the monitor
   // use an indexed quad mesh to model it
   
   int i, j, k;
   float d1[3], d2[3], normal[3];

   // save the transformation state
   glPushMatrix();

   if (mode == GL_SELECT) glLoadName(3);

   // set the material
   glMaterialfv(GL_FRONT, GL_DIFFUSE, monitorColor);

   // locate it in the scene
   glMatrixMode(GL_MODELVIEW);
   // Adjust the translate and scale to make the monitor the right
   //    shape and in the right place
   glTranslatef(0, 0, 0);	 

   // vertices for keyboard - must outline each face
   GLfloat monitorVerts [] = {
      // first four coordinates are the front corners, ccw from lower left
      0, 2, 2,   0, 8, 2,   8, 8, 2,   8, 2, 2,
      // next four are the back corners
      0, 2, 0,    0, 7, 0,    8, 7, 0,    8, 2, 0
   };
   
   int monitorIndices []= {
      //six faces for monitor
      0, 3, 2, 1,    // front face
      4, 5, 6, 7,    // back face
      0, 4, 7, 3,    // left side
      1, 2, 6, 5,    // right side
      3, 7, 6, 2,    // top
      0, 1, 5, 4     // bottom
   };

   // now load the vertices into OpenGL Quads
   glBegin(GL_QUADS);

      for (i=0, k=0; k<6; k++, i+=4) {
         // each quad consists of 12 numbers, in this order:
         //   the x,y,z of the first vertex, x,y,z of the second,
         //   and so on 

         // compute the normal - the same for all four corners
         for (j=0; j<3; j++) {
            // i is the beginning index of the 12 numbers, and
            //   j is 0 for an x coordinate, 1 for y, 2 for z
            d1[j] = monitorVerts[monitorIndices[(i+1)]*3+j] - 
                            monitorVerts[monitorIndices[i]*3+j];
            d2[j] = monitorVerts[monitorIndices[(i+2)]*3+j] - 
                            monitorVerts[monitorIndices[i]*3+j];
         }
         
         // compute normalized cross product of edge vectors
         normCrossProd(d1, d2, normal);

         // this is the normal
         glNormal3fv(normal);

         // four points per quad
         glVertex3fv(&monitorVerts[monitorIndices[i]*3]);
         glVertex3fv(&monitorVerts[monitorIndices[(i+1)]*3]);
         glVertex3fv(&monitorVerts[monitorIndices[(i+2)]*3]);
         glVertex3fv(&monitorVerts[monitorIndices[(i+3)]*3]);

      }
   glEnd();
      
   // recover the transform state
   glPopMatrix();

   return;
}

void processHits(GLint hits, GLuint buffer[])
{
   // this function goes through the selection hit list of object names

   int i, j;
   GLuint *ptr, minZ, names, closestItem;
   char *nameChosen[5] = {"empty space", "cpu", "keyboard", "monitor", "sphere"};

   printf("hits = %d\n", hits);
   if(hits==0) {
      printf("You have not selected any object.\n");
      return;
   }
   ptr = (GLuint *) buffer;
   minZ = 0xffffffff;
   closestItem = 0;
   for(i=0; i<hits; i++) {
      names = *ptr;
      ptr++;
      if (*ptr < minZ) {
         minZ = *ptr;
         closestItem = *(ptr+2);
      }
      ptr += names+2;
   }
   printf("  front at %g\n",(float) *ptr/0x7fffffff);
   printf("  back at = %g\n",(float) *(ptr+1)/0x7fffffff);
   printf("You have picked the %s.\n", nameChosen[closestItem]);
   printf("Please choose new %s color (r g b a): ", nameChosen[closestItem]);
   if (closestItem==1) scanf("%f %f %f %f", &cpuColor[0], &cpuColor[1],
                  &cpuColor[2], &cpuColor[3]);
   if (closestItem==2) scanf("%f %f %f %f", &keyboardColor[0],
                  &keyboardColor[1], &keyboardColor[2], &keyboardColor[3]);
   if (closestItem==3) scanf("%f %f %f %f", &monitorColor[0],
                  &monitorColor[1], &monitorColor[2], &monitorColor[3]);
   if (closestItem==4) scanf("%f %f %f %f", &sphereColor[0],
                  &sphereColor[1], &sphereColor[2], &sphereColor[3]);

   printf("**************************************************************\n");
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

   // rotate camera
   GLint axis = 3;
   //if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) axis = 0;
   if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) axis = 1;
   if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) axis = 0;
   if (axis < 3) {  // button ups won't change axis value from 3
      theta[axis] += thetaIncr;
      if (theta[axis] > 360.0) theta[axis] -= 360.0;
      display();
   }

   if ((button == GLUT_LEFT_BUTTON)&&(state == GLUT_DOWN)) {
      // here is where selection occurs
      #define BUFSIZE 512
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
         drawCPU(GL_SELECT);
         drawKeyboard(GL_SELECT);
         drawMonitor(GL_SELECT);
         drawSphere(GL_SELECT);
      glPopMatrix();

      glFlush();
  
      hits = glRenderMode(GL_RENDER);
      processHits(hits,selectBuf);

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
   // this code executes whenever the window is redrawn (when opened,
   //   moved, resized, etc.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // set the viewing transform
   setUpView();

   // set up light source
   setUpLight();

   // start drawing objects
   setUpModelTransform();
   drawCPU(GL_RENDER);
   drawKeyboard(GL_RENDER);
   drawMonitor(GL_RENDER);
   drawSphere(GL_RENDER);

   glutSwapBuffers();
}

/* create a double buffered 500x500 pixel color window
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Selection: Lab 8");
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
} */

