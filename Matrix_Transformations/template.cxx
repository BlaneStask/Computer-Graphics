#include <GL/glut.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <iostream>
#include <cmath>

// typedefs
typedef glm::dvec3 Vector3;   // 3D vectors of double
typedef glm::dvec3 Point3;    // 3D points of double
typedef glm::dvec4 HPoint3;   // 3D points in Homogeneous coordinate system
typedef glm::dmat4 Matrix4;   // 4-by-4 matrix

// glut callbacks
void reshape(int w, int h);
void display();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);

// helpers
void init();
void initObj();
void initCam();
void drawFaces();

// projection
void SetViewMatrix();
void SetOrthoMatrix();
void SetPerspMatrix();

// utilities for matrices and vectors
void    DeviceToWorld(double u, double v, double& x, double& y);
Matrix4 Mult4(Matrix4 a, Matrix4 b);         // (4x4 matrix) . (4x4 matrix)
HPoint3 Homogenize(HPoint3 a);               // returns homogenized HPoint3
HPoint3 TransHPoint3(Matrix4 m, HPoint3 p);  // (4x4 matrix) . (4x1 Vector)

// transformations
void Rotate(double dx, double dy);
void Translate_xy(double tx, double ty);
void Translate_xz(double tx, double ty);
void Scale(double s);

// transformation helpers
Matrix4 SetScaleMatrix(double sx, double sy, double sz); // 4x4 scale matrix
Matrix4 SetTransMatrix(double tx, double ty, double tz); // 4x4 translation matrix
Matrix4 SetRotMatrix(Vector3 n, double angle);           // 4x4 rotation matrix

// default device window size
int win_w = 512;
int win_h = 512;
const double EPSILON = 0.0000001;

// for your convenience while debugging
using std::cout;
using std::cerr;
using std::endl;

void PrintMat(Matrix4 m);     // print Matrix4
void PrintHPoint(HPoint3 p);  // print HPoint3
void PrintPoint(Point3 p);    // print Point3

// for tracking mouse events
struct MouseTracker
{
  int modifiers;
  int button;
  double initx, inity;
  double finalx, finaly;
};
MouseTracker mtracker;

// for camera parameters
struct Camera
{
  bool perspective;               /* projection method */
  double l, r, b, t, n, f;        /* view volume */
  Point3 eye;                     /* eye position */
  Vector3 u, v, w;                /* eye coordinate system */
  Matrix4 Mo;                     /* orthographic projection */
  Matrix4 Mv;                     /* view matrix for arbitrary view*/
  Matrix4 Mp;                     /* perspective matrix */
};

Camera cam;

// for objects
const int MAXNAMESTRING = 20;
const int MAXVERTICES = 1000;
const int MAXEDGES = 500;
const int MAXFACES = 50;

struct Object3D {
  char name[MAXNAMESTRING];       /* The name of object for printing */
  int Nvertices;                  /* number of vertices */
  int Nfaces ;                    /* number of faces */
  Matrix4 frame;                  /* the object to world coord transform */
  Point3 center;                  /* center of mass */
  HPoint3 vertices[MAXVERTICES];  /* coodrdinates of each vertex */
  int faces[MAXFACES][6];         /* If face has N vertices, give N + 1 
			  	     numbers -> first the number of vertices
			  	     in the face, then the index numbers of
				     each vertex as it appears in the 
				     "vertices" array. */
};

Object3D obj = {
  "house", 10, 7,
  
  Matrix4(1.0), // identiy matrix (no transformations applied yet)
  
  // center of the object is at origin
  {0.0, 0.0, 0.0},
  
  // vertices of the object in no particular order
  {  HPoint3(0.0, 1.0, 2.0, 1.0),    HPoint3(-1.0, 0.5, 2.0, 1.0),
     HPoint3(-1.0, -1.0, 2.0, 1.0),  HPoint3(1.0, -1.0, 2.0, 1.0),
     HPoint3(1.0, 0.5, 2.0, 1.0),    HPoint3(0.0, 1.0, -2.0, 1.0),
     HPoint3(-1.0, 0.5, -2.0, 1.0),  HPoint3(-1.0, -1.0, -2.0, 1.0),
     HPoint3(1.0, -1.0, -2.0, 1.0),  HPoint3(1.0, 0.5, -2.0, 1.0)   },
  
  // faces
  {  {5,   0, 1, 2, 3, 4},
     {5,   9, 8, 7, 6, 5},
     {4,   4, 3, 8, 9},
     {4,   0, 4, 9, 5},
     {4,   1, 0, 5, 6},
     {4,   2, 1, 6, 7},
     {4,   3, 2, 7, 8}    }
};

