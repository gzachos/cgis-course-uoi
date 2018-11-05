#include <iostream>
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* Global Definitions */
#define WINDOW_WIDTH		600
#define WINDOW_HEIGHT		500
#define HALF_WINDWOW_WIDTH_F	300.0f
#define HALF_WINDWOW_HEIGHT_F	250.0f

enum option_e {MENU_EXIT, MENU_POLYGON};
enum state_e  {NORMAL, DRAWING_POLYGON};

using namespace std;

/* Class definitions */
class Vertex
{
public:
	GLint x;
	GLint y;
	GLfloat xf;
	GLfloat yf;
	Vertex(int, int);
	void update(int, int);
	friend ostream& operator<<(ostream &strm, const Vertex &v);
};

Vertex::Vertex(int x, int y)
{
	update(x,y);
}

void Vertex::update(int x, int y)
{
	this->x = x;
	this->y = y;
	this->xf = x / HALF_WINDWOW_WIDTH_F - 1.0;
	this->yf = - (y / HALF_WINDWOW_HEIGHT_F - 1.0);
	glutPostRedisplay();
}

ostream& operator<<(ostream &strm, const Vertex &v)
{
	return strm << "Vertex(" << v.x << ", " << v.y << ")" << endl;
}


class Polygon
{
public:
	vector<Vertex> vertices;
};

/* Function Prototypes */
void window_display(void);
void resize_window(int width, int height);
void menu_handler(int value);
void mouse_event_handler(int button, int state, int x, int y);
void finalize(void);
void draw_polygons(void);

/* Global Data */
int window_id, state = NORMAL;
vector<Polygon> polygons;

int main(int argc, char **argv)
{
	int action_smenuid;

	// Initialize GLUT lib and negotiate a session with the window system
	glutInit(&argc, argv);
	// Set initial display mode to RGBA
	glutInitDisplayMode(GLUT_RGBA);
	// Set the initial window size and position respectively
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(-1, -1); // negative values result in the window system determining actual window position
	// Create a top-level window
	window_id = glutCreateWindow("Project #1");
	// Set 'window_display' as the display callback for the current window
	glutDisplayFunc(&window_display);
	// Disable the generation of keyboard callbacks
	glutKeyboardFunc(NULL);
	// Set 'mouse_event_handler' as the mouse callback for the current window
	glutMouseFunc(&mouse_event_handler);
	// Disable window resizing
	glutReshapeFunc(resize_window);

	// Create Menu and Sub-menus to be displayed on right click
	action_smenuid = glutCreateMenu(&menu_handler);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAddMenuEntry("Polygon", MENU_POLYGON);

	glutCreateMenu(&menu_handler);
	glutAddSubMenu("ACTION", action_smenuid);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Enter the GLUT event processing loop.
	// This function should never return.
	glutMainLoop();

	return (EXIT_FAILURE);
}


void window_display()
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_polygons();
	glFlush();
}


void resize_window(int width, int height)
{
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
}


void menu_handler(int value)
{
	switch (value)
	{
		case MENU_EXIT:
			finalize();
			break;
		case MENU_POLYGON:
			state = DRAWING_POLYGON;
			polygons.push_back(Polygon());
			break;
	}
}


void mouse_event_handler(int button, int state, int x, int y)
{
	if (::state == DRAWING_POLYGON)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			glutDetachMenu(GLUT_RIGHT_BUTTON);
			polygons.back().vertices.push_back(Vertex(x, y));
		}
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			glutAttachMenu(GLUT_RIGHT_BUTTON);
			::state = NORMAL;
		}
	}
}


void finalize(void)
{
	glutDestroyWindow(window_id);
	exit(EXIT_SUCCESS);
}


void draw_polygons(void)
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		glColor3ub(0x00, 0x00, 0x00);
		for (unsigned int i = 0, j; i < p->vertices.size(); i++)
		{
			glVertex3f(p->vertices[i].xf, p->vertices[i].yf, 0.0f);
			j = (i + 1) % p->vertices.size();
			glVertex3f(p->vertices[j].xf, p->vertices[j].yf, 0.0f);
		//	cout << "(" << p->vertices[i].xf << "," << p->vertices[i].yf << ") ";
		}
	}
	glEnd();
	glFlush();
}


