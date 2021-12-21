#include <GL/glut.h>
#include <stdlib.h>
#include <iostream>

using std::cin;
using std::cerr;
using std::endl;

// callbacks for glut (see main() for what they do)
void reshape(int w, int h);
void display();
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y); 

// Simple structure for a point
struct Point
{
    int x;
    int y;
    Point() : x(-1), y(-1) {}
    Point(int x, int y) : x(x), y(y) {}
};

struct Color
{
    float r;
    float g;
    float b;

    Color() : r(0), g(0), b(0) {}
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
};

// helpers
void init();
void addPoint(int x, int y);
void keyboard_input();
void draw_point(int x, int y, Color c);
void draw_line(int x0, int y0, int x1, int y1, Color c);
void draw_triangle();
void triangle_wireframe(Color color);

// Keeps track of current shading mode
enum ShadingMode { WIREFRAME, FLAT, GOURAUD };
ShadingMode shading_mode = WIREFRAME;

// Initial window size
int win_w = 512;
int win_h = 512;

// For triangles, 3 points will do
Point points[3];

// Used to keep track of how many points I have so far
int num_points;

int main(int argc, char* argv[])
{
    // initialize glut
    glutInit(&argc, argv);

    // use double buffering with RGB colors
    // double buffer removes most of the flickering
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // set window size and position
    glutInitWindowSize(win_w, win_h);
    glutInitWindowPosition(100, 100);

    // now, create window with title "Scan Conversion"
    glutCreateWindow("Scan Conversion");

    // other stuffs like background color, viewing, etc will be
    // set up in this function.
    init();

    // register callbacks for glut
    glutDisplayFunc(display);   // for display
    glutReshapeFunc(reshape);   // for window move/resize
    glutMouseFunc(mouse);       // for mouse buttons
    glutKeyboardFunc(keyboard); // for keyboard

    // start event processing, i.e., accept user inputs
    glutMainLoop();

    return 0;
}

// called when the window is resized/moved (plus some other cases)
void reshape(int w, int h)
{
    win_w = w;
    win_h = h;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, win_w-1, 0.0, win_h-1, -1.0, 1.0);

    glViewport(0, 0, win_w, win_h);
}


// called when the window needs to be redrawn
void display()
{    
    // clear back buffer with background color that is set in init()
    glClear(GL_COLOR_BUFFER_BIT);

    // now, draw on back buffer just cleared
    draw_triangle();
    glutSwapBuffers();
}

// called when a mouse event (button pressed/released/moved/dragged) occurs
// in glut, 
//     mouse buttons are represented as
//           GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, and GLUT_RIGHT_BUTTON
//     status of mouse buttons are represented as
//           GLUT_UP and GLUT_DOWN
//     (x, y) is the mouse position when the event occurred
void mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:
	if (state == GLUT_DOWN) 
	    addPoint(x, y);
	break;
    default:
	break;
    }
}

// called when a keyboard event (key typed) occurs
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'q':  // quit the program
	exit(0);
    case 'f':  // flat shading
	shading_mode = FLAT;
	break;
    case 'g':  // gouraud shading
	shading_mode = GOURAUD;
	break;
    case 'k':
	keyboard_input();
	num_points = 0;
	break;
    default:
	shading_mode = WIREFRAME;
	break;
    }
}

void init()
{
    // set background color to black
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // create viewing volume
    // -- will use orthogonal projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, win_w-1, 0.0, win_h-1, -1.0, 1.0);
}

// add the point just selected by mouse button
void addPoint(int x, int y)
{
    points[num_points++] = Point(x, y);
    if (num_points == 3) {
	// reset the num_points to 0 for next line
	num_points = 0;

	// tell glut that the current window needs to be redisplayed
	// glut will then redisplay the current window
	glutPostRedisplay();
    }
}

void keyboard_input()
{
    int x, y;
    num_points = 0;
    for (int i=0; i<3; i++) {
	cerr << "Enter point " << i << " => ";
	cin >> x >> y;
	cerr << endl;
	addPoint(x, y);
    }
}
 
void draw_point(int x, int y, Color c)
{
    glBegin(GL_POINTS);
    {
	glColor3f(c.r, c.g, c.b);
	glVertex2d(x, win_h-y);
    }
    glEnd();
}

void draw_line(int x0, int y0, int x1, int y1, Color c)
{
    glBegin(GL_LINES);
    glColor3f(c.r, c.g, c.b);
    glVertex2d(x0, win_h - y0);
    glVertex2d(x1, win_h - y1);
    glEnd();
}
 
float area(int x1, int y1, int x2, int y2, int x3, int y3)
{
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}

