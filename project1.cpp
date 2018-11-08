#include <iostream>
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>

/* Global Definitions */
#define WINDOW_WIDTH		600
#define WINDOW_HEIGHT		500
#define HALF_WINDOW_WIDTH_F	300.0f
#define HALF_WINDOW_HEIGHT_F	250.0f
#define DISTANCE(x,y)		abs((x)-(y))
#define X_TO_XF(x)		((x) / HALF_WINDOW_WIDTH_F - 1.0)
#define Y_TO_YF(y)		(- ((y) / HALF_WINDOW_HEIGHT_F - 1.0))
#define X_MOVE_THRESHOLD(r)	((r) / HALF_WINDOW_WIDTH_F)
#define Y_MOVE_THRESHOLD(r)	((r) / HALF_WINDOW_HEIGHT_F)

enum option_e {MENU_EXIT, MENU_POLYGON, MENU_MOVE_VERTEX};
enum state_e  {NORMAL, DRAWING_POLYGON, MOVING_VERTEX};

using namespace std;

/* Class definitions */
class Vertex
{
public:
	GLfloat xf;
	GLfloat yf;
	Vertex(int, int);
	Vertex(float, float);
	Vertex(const Vertex&);
	void update(int, int);
	void update(float xf, float yf);
	Vertex operator-(Vertex v1);
	Vertex operator+(Vertex v1);
	Vertex operator*(float c);
	friend ostream& operator<<(ostream &strm, const Vertex &v);
	bool in_range(int, int, int);
};

Vertex::Vertex(int x, int y)
{
	update(x,y);
}

Vertex::Vertex(float xf, float yf)
{
	update(xf, yf);
}

Vertex::Vertex(const Vertex &v)
{
	update(v.xf, v.yf);
}

void Vertex::update(int x, int y)
{
	this->xf = X_TO_XF(x);
	this->yf = Y_TO_YF(y);
//	cout << "(" << x << " " << y << ") [" << xf << " " << yf << "]" << endl << endl;
	glutPostRedisplay();
}

void Vertex::update(float xf, float yf)
{
	this->xf = xf;
	this->yf = yf;
	glutPostRedisplay();
}

Vertex Vertex::operator-(Vertex v)
{
	Vertex retv = Vertex(xf - v.xf, yf - v.yf);
	return retv;
}

Vertex Vertex::operator+(Vertex v)
{
	Vertex retv = Vertex(xf + v.xf, yf + v.yf);
	return retv;
}

Vertex Vertex::operator*(float c)
{
	Vertex retv = Vertex(xf*c, yf*c);
	return retv;
}

ostream& operator<<(ostream &strm, const Vertex &v)
{
	return strm << "Vertex(" << v.xf << ", " << v.yf << ")" << endl;
}

bool Vertex::in_range(int x, int y, int radial)
{
	float xf = X_TO_XF(x),
	      yf = Y_TO_YF(y);
	return (DISTANCE(this->xf, xf) <= X_MOVE_THRESHOLD(radial) &&
			DISTANCE(this->yf, yf) <= Y_MOVE_THRESHOLD(radial));
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
inline void cancel_drawing(void);
void finalize(void);
void draw_polygons(void);
inline float crossproduct(Vertex v1, Vertex v2);
bool intersecting_polygon(Polygon *p);
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4);

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
	glutAddMenuEntry("Move Vertex", MENU_MOVE_VERTEX);

	glutCreateMenu(&menu_handler);
	glutAddSubMenu("ACTION", action_smenuid);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

#if 0
	// For checking intersection(...);
//	Vertex v1(0,0), v2(600,500), v3(0,500), v4(600,0); // Vertical line segments
//	Vertex v1(0,0), v2(600,500), v3(300,0), v4(600,250); // Parallel line segments
	Vertex v1(0,0), v2(600,500), v4(300,0), v3(600,250); // Parallel line segments
	if (intersection(&v1, &v2, &v3, &v4) != NULL)\
		cout << "INTERSECTS"<< endl;
	else
		cout << "DOESN'T"<< endl;