int main(int argc, char *argv[])
{
  // initialize glut
  glutInit(&argc, argv);

  // use double buffering with RGB colors
  // double buffer removes most of the flickering
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // set window size and position
  glutInitWindowSize(win_w, win_h);
  glutInitWindowPosition(100, 100);

  // now, create window with title "Viewing"
  glutCreateWindow("Viewing");
  init();
  
  // initialize (arrange) the object
  initObj();

  // initialize the camera
  initCam();
  
  // register callbacks for glut
  glutDisplayFunc(display);   // for display
  glutReshapeFunc(reshape);   // for window move/resize
  glutMouseFunc(mouse);       // for mouse buttons
  glutMotionFunc(motion);     // for mouse movement while mouse button pressed
  glutKeyboardFunc(keyboard); // for keyboard

  // start event processing, i.e., accept user inputs
  glutMainLoop();

  return 0;
}

// called when the window is resized/moved (and some other cases)
void reshape(int w, int h)
{
  // change window size
  win_w = w;
  win_h = h;

  // set the new viewport
  glViewport(0, 0, (GLint)win_w, (GLint)win_h);

  // orthographic projection when drawing the object.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 511.0, 0.0, 511.0, -1.0, 1.0);
}

// called when the window needs to be redrawn
void display()
{
  glClear(GL_COLOR_BUFFER_BIT);

  // draw the object on the buffer you just cleared
  drawFaces();

  // swap the buffers
  glutSwapBuffers();
}

// called when a mouse event (button pressed/released) occurs in glut, 
//     mouse buttons are represented as
//           GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, and GLUT_RIGHT_BUTTON
//     status of mouse buttons are represented as
//           GLUT_UP and GLUT_DOWN
// 
void mouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {  // mouse pressed.  retrieve the detail
    // which button is pressed?
    mtracker.button = button;
    // any modifiers (keys like shift/ctrl/alt) pressed?
    mtracker.modifiers = glutGetModifiers();
    // convert the mouse position (x,y) in device coord. system
    //   to the corresponding position in world coord. system
    DeviceToWorld(double(x), double(y), mtracker.initx, mtracker.inity);
  }
}

// called when a mouse moves with a button pressed
void motion(int x, int y)
{
  // get the mouse position in world
  DeviceToWorld(double(x), double(y), mtracker.finalx, mtracker.finaly);

  cout << '(' << x << ',' << y << ',' << mtracker.initx << ',' << mtracker.inity << ")  ("
       << x << ',' << y << ',' << mtracker.finalx << ',' << mtracker.finaly << ")" << endl;
  
  // now, process the user input, i.e., mouse movement
  switch (mtracker.button) {
  case GLUT_LEFT_BUTTON:
    if (mtracker.modifiers & GLUT_ACTIVE_SHIFT) {
      // shift + left button ==> translate in xz plane
      Translate_xz(mtracker.finalx - mtracker.initx,
		   mtracker.finaly - mtracker.inity);
    }
    else {
      // left button ==> translate in xy plane
      Translate_xy(mtracker.finalx - mtracker.initx,
		   mtracker.finaly - mtracker.inity);
    }
    break;
  case GLUT_RIGHT_BUTTON:
      // right button ==> scale
    Scale(mtracker.finalx - mtracker.initx);
    break;
  case GLUT_MIDDLE_BUTTON:
      // middle button ==> rotate
    Rotate(mtracker.finalx - mtracker.initx,
	   mtracker.finaly - mtracker.inity);
    break;
  }
  
  // redisplay after transformation
  glutPostRedisplay();

  // reset the mouse position
  mtracker.initx = mtracker.finalx;
  mtracker.inity = mtracker.finaly;
}  

// called when a keyboard event (key typed) occurs
// you need to add cases for 'r' and 'b'
void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'Q':  // quit the program
  case 'q':
    exit(0);
  case 'P':  // toggle the projection method
  case 'p':  // between orthographic and perspective projections
    cam.perspective = !cam.perspective;
    SetPerspMatrix();
    glutPostRedisplay();
    break;
  case 'R': // reset
  case 'r':
      initObj();
      initCam();
      glutPostRedisplay();
      break;
  }
}