void draw_triangle()
{
    switch (shading_mode) {
    case WIREFRAME:
    {
	// choose the color for wireframe
	Color color(1.0, 0.0, 0.0);
	// draw a triangle as wireframe
	// using draw_line()
	triangle_wireframe(color);
	break;
    }
    case FLAT:
    {
	// choose the color for flat shading
	Color color(0.5, 1.0, 0.5);
	// HERE, draw a triangle with flat shading
	//       using draw_point()
	
	triangle_wireframe(color);
	
	int x0 = points[0].x;
        int y0 = points[0].y;
	int x1 = points[1].x;
	int y1 = points[1].y;
	int x2 = points[2].x;
        int y2 = points[2].y;

	int xmin, xmax, ymin, ymax;
	
	if (x0 > x1 && x0 > x2) xmax = x0;
	else if (x1 > x0 && x1 > x2) xmax = x1;
	else if (x2 > x1 && x2 > x0) xmax = x2;

	if (x0 < x1 && x0 < x2) xmin = x0;
	else if (x1 < x0 && x1 < x2) xmin = x1;
	else if (x2 < x1 && x2 < x0) xmin = x2;

	if (y0 > y1 && y0 > y2) ymax = y0;
        else if (y1 > y0 && y1 > y2) ymax = y1;
        else if (y2 > y1 && y2 > y0) ymax = y2;

        if (y0 < y1 && y0 < y2) ymin = y0;
        else if (y1 < y0 && y1 < y2) ymin = y1;
        else if (y2 < y1 && y2 < y0) ymin = y2;

	if (xmin < 0) xmin = xmin * -1;
	if (xmax < 0) xmax = xmax * -1;
	if (ymin < 0) ymin = ymin * -1;
	if (ymax < 0) ymax = ymax * -1;
	
    	for (int j = ymin; j < ymax; j++) { 
        	for (int i = xmin; i < xmax; i++) { 
			//float alpha = ((y1 - y2)*(i - x2)+(x2-x1)*(j - y2))/((y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2));
                        //float beta = ((y2 - y0)*(i - x2)+(x0-x2)*(j - y2))/((y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2));
                        //float gamma = 1.0f - alpha - beta;
			
   			float A = area (x0, y0, x1, y1, x2, y2);
   			float A0 = area (i, j, x1, y1, x2, y2);
   			float A1 = area (x0, y0, i, j, x2, y2);
   			float A2 = area (x0, y0, x1, y1, i, j);

			if (A0 + A1 + A2 == A) {
				draw_point(i, j, color);
            		} 
        	} 
    	}
	break;
    }
    case GOURAUD:
    {
	// choose the vertex colors for gouraud shading
	Color c0(1.0, 0.0, 0.0);
	Color c1(0.0, 1.0, 0.0);
	Color c2(0.0, 0.0, 1.0);
	// HERE, draw a triangle with gouraud shading
	//       using draw_point()
	
	Color color(0.0, 0.0, 0.0);
	triangle_wireframe(color);

        int x0 = points[0].x;
        int y0 = points[0].y;
        int x1 = points[1].x;
        int y1 = points[1].y;
        int x2 = points[2].x;
        int y2 = points[2].y;

	int xmin, xmax, ymin, ymax;

        if (x0 > x1 && x0 > x2) xmax = x0;
        else if (x1 > x0 && x1 > x2) xmax = x1;
        else if (x2 > x1 && x2 > x0) xmax = x2;

        if (x0 < x1 && x0 < x2) xmin = x0;
        else if (x1 < x0 && x1 < x2) xmin = x1;
        else if (x2 < x1 && x2 < x0) xmin = x2;

        if (y0 > y1 && y0 > y2) ymax = y0;
        else if (y1 > y0 && y1 > y2) ymax = y1;
        else if (y2 > y1 && y2 > y0) ymax = y2;

        if (y0 < y1 && y0 < y2) ymin = y0;
        else if (y1 < y0 && y1 < y2) ymin = y1;
        else if (y2 < y1 && y2 < y0) ymin = y2;

        if (xmin < 0) xmin = xmin * -1;
        if (xmax < 0) xmax = xmax * -1;
        if (ymin < 0) ymin = ymin * -1;
        if (ymax < 0) ymax = ymax * -1;

	for (int j = ymin; j < ymax; j++) {
                for (int i = xmin; i < xmax; i++) {
                        //float alpha = ((y1 - y2)*(i - x2)+(x2-x1)*(j - y2))/((y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2));
                        //float beta = ((y2 - y0)*(i - x2)+(x0-x2)*(j - y2))/((y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2));
                        //float gamma = 1.0f - alpha - beta;

                        float A = area (x0, y0, x1, y1, x2, y2);
                        float A0 = area (i, j, x1, y1, x2, y2);
                        float A1 = area (x0, y0, i, j, x2, y2);
                        float A2 = area (x0, y0, x1, y1, i, j);
                        if (A0 + A1 + A2 == A) {
				A0 /= A;
                		A1 /= A;
                		A2 /= A; 
				float r = (A0 * 1) + (A1 * 0) + (A2 * 0);
                                float g = (A0 * 0) + (A1 * 1) + (A2 * 0);
                                float b = (A0 * 0) + (A1 * 0) + (A2 * 1);
                                Color color(r, g, b);
                                draw_point(i, j, color);
                        }
                }
        }
	break;
    }
    }
}

void triangle_wireframe(Color color)
{
    // not much to do.
    // just draw 3 lines using the 3 points
    for (int i=0; i<3; i++) {
	int x0 = points[i].x, y0 = points[i].y;
	int x1 = points[(i+1)%3].x, y1 = points[(i+1)%3].y;
	draw_line(x0, y0, x1, y1, color);
    }
}