#endif

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
		case MENU_MOVE_VERTEX:
			state = MOVING_VERTEX;
			break;
	}
}


void mouse_event_handler(int button, int state, int x, int y)
{
	static int mouse_event_count = 0,
		   editing_polygon_index = -1;
	static Vertex *moving_vertex = NULL;

	if (::state == DRAWING_POLYGON)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			glutDetachMenu(GLUT_RIGHT_BUTTON);
			polygons.back().vertices.push_back(Vertex(x, y));
			if (intersecting_polygon(&polygons.back()) == true)
			{
				cerr << "You created an intersecting polygon." << 
					" Nothing to be saved!" << endl;
				polygons.pop_back();
				cancel_drawing();
			}
		}
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			cancel_drawing();
		}
	}
	else if (::state == MOVING_VERTEX)
	{
		if (mouse_event_count++ == 0 && state == GLUT_UP)
			return; // ignore GLUT_UP generated by menu option selection

		if (state == GLUT_UP)
		{
			Vertex old_vertex = Vertex(*moving_vertex);
			if (moving_vertex)
				moving_vertex->update(x, y);
			if (intersecting_polygon(&(polygons[editing_polygon_index])) == true)
			{
				moving_vertex->update(old_vertex.xf, old_vertex.yf);
				cerr << "Cannot move vertex. Resulted in an intersecting polygon!" << endl;
			}
			::state = NORMAL;
			editing_polygon_index = -1;
			moving_vertex = NULL;
			mouse_event_count = 0;
			return;
		}

		for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
			for (unsigned int i = 0; i < p->vertices.size(); i++)
			{
				if (p->vertices[i].in_range(x, y, 10))
				{
					editing_polygon_index = p - polygons.begin();
					moving_vertex = &(p->vertices[i]);
//					cout << "In range of: (" << moving_vertex->xf << ", " <<
//						moving_vertex->yf << ")" << endl;
				}
			}
	}
}

inline void cancel_drawing(void)
{
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	state = NORMAL;
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

/*
 * Checks if two vectors (AB and CD) intersect.
 * A = v1, B = v2, C = v3, D = v4
 */
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4)
{
	float   denom, l, k;

	if (!v1 || !v2 || !v3 || !v4)
		return NULL;

	Vertex p1 = *v1,
	       p2 = *v2,
	       p3 = *v3,
	       p4 = *v4;

//	cout << "Ap1: " << p1 <<"Ap2: " << p2 << "Ap3: " <<p3 <<"Ap4: " << p4 << endl;
	denom = crossproduct(p2-p1, p4-p3);
	
	l = crossproduct(p3-p1, p4-p3) /  denom;
	k = crossproduct(p1-p3, p2-p1) / -denom;

#if 0
	cout << "l: " << l << " k: "<< k << " denom: " << denom << endl;
	cout << "p1: " << p1 <<"p2: " << p2 << "p3: " <<p3 <<"p4: " << p4 << endl;
	cout << p1 + (p2-p1)*l;
#endif
	if (l > 0 && l < 1 && k > 0 && k < 1 && denom != 0)
	{
		return new Vertex(p1 + (p2-p1)*l);
	}

	return NULL;
}

inline float crossproduct(Vertex v1, Vertex v2)
{
	return ((v2.yf * v1.xf) - (v2.xf * v1.yf));
}

bool intersecting_polygon(Polygon *p)
{
	unsigned int i, j;
	unsigned int vnum = p->vertices.size();

	if (vnum < 4)
		return false;

	for (i = 0; i < vnum-1; i++)
	{
		for (j = i+1; j < vnum; j++)
		{
			if (intersection(&(p->vertices[i]),
						&(p->vertices[i+1]),
						&(p->vertices[j]),
						&(p->vertices[(j+1) % vnum])) != NULL)
			{
//				cout << "INTERSECTS" << endl;
//				cout << p->vertices[i] << p->vertices[i+1] <<
//					p->vertices[j] << p->vertices[(j+1)%vnum] << endl;
				return true;
			}
		}
	}
	return false;
}