void init()
{
  // set background color to black
  glClearColor(0.0, 0.0, 0.0, 0.0);
}


// arrange the object to its initial position
void initObj()
{
  Vector3 n;

  // rotate around y-axis
  n.x = 0.0;  n.y = 1.0;  n.z = 0.0;
  double angle = M_PI / 6.0;
  Matrix4 m1 = SetRotMatrix(n, angle);
  
  // rotate around x-axis
  n.x = 1.0;  n.y = 0.0;  n.z = 0.0;
  angle = M_PI / 6.0;
  Matrix4 m2 = SetRotMatrix(n, angle);

  // translate so that the object is inside view volume
  // (see initCam() for the view volume)
  Matrix4 m3 = SetTransMatrix(0.0, 0.0, -5.0);

  // notice the order of the transformations applied
  //  i.e., Ry -> Rx -> T  becomes  T.Rx.Ry in matrix multiplication
  obj.frame = Mult4(m3, Mult4(m2, m1));
}

// initialize camera parameters
void initCam()
{
  // use orthographic projection as default
  cam.perspective = false;

  // camera position
  cam.eye.x = 0.0;
  cam.eye.y = 0.0;
  cam.eye.z = 0.0;

  // view volume
  cam.l = -5.0;  cam.r = 5.0;
  cam.b = -5.0;  cam.t = 5.0;
  cam.n = -1.0;  cam.f = -6.0;

  // camera coordinate system
  cam.u.x = 1.0;  cam.u.y = 0.0;  cam.u.z = 0.0;
  cam.v.x = 0.0;  cam.v.y = 1.0;  cam.v.z = 0.0;
  cam.w.x = 0.0;  cam.w.y = 0.0;  cam.w.z = 1.0;

  // set Mcam, Mp, Mo
  SetViewMatrix(); 
  SetPerspMatrix();
  SetOrthoMatrix();
}

// draw object faces
void drawFaces()
{
	Matrix4 m1 = Mult4(cam.Mv, obj.frame);
	Matrix4 m2 = Mult4(cam.Mp, m1);
	Matrix4 m = Mult4(cam.Mo, m2);

	const int nf = obj.Nfaces;
	HPoint3 *h = new HPoint3[nf];

	for (int i = 0; i < nf; i++) {
                glBegin(GL_LINE_LOOP);
                        for (int j = 1; j <= obj.faces[i][0]; j++) {
				h[i] = TransHPoint3(m, obj.vertices[obj.faces[i][j]]);
				h[i] = Homogenize(h[i]);
                                glVertex2d(h[i].x, h[i].y);
                        }
                glEnd();
        }
}

// Mcam
void SetViewMatrix()
{
	Matrix4 m;
        m[0][0] =  cam.u.x;  m[0][1] =  cam.u.y;  m[0][2] = cam.u.z;  m[0][3] = 0.0;
        m[1][0] =  cam.v.x;  m[1][1] =  cam.v.y;  m[1][2] = cam.v.z;  m[1][3] = 0.0;
        m[2][0] =  cam.w.x;  m[2][1] =  cam.w.y;  m[2][2] = cam.w.z;  m[2][3] = 0.0;
        m[3][0] =  0.0;  m[3][1] =  0.0;  m[3][2] = 0.0;  m[3][3] = 1.0;

	Matrix4 me;
        me[0][0] =  1.0;  me[0][1] =  0.0;  me[0][2] = 0.0;  me[0][3] = -cam.eye.x;
        me[1][0] =  0.0;  me[1][1] =  1.0;  me[1][2] = 0.0;  me[1][3] = -cam.eye.y;
        me[2][0] =  0.0;  me[2][1] =  0.0;  me[2][2] = 1.0;  me[2][3] = -cam.eye.z;
        me[3][0] =  0.0;  me[3][1] =  0.0;  me[3][2] = 0.0;  me[3][3] = 1.0;
	cam.Mv = Mult4(m, me);
}

// Mo = Mvp . Morth
void SetOrthoMatrix()
{	
	Matrix4 m;
  	m[0][0] = win_w/2.0;  m[0][1] =  0.0;  m[0][2] = 0.0;  m[0][3] = (win_w-1)/2.0;
  	m[1][0] =  0.0;  m[1][1] = win_h/2.0;  m[1][2] = 0.0;  m[1][3] = (win_h-1)/2.0;
  	m[2][0] =  0.0;  m[2][1] =  0.0;  m[2][2] = 1.0;  m[2][3] = 0.0;
  	m[3][0] =  0.0;  m[3][1] =  0.0;  m[3][2] = 0.0;  m[3][3] = 1.0;

	Matrix4 mo;
        mo[0][0] = 2.0/(cam.r-cam.l);  mo[0][1] =  0.0;  mo[0][2] = 0.0;  mo[0][3] = -(cam.r+cam.l)/(cam.r-cam.l);
        mo[1][0] =  0.0;  mo[1][1] = 2.0/(cam.t-cam.b);  mo[1][2] = 0.0;  mo[1][3] = -(cam.t+cam.b)/(cam.t-cam.b);
        mo[2][0] =  0.0;  mo[2][1] =  0.0;  mo[2][2] = 2/(cam.n-cam.f);  mo[2][3] = -(cam.n+cam.f)/(cam.f-cam.n);
        mo[3][0] =  0.0;  mo[3][1] =  0.0;  mo[3][2] = 0.0;  mo[3][3] = 1.0;

	cam.Mo = Mult4(m, mo);
}

// Mp
void SetPerspMatrix()
{
	Matrix4 m;
	if (cam.perspective == false) {
        	m[0][0] =  1.0;  m[0][1] =  0.0;  m[0][2] = 0.0;  m[0][3] = 0.0;
        	m[1][0] =  0.0;  m[1][1] =  1.0;  m[1][2] = 0.0;  m[1][3] = 0.0;
        	m[2][0] =  0.0;  m[2][1] =  0.0;  m[2][2] = 1.0;  m[2][3] = 0.0;
        	m[3][0] =  0.0;  m[3][1] =  0.0;  m[3][2] = 0.0;  m[3][3] = 1.0;
	}
	else {
                m[0][0] =  cam.n;  m[0][1] =  0.0;  m[0][2] = 0.0;  m[0][3] = 0.0;
                m[1][0] =  0.0;  m[1][1] =  cam.n;  m[1][2] = 0.0;  m[1][3] = 0.0;
                m[2][0] =  0.0;  m[2][1] =  0.0;  m[2][2] = (cam.n + cam.f);  m[2][3] = -(cam.f * cam.n);
                m[3][0] =  0.0;  m[3][1] =  0.0;  m[3][2] = 1.0;  m[3][3] = 0.0;
	}
	cam.Mp = m;
}

// convert device coordinate to world coordinate
void DeviceToWorld(double u, double v, double& x, double& y)
{
	x = (((cam.r - cam.l) + u) / win_w) - cam.l;
  	y = (((cam.t - cam.b) + v) / win_h) - cam.b;
}

// returns the product of two 4x4 matrices
Matrix4 Mult4(Matrix4 a, Matrix4 b)
{
  Matrix4 m;
  register double sum;

  for (int j = 0; j < 4;  j++) 
    for (int i = 0; i < 4; i++) {
      sum = 0.0;
      for (int k = 0; k < 4; k++)
	sum +=  a[j][k] * b[k][i];
      m[j][i] = sum;
    }

  return m;
}

// returns the result of homogenization of the input point
// homogenization is to make w = 1
HPoint3 Homogenize(HPoint3 a)
{
  HPoint3 p;
  if ((a.w) != 0.0) {
    p.x = a.x /(a.w);  p.y = a.y /(a.w); 
    p.z = a.z /(a.w);  p.w = 1.0;
  }
  else {
    cerr << "Cannot Homogenize, returning original point\n";
    p.x = a.x;  p.y = a.y;  p.z = a.z;  p.w = a.w;
  }
  return p;
}

// returns the homogeneous 3d point as a result of
// multiplying a 4x4 matrix with a homogeneous point
HPoint3 TransHPoint3(Matrix4 m, HPoint3 p)
{
  HPoint3 temp;
  temp.x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3]*p.w;
  temp.y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3]*p.w;
  temp.z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]*p.w;
  temp.w = m[3][0]*p.x + m[3][1]*p.y + m[3][2]*p.z + m[3][3]*p.w;
  return temp;
}

// translation in xy-plane
void Translate_xy(double tx, double ty)
{
	Matrix4 m;
        if (cam.perspective == false) {
        	m = SetTransMatrix(tx, -ty, 1);
	}
        else {
        	m = SetTransMatrix(tx, -ty, 0);
	}
        obj.frame = Mult4(m, obj.frame);
        drawFaces();
}

// translation in xz-plane
void Translate_xz(double tx, double ty)
{
        Matrix4 m = SetTransMatrix(tx, 0, -tx);
	obj.frame = Mult4(m, obj.frame);
        drawFaces();
}


// uniform scale
void Scale(double sx)
{	
	Matrix4 m;
	if (cam.perspective == false) {
		m = SetScaleMatrix((sx * 0.09) + 1, (sx * 0.09) + 1, 1);
	}
	else {
		m = SetScaleMatrix((sx * 0.5) + 1, (sx * 0.5) + 1, 1);
	}
	obj.frame = Mult4(m, obj.frame);
	drawFaces();
}

// rotation using the Rolling Ball transformation
void Rotate(double dx, double dy)
{
	Vector3 v;
	
	double pdx = dx * dx;
	double pdy = dy * dy;
	double inside = pdx + pdy;
	double dr = sqrt(inside);

	double R = 5.0;

	double insideA = dr / R;
	double angle = atan(insideA);

	v.x = -(dy / dr);
	v.y = dx / dr;
	v.z = 0.0;
	
	//Translate_xy(-win_w / 2, -win_h / 2);
	//Translate_xy(-obj.center.x, -obj.center.y);
	
	Matrix4 m = SetRotMatrix(v, angle);
        obj.frame = Mult4(m, obj.frame);
	
	//Translate_xy(win_w / 2, win_h / 2);
	//Translate_xy((mtracker.finalx - mtracker.initx), (mtracker.finaly - mtracker.inity));
	//Translate_xy(obj.center.x, obj.center.y);
	
	drawFaces();
}

// returns a 4x4 scale matrix, given sx, sy, sz as inputs 
Matrix4 SetScaleMatrix(double sx, double sy, double sz)
{
  Matrix4 m;
  m = Matrix4(1.0);
  m[0][0] = sx;
  m[1][1] = sy;
  m[2][2] = sz;
  return m;
}

// returns a 4x4 translation matrix, given tx, ty, tz as inputs 
Matrix4 SetTransMatrix(double tx, double ty, double tz)
{
  Matrix4 m;
  m = Matrix4(1.0);
  m[0][3] = tx;
  m[1][3] = ty;
  m[2][3] = tz;
  return m;
}

// returns a 4x4 rotation matrix, given an axis and an angle 
Matrix4 SetRotMatrix(Vector3 n, double angle)
{
	Matrix4 m;
	m[0][0] = cos(angle) + (n.x * n.x) * (1 - cos(angle));  
	m[0][1] = n.x * n.y * (1 - cos(angle)) - n.z * sin(angle);  
	m[0][2] = n.x * n.z * (1 - cos(angle)) + n.y * sin(angle);  
	m[0][3] = 0.0;
        m[1][0] = n.y * n.x * (1 - cos(angle)) + n.z * sin(angle);  
	m[1][1] = cos(angle) + (n.y * n.y) * (1 - cos(angle));  
	m[1][2] = n.y * n.z * (1 - cos(angle)) - n.x * sin(angle);  
	m[1][3] = 0.0;
        m[2][0] = n.z * n.x * (1 - cos(angle)) - n.y * sin(angle);  
	m[2][1] = n.z * n.y * (1 - cos(angle)) + n.x * sin(angle);  
	m[2][2] = cos(angle) + (n.z * n.z) * (1 - cos(angle));  
	m[2][3] = 0.0;
        m[3][0] = 0.0;  
	m[3][1] = 0.0;  
	m[3][2] = 0.0;  
	m[3][3] = 1.0;

  	return m;
}

// prints a 4x4 matrix
void PrintMat(Matrix4 m)
{
   for (int i = 0; i < 4; i++) {
     for (int j = 0; j < 4; j++) {
       std::cerr << m[i][j] << " "; 
    }
     std::cerr << std::endl;
   }
}

// prints a homogeneous 3d point / vector
void PrintHPoint(HPoint3 p)
{
  std::cerr << "("
            << p.x << " "
            << p.y << " "
            << p.z << " "
            << p.w << ")" << std::endl;
}

// prints a 3d point / vector
void PrintPoint(Point3 p) {
  std::cerr << "("
            << p.x << " "
            << p.y << " "
            << p.z << " " << std::endl;
}
