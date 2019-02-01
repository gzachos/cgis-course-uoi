#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

#if 0
	#define DRAW_LAST_EDGE_SINCE_START
#else
	#undef DRAW_LAST_EDGE_SINCE_START
#endif

/* Global Definitions */
#define WINDOW_WIDTH		600
#define WINDOW_HEIGHT		500
#define DISTANCE(x,y)		abs((x)-(y))
#define MIN_VERTEX_NUM		3
#define SHOW_CLIP_POLY_MILLIS	2500

#define NUM_OF_COLORS		16
#define ENTRY(x,offset)		#x,x+offset
#define COLOR_TO_RGB(x)		color[x].r,color[x].g,color[x].b
#define ADD_COLOR_ENTRIES(offset) {                         \
		glutAddMenuEntry(ENTRY(BLACK,offset));      \
		glutAddMenuEntry(ENTRY(WHITE,offset));      \
		glutAddMenuEntry(ENTRY(RED,offset));        \
		glutAddMenuEntry(ENTRY(GREEN,offset));      \
		glutAddMenuEntry(ENTRY(BLUE,offset));       \
		glutAddMenuEntry(ENTRY(YELLOW,offset));     \
		glutAddMenuEntry(ENTRY(SIENNA,offset));     \
		glutAddMenuEntry(ENTRY(ORANGE,offset));     \
		glutAddMenuEntry(ENTRY(INDIGO,offset));     \
		glutAddMenuEntry(ENTRY(MAGENTA,offset));    \
		glutAddMenuEntry(ENTRY(VIOLET,offset));     \
		glutAddMenuEntry(ENTRY(SILVER,offset));     \
		glutAddMenuEntry(ENTRY(ROYAL_BLUE,offset)); \
		glutAddMenuEntry(ENTRY(CYAN,offset));       \
		glutAddMenuEntry(ENTRY(CHARTREUSE,offset)); \
		glutAddMenuEntry(ENTRY(GOLD,offset));       \
	}

enum color_e  {BLACK, WHITE, RED, GREEN, BLUE, YELLOW, SIENNA, ORANGE, INDIGO,
               MAGENTA, VIOLET, SILVER, ROYAL_BLUE, CYAN, CHARTREUSE, GOLD};
enum option_e {MENU_EXIT = 2*NUM_OF_COLORS, MENU_POLYGON, MENU_MOVE_VERTEX,
		MENU_CLIPPING, MENU_EXTRUDE, MENU_EXIT3D};
enum state_e  {NORMAL, DRAWING_POLYGON, MOVING_VERTEX, CLIPPING, EXTRUSION};
enum clipstate_e {CLIPPING_START, CLIPPING_END};

typedef struct color_s {
	GLubyte r;
	GLubyte g;
	GLubyte b;
} color_t;

color_t color[NUM_OF_COLORS] = {{0x00, 0x00, 0x00},  // BLACK
                                {0xff, 0xff, 0xff},  // WHITE
                                {0xff, 0x00, 0x00},  // RED
                                {0x00, 0x80, 0x00},  // GREEN
                                {0x00, 0x00, 0xff},  // BLUE
                                {0xff, 0xff, 0x00},  // YELLOW
                                {0xa0, 0x52, 0x2d},  // SIENNA
                                {0xff, 0xa5, 0x00},  // ORANGE
                                {0x4b, 0x00, 0x82},  // INDIGO
                                {0xff, 0x00, 0xff},  // MAGENTA
                                {0xee, 0x82, 0xee},  // VIOLET
                                {0xc0, 0xc0, 0xc0},  // SILVER
                                {0x41, 0x69, 0xe1},  // ROYAL_BLUE
                                {0x00, 0xff, 0xff},  // CYAN
                                {0x7f, 0xff, 0x00},  // CHARTREUSE
                                {0xff, 0xd7, 0x00}}; // GOLD

/* Class definitions */
class Vertex
{
private:
	GLint x;
	GLint y;
public:
	Vertex(int, int);
	Vertex(int, int, int);
	Vertex(const Vertex&);
	const GLint& get_x() const;
	const GLint& get_y() const;
	void update(int, int);
	Vertex operator-(Vertex v1);
	Vertex operator+(Vertex v1);
	Vertex operator*(float c);
	bool operator==(const Vertex rhs);
	bool operator!=(const Vertex rhs);
	bool operator<(const Vertex rhs);
	friend ostream& operator<<(ostream &strm, const Vertex &v);
	bool in_range(int, int, int);
	bool in_x_range(int, int, int);
};

Vertex::Vertex(int x, int y)
{
	update(x, y);
}

Vertex::Vertex(const Vertex &v)
{
	update(v.x, v.y);
}

void Vertex::update(int x, int y)
{
	this->x = x;
	this->y = y;
}

const GLint& Vertex::get_x() const
{
	return x;
}

const GLint& Vertex::get_y() const
{
	return y;
}

Vertex Vertex::operator-(Vertex v)
{
	Vertex retv = Vertex(x - v.x, y - v.y);
	return retv;
}

Vertex Vertex::operator+(Vertex v)
{
	Vertex retv = Vertex(x + v.x, y + v.y);
	return retv;
}

Vertex Vertex::operator*(float c)
{
	Vertex retv = Vertex(x*c, y*c);
	return retv;
}

bool Vertex::operator==(const Vertex rhs)
{
	return x == rhs.x && y == rhs.y;
}

bool Vertex::operator!=(const Vertex rhs)
{
	return x != rhs.x && y != rhs.y;
}

bool Vertex::operator<(const Vertex rhs)
{
	return x < rhs.x && y < rhs.y;
}

ostream& operator<<(ostream &strm, const Vertex &v)
{
	return strm << "Vertex(" << v.x << ", " << v.y << ")" << endl;
}

bool Vertex::in_range(int x, int y, int radial)
{
	return (DISTANCE(this->x, x) <= radial && DISTANCE(this->y, y) <= radial);
}

bool Vertex::in_x_range(int x, int y, int threshold)
{
	return (DISTANCE(this->x, x) <= threshold && this->y == y);
}


class Triangle
{
private:
	Vertex v0;
	Vertex v1;
	Vertex v2;
public:
	Triangle(Vertex v0, Vertex v1, Vertex v2);
	const Vertex& get_v0() const;
	const Vertex& get_v1() const;
	const Vertex& get_v2() const;
};

Triangle::Triangle(Vertex v0, Vertex v1, Vertex v2)
	: v0(v0), v1(v1), v2(v2)
{
	// nothing else to do here!
};

const Vertex& Triangle::get_v0() const
{
	return v0;
}

const Vertex& Triangle::get_v1() const
{
	return v1;
}

const Vertex& Triangle::get_v2() const
{
	return v2;
}


class Polygon
{
private:
	vector<Vertex> vertices;
	vector<Triangle> triangles;
	color_e line_clr;
	color_e fill_clr;
	int extrusion_length;
public:
	Polygon(color_e, color_e);
	vector<Vertex>& get_vertices();
	vector<Triangle>& get_triangles();
	const color_e& get_line_clr() const;
	const color_e& get_fill_clr() const;
	const int& get_extrusion_length() const;
	void set_extrusion_length(int extrusion_length);
	bool operator==(const Polygon rhs);
	Vertex *contains(Vertex v);
};

Polygon::Polygon(color_e line, color_e fill)
{
	line_clr = line;
	fill_clr = fill;
}

vector<Vertex>& Polygon::get_vertices()
{
	return vertices;
}

vector<Triangle>& Polygon::get_triangles()
{
	return triangles;
}

const color_e& Polygon::get_line_clr() const
{
	return line_clr;
}

const color_e& Polygon::get_fill_clr() const
{
	return fill_clr;
}

const int& Polygon::get_extrusion_length() const
{
	return extrusion_length;
}

void Polygon::set_extrusion_length(int extrusion_length)
{
	this->extrusion_length = extrusion_length;
}

bool Polygon::operator==(const Polygon rhs)
{
	if (this->vertices.size() != rhs.vertices.size())
		return false;
	for (unsigned int i = 0; i < this->vertices.size(); i++)
		if (this->vertices[i] != rhs.vertices[i])
			return false;
	return this->fill_clr == rhs.fill_clr &&
		this->line_clr == rhs.line_clr &&
		this->extrusion_length == rhs.extrusion_length;
}

Vertex *Polygon::contains(Vertex v)
{
	unsigned int i;
	for (i = 0; i < vertices.size(); i++)
		if (vertices[i] == v)
			return &(vertices[i]);
	for (i = 0; i < vertices.size(); i++)
		if (vertices[i].in_x_range(v.get_x(), v.get_y(), 3))
			return &(vertices[i]);
	return NULL;
}


/* Function Prototypes */
void window_display(void);
void resize_window(int width, int height);
void menu_handler(int value);
void keyboard_event_handler(unsigned char key, int x, int y);
void mouse_event_handler(int button, int state, int x, int y);
inline void leave_current_state(void);
void finalize(void);
inline void glLine3i(Vertex v0, Vertex v1, int z);
inline void glTriangle3i(Vertex v0, Vertex v1, Vertex v2, int z);
void draw_polygons(void);
void draw_polygon_bounds(void);
void draw_polygon_quads(void);
void draw_polygon_area(void);
void draw_polygon_triangles(void);
void draw_clipping_polygon(void);
void draw_grid(void);
inline float crossproduct(Vertex v1, Vertex v2);
#ifdef DRAW_LAST_EDGE_SINCE_START
bool intersecting_polygon(Polygon *p);
#else
bool intersecting_polygon(Polygon *p, bool ignore_last_edge);
#endif
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, bool ignore_edge_points);
void sh_clip(Polygon *p);
bool inside_clip_edge(Vertex p, Vertex cp1, Vertex cp2);
void triangulate(Polygon *p);

// Given Triangulation Code function prototype
bool Process(const vector<Vertex> &contour, vector<Vertex> &result);


/* Global Data */
int window_id, state = NORMAL;
vector<Polygon> polygons;
color_e line_clr = BLACK, fill_clr = WHITE;
Vertex *cmin, *cmax;
bool show_triangles = false, show_clipping_polygon = false;
// Extrusion-related data
double posx = WINDOW_WIDTH + 150, posy = WINDOW_HEIGHT + 100,
       posz = -150, lookx = 0, looky = 1, lookz = 0,
       upx = 0, upy = 0, upz = -1;


int main(int argc, char **argv)
{
	int action_smenuid, lineclr_smenuid, fillclr_smenuid;

	// Initialize GLUT lib and negotiate a session with the window system
	glutInit(&argc, argv);

	// Set initial display mode to RGBA
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// Set the initial window size and position respectively
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(-1, -1); // negative values result in the window system determining actual window position
	// Create a top-level window
	window_id = glutCreateWindow("Project #1");
	// Set 'window_display' as the display callback for the current window
	glutDisplayFunc(&window_display);
	// Set 'keyboard_event_handler' as the keyboard callback for the current window
	glutKeyboardFunc(&keyboard_event_handler);
	// Set 'mouse_event_handler' as the mouse callback for the current window
	glutMouseFunc(&mouse_event_handler);
	// Disable window resizing
	glutReshapeFunc(resize_window);

	// Create Menu and Sub-menus to be displayed on right click
	action_smenuid = glutCreateMenu(&menu_handler);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAddMenuEntry("Polygon", MENU_POLYGON);
	glutAddMenuEntry("Move Vertex", MENU_MOVE_VERTEX);
	glutAddMenuEntry("Clipping", MENU_CLIPPING);
	glutAddMenuEntry("Extrude", MENU_EXTRUDE);
	glutAddMenuEntry("Exit 3D mode", MENU_EXIT3D);

	lineclr_smenuid = glutCreateMenu(&menu_handler);
	ADD_COLOR_ENTRIES(0);
	fillclr_smenuid = glutCreateMenu(&menu_handler);
	ADD_COLOR_ENTRIES(NUM_OF_COLORS);

	glutCreateMenu(&menu_handler);
	glutAddSubMenu("ACTION", action_smenuid);
	glutAddSubMenu("LINE_COLOR", lineclr_smenuid);
	glutAddSubMenu("FILL_COLOR", fillclr_smenuid);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Enter the GLUT event processing loop.
	// This function should never return.
	glutMainLoop();

	return (EXIT_FAILURE);
}

// Used to display the clipping polygon for SHOW_CLIP_POLY_MILLIS milliseconds
// after selection.
void timer_func(int value)
{
	show_clipping_polygon = (value == 0);
	glutPostRedisplay();
	if (value == 0)
		glutTimerFunc(SHOW_CLIP_POLY_MILLIS, timer_func, CLIPPING_END);
	else
		cmin = cmax = NULL;
}

void window_display()
{
	/* Clear colors and depth */
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	/* projection matrix (camera) */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (state == EXTRUSION)
	{
		/* Enable depth */
		glClearDepth(1.0);
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);

		GLfloat aspect = (GLfloat) WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT;
		glViewport(0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT);

		/* Camera perspective */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, aspect, 1.0, 2000.0);

		/* Model view */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		gluLookAt(posx, posy, posz, lookx, looky, lookz, upx, upy, upz);
	}

	draw_polygons();
	if (show_clipping_polygon)
		draw_clipping_polygon();
	glutSwapBuffers();
}

void draw_grid(void)
{
	int i;
	for (i = 0; i < 40; i++)
	{
		glPushMatrix();
		if (i < 20)
		{
			glTranslatef(0, 0, i);

			glBegin(GL_LINES);
			glColor3f(0,0,0);
			glLineWidth(1);
			glVertex3f(0, -0.1, 0);
			glVertex3f(WINDOW_HEIGHT, -0.1, 0);
			glEnd();
			glPopMatrix();
		}
		else
		{
			glTranslatef(i-20, 0, 0);
			glRotatef(-90, 0, 1, 0);

			glBegin(GL_LINES);
			glColor3f(0,0,0);
			glLineWidth(1);
			glVertex3f(0, -0.1, 0);
			glVertex3f(WINDOW_WIDTH, -0.1, 0);
			glEnd();
			glPopMatrix();
		}
	}
}

void draw_clipping_polygon(void)
{
	if (cmin != NULL && cmax != NULL)
	{
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glColor3ub(COLOR_TO_RGB(RED));
		glVertex3i(cmin->get_x(), cmin->get_y(), 0);
		glVertex3i(cmax->get_x(), cmin->get_y(), 0);
		glVertex3i(cmax->get_x(), cmin->get_y(), 0);
		glVertex3i(cmax->get_x(), cmax->get_y(), 0);
		glVertex3i(cmax->get_x(), cmax->get_y(), 0);
		glVertex3i(cmin->get_x(), cmax->get_y(), 0);
		glVertex3i(cmin->get_x(), cmax->get_y(), 0);
		glVertex3i(cmin->get_x(), cmin->get_y(), 0);
		glEnd();
	}
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
			if (state == EXTRUSION)
			{
				cerr << "Cannot create a new polygon when on 3D mode." << endl;
				break;
			}
			state = DRAWING_POLYGON;
			polygons.push_back(Polygon(line_clr, fill_clr));
#ifdef VARIABLE_EXTRUSION_LENGTH
			cout << "Please provide an extrusion length for this polygon." << endl <<
				"A good value is in the range [50, 200] depending" <<
				" on the size of your polygons." << endl << "Extrusion length: ";
			cin >> polygons.back().extrusion_length;
			cout << "You selected " << polygons.back().extrusion_length <<
				" as the extrusion length." << endl;
#endif
			break;
		case MENU_MOVE_VERTEX:
			if (state == EXTRUSION)
			{
				cerr << "Cannot move a polygon vertex when on 3D mode." << endl;
				break;
			}
			state = MOVING_VERTEX;
			break;
		case MENU_CLIPPING:
			if (state == EXTRUSION)
			{
				cerr << "Cannot clip polygon(s) when on 3D mode." << endl;
				break;
			}
			state = CLIPPING;
			break;
		case MENU_EXTRUDE:
			state = EXTRUSION;
#ifndef VARIABLE_EXTRUSION_LENGTH
			cout << "Please provide an extrusion length." << endl <<
				"A good value is in the range [50, 200] depending" <<
				" on the size of your polygons." << endl << "Extrusion length: ";
			int extrusion_length;
			cin >> extrusion_length; // Check for positive or something else?
			cout << "You selected " << extrusion_length << " as the extrusion length." << endl;
			for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
				p->set_extrusion_length(extrusion_length);
#endif
			break;
		case MENU_EXIT3D:
			state = NORMAL;
			break;
		default:
			if (value >= BLACK && value < BLACK + NUM_OF_COLORS)
				line_clr = (color_e) value;
			else if (value >= BLACK + NUM_OF_COLORS && value < (NUM_OF_COLORS<<1))
				fill_clr = (color_e) (value - NUM_OF_COLORS);
	}
}

void keyboard_event_handler(unsigned char key, int x, int y)
{
	switch(key) {
		case 'T':
		case 't':
			show_triangles = !show_triangles;
			break;
		case 'W':
		case 'w': // move camera up
			posz -= 2.0;
			break;
		case 'S':
		case 's': // move camera down
			posz += 2.0;
			break;
		case 'A':
		case 'a': // move camera left
			posx -= 2.0;
			break;
		case 'D':
		case 'd': // move camera right
			posx += 2.0;
			break;
		case 'I':
		case 'i': // move camera in
			posy -= 2.0;
			break;
		case 'O':
		case 'o': // move camera out
			posy += 2.0;
			break;
	}
}

void mouse_event_handler(int button, int state, int x, int y)
{
	static int mouse_event_count = 0,
		   editing_polygon_index = -1;
	static Vertex *moving_vertex = NULL,
		      *v0 = NULL, *v1 = NULL;
	static bool created_polygon = false;

	// A polygon is currently being drawn.
	if (::state == DRAWING_POLYGON)
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		// Add another polygon vertex and in case a self-intersecting
		// polygon occured, the polygon it is removed.
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			polygons.back().get_vertices().push_back(Vertex(x, y));
			triangulate(&(polygons.back())); // To allow area coloring (on the fly)
#ifdef DRAW_LAST_EDGE_SINCE_START
			if (intersecting_polygon(&polygons.back()) == true)
#else
			if (intersecting_polygon(&polygons.back(), true) == true)
#endif
			{
				cerr << "You created an intersecting polygon." <<
					" Nothing to be saved!" << endl;
				polygons.pop_back();
				leave_current_state();
			}
			created_polygon = true;
		}
		// Stop polygon drawing, check for a minumum number of vertices and
		// for self-intersection. Remove polygon if requirements are not met.
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			leave_current_state();
#ifndef DRAW_LAST_EDGE_SINCE_START
			bool removed_polygon = false;
			if (intersecting_polygon(&polygons.back(), false) == true)
			{
				cerr << "You created an intersecting polygon." <<
					" Nothing to be saved!" << endl;
				polygons.pop_back();
				removed_polygon = true;
			}
#endif
			if (created_polygon == true && polygons.back().get_vertices().size() < MIN_VERTEX_NUM)
			{

				cerr << "At least " << MIN_VERTEX_NUM << " vertices are required to" <<
					" create a polygon. Nothing to be saved!" << endl;
#ifndef DRAW_LAST_EDGE_SINCE_START
				if (removed_polygon == false)
#endif
				{
					polygons.pop_back();
				}
			}
			created_polygon = false;
		}
	}
	// Move a vertex to a new position.
	else if (::state == MOVING_VERTEX)
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		if (mouse_event_count++ == 0 && state == GLUT_UP)
			return; // ignore GLUT_UP generated by menu option selection

		if (state == GLUT_UP)
		{
			if (moving_vertex != NULL) // A vertex has been selected
			{
				Vertex *old_vertex = new Vertex(*moving_vertex);
				moving_vertex->update(x, y);
#ifdef DRAW_LAST_EDGE_SINCE_START
				if (intersecting_polygon(&(polygons[editing_polygon_index])) == true)
#else
				if (intersecting_polygon(&(polygons[editing_polygon_index]), false) == true)
#endif
				{
					moving_vertex->update(old_vertex->get_x(), old_vertex->get_y());
					cerr << "Cannot move vertex. Resulted in an intersecting polygon!" << endl;
				}
				else // In case a vertex has been updated
					triangulate(&(polygons[editing_polygon_index]));
			}
			leave_current_state();
			editing_polygon_index = -1;
			moving_vertex = NULL;
			mouse_event_count = 0;
			return;
		}

		// Find the vertex, if it exists, that lies near to the window point the
		// user has left (down) clicked.
		for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
			for (unsigned int i = 0; i < p->get_vertices().size(); i++)
			{
				if (p->get_vertices()[i].in_range(x, y, 10))
				{
					editing_polygon_index = p - polygons.begin();
					moving_vertex = &(p->get_vertices()[i]);
				}
			}
	}
	// Polygon clipping using the Sutherland–Hodgman algorithm.
	else if (::state == CLIPPING)
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		if (mouse_event_count++ == 0 && state == GLUT_UP)
			return; // ignore GLUT_UP generated by menu option selection

		if (state == GLUT_DOWN)
		{
			v0 = new Vertex(x,y);
		}
		else if (state == GLUT_UP)
		{
			v1 = new Vertex(x,y);
			if (*v0 == *v1)
			{
				cerr << "The clipping reactangular cannot be a single point!" << endl;
				cmin = cmax = NULL; // TODO remove if cmin/cmax are set to NULL after clipping
			}
			else
			{
				cmin = new Vertex(min(v0->get_x(),v1->get_x()), min(v0->get_y(), v1->get_y()));
				cmax = new Vertex(max(v0->get_x(),v1->get_x()), max(v0->get_y(), v1->get_y()));
				glutTimerFunc(0, timer_func, CLIPPING_START); // Display clipping polygon
				// Clip and triangulate each polygon.
				for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
				{
					sh_clip(&(*p));
					triangulate(&(*p));
				}
			}
			leave_current_state();
			mouse_event_count = 0;
		}
	}
}

inline void leave_current_state(void)
{
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	::state = NORMAL;
}

void finalize(void)
{
	glutDestroyWindow(window_id);
	exit(EXIT_SUCCESS);
}

void draw_polygons(void)
{
	draw_polygon_area();
	if (show_triangles == true)
		draw_polygon_triangles();

	if (::state == EXTRUSION)
		draw_polygon_quads();
	draw_polygon_bounds();
}

void draw_polygon_bounds(void)
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		glColor3ub(COLOR_TO_RGB(p->get_line_clr()));
		for (unsigned int i = 0, j; i < p->get_vertices().size(); i++)
		{
#ifndef DRAW_LAST_EDGE_SINCE_START
			if (::state == DRAWING_POLYGON && *p == polygons.back() && i == p->get_vertices().size()-1)
				continue;
#endif
			j = (i + 1) % p->get_vertices().size();
			glLine3i(p->get_vertices()[i], p->get_vertices()[j], 0);
			if (::state == EXTRUSION)
				glLine3i(p->get_vertices()[i], p->get_vertices()[j], -(p->get_extrusion_length()));
		}
	}
	glEnd();
}

void draw_polygon_quads(void)
{
	glLineWidth(2.0f);
	glDepthMask(true);

	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		glBegin(GL_QUADS);
		glColor3ub(COLOR_TO_RGB(p->get_line_clr()));
		for (unsigned int i = 0, j; i < p->get_vertices().size(); i++)
		{
			Vertex *v0 = &(p->get_vertices()[i]);
			j = (i + 1) % p->get_vertices().size();
			Vertex *v1 = &(p->get_vertices()[j]);

			glVertex3i(v0->get_x(), v0->get_y(), -(p->get_extrusion_length()));
			glVertex3i(v1->get_x(), v1->get_y(), -(p->get_extrusion_length()));
			glVertex3i(v1->get_x(), v1->get_y(), 0);
			glVertex3i(v0->get_x(), v0->get_y(), 0);
		}
		glEnd();
	}
}

inline void glTriangle3i(Vertex v0, Vertex v1, Vertex v2, int z)
{
	glVertex3i(v0.get_x(), v0.get_y(), z);
	glVertex3i(v1.get_x(), v1.get_y(), z);
	glVertex3i(v2.get_x(), v2.get_y(), z);
}

inline void glLine3i(Vertex v0, Vertex v1, int z)
{
	glVertex3i(v0.get_x(), v0.get_y(), z);
	glVertex3i(v1.get_x(), v1.get_y(), z);
}

void draw_polygon_area()
{
	glLineWidth(1.0f);
	glBegin(GL_TRIANGLES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
#ifndef DRAW_LAST_EDGE_SINCE_START
		if (::state == DRAWING_POLYGON && *p == polygons.back())
			continue;
#endif
		glColor3ub(COLOR_TO_RGB(p->get_fill_clr()));
		for (unsigned int i = 0; i < p->get_triangles().size(); i++)
		{
			Triangle *t = &(p->get_triangles()[i]);
			glTriangle3i(t->get_v0(), t->get_v1(), t->get_v2(), 0);
			if (::state == EXTRUSION)
				glTriangle3i(t->get_v0(), t->get_v1(), t->get_v2(), -(p->get_extrusion_length()));
		}
	}
	glEnd();
}

void draw_polygon_triangles()
{
	glLineWidth(2.0f);
	glColor3ub(COLOR_TO_RGB(GREEN));
	glBegin(GL_LINES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		int z1 = 1, z2 = -1;
		if (p->get_extrusion_length() < 0)
		{
			z1 = -1;
			z2 = 1;
		}
		for (unsigned int i = 0; i < p->get_triangles().size(); i++)
		{
			Triangle *t = &(p->get_triangles()[i]);
			glLine3i(t->get_v0(), t->get_v1(), z1);
			glLine3i(t->get_v1(), t->get_v2(), z1);
			glLine3i(t->get_v2(), t->get_v0(), z1);

			if (::state == EXTRUSION)
			{
				glLine3i(t->get_v0(), t->get_v1(), -(p->get_extrusion_length()) + z2);
				glLine3i(t->get_v1(), t->get_v2(), -(p->get_extrusion_length()) + z2);
				glLine3i(t->get_v2(), t->get_v0(), -(p->get_extrusion_length()) + z2);
			}
		}
	}
	glEnd();
}

/*
 * Checks if two vectors (AB and CD) intersect.
 * A = v1, B = v2, C = v3, D = v4
 */
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, bool ignore_edge_points)
{
	float   denom, l, k;

	if (!v1 || !v2 || !v3 || !v4)
		return NULL;

	Vertex p1 = *v1,
	       p2 = *v2,
	       p3 = *v3,
	       p4 = *v4;

	denom = crossproduct(p2-p1, p4-p3);

	l = crossproduct(p3-p1, p4-p3) /  denom;
	k = crossproduct(p1-p3, p2-p1) / -denom;

	if (l >= 0 && l <= 1 && k >= 0 && k <= 1 && denom != 0)
	{
		if (ignore_edge_points && (l == 0 || l == 1 || k == 0 || k == 1))
			return NULL;
		return new Vertex(p1 + (p2-p1)*l);
	}

	return NULL;
}

inline float crossproduct(Vertex v1, Vertex v2)
{
	return ((v2.get_y() * v1.get_x()) - (v2.get_x() * v1.get_y()));
}

// Checks for a self-intersecting polygon.
#ifdef DRAW_LAST_EDGE_SINCE_START
bool intersecting_polygon(Polygon *p)
#else
bool intersecting_polygon(Polygon *p, bool ignore_last_edge)
#endif
{
	unsigned int i, j;
	unsigned int vnum = p->get_vertices().size();

	if (vnum < 4)
		return false;

	for (i = 0; i < vnum-1; i++)
	{
		for (j = i+1; j < vnum; j++)
		{
#ifndef DRAW_LAST_EDGE_SINCE_START
			if (ignore_last_edge && (i == vnum-1 || j == vnum-1))
				continue;
#endif
			if (intersection(&(p->get_vertices()[i]),
						&(p->get_vertices()[i+1]),
						&(p->get_vertices()[j]),
						&(p->get_vertices()[(j+1) % vnum]), true) != NULL)
			{
				return true;
			}
		}
	}
	return false;
}

// Implementation of the Sutherland–Hodgman clipping algorithm.
void sh_clip(Polygon *p)
{
	if (cmin == NULL || cmax == NULL || p == NULL)
		return;

	// The clipping polygon
	Vertex cp[8] = {
		Vertex(0, cmin->get_y()),
		Vertex(WINDOW_WIDTH, cmin->get_y()),
		Vertex(cmax->get_x(), 0),
		Vertex(cmax->get_x(), WINDOW_HEIGHT),
		Vertex(WINDOW_WIDTH, cmax->get_y()),
		Vertex(0, cmax->get_y()),
		Vertex(cmin->get_x(), WINDOW_HEIGHT),
		Vertex(cmin->get_x(), 0)
	};

	vector<Vertex> output_list = p->get_vertices();
	Vertex *cp0, *cp1, *ip;

	for (int j = 0; j < 8; j += 2)
	{
		cp0 = &(cp[j]);
		cp1 = &(cp[j+1]);

 		vector<Vertex> input_list = output_list;
		output_list.clear();
		Vertex *s;
		if (input_list.empty() == false)
			s = new Vertex(input_list.back());
		for (unsigned int i = 0; i < input_list.size(); i++)
		{
			Vertex *e = &(input_list[i]);
			if (inside_clip_edge(*e, *cp0, *cp1))
			{
				if (!inside_clip_edge(*s, *cp0, *cp1))
				{
					// Case 4: incoming
					ip = intersection(s, e, cp0, cp1, false);
					if (ip != NULL)
						output_list.push_back(*ip);
					else
						cerr << "A: no intersection point" << endl;
				}
				// else: Case 1
				output_list.push_back(input_list[i]);
			}
			else if (inside_clip_edge(*s, *cp0, *cp1))
			{
				// Case 2: outgoing
				ip = intersection(s, e, cp0, cp1, false);
				if (ip != NULL)
					output_list.push_back(*ip);
				else
					cerr << "B: no intersection point" << endl;
			}
			// else: Case 3
			s = e;
		}
	}
	p->get_vertices() = output_list;
}

// Checks whether a point (p) lies on the left of the clipping polygon's edge.
// If true, the point lies inside the clipping polygon as the later is defined
// in a counter-clockwise manner.
bool inside_clip_edge(Vertex p, Vertex cp1, Vertex cp2)
{
	if (cp1.get_x() == cp2.get_x())
	{
		if (cp1.get_y() < cp2.get_y())
			return p.get_x() < cp1.get_x();
		else
			return p.get_x() > cp1.get_x();
	}
	else if (cp1.get_y() == cp2.get_y())
	{
		if (cp1.get_x() < cp2.get_x())
			return p.get_y() > cp1.get_y();
		else
			return p.get_y() < cp1.get_y();
	}
	return false;
}

void triangulate(Polygon *p)
{
	unsigned int i;
	vector<Vertex> result;

	if (p == NULL)
		return;

	p->get_triangles().clear();
	Process(p->get_vertices(), result);

	for (i = 0; i < result.size(); i += 3)
	{
		p->get_triangles().push_back(Triangle(
					result[i],
					result[i+1],
					result[i+2]));
	}
}


///////////////////////////////////////////////////////////////////////////////
//                Given Triangulation Code (Slightly Adjusted)
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const float EPSILON = 0.0000000001f;

/* Function prototypes */
float Area(const vector<Vertex> &contour);
bool InsideTriangle(float Ax, float Ay,
                           float Bx, float By,
                           float Cx, float Cy,
                           float Px, float Py);
bool Snip(const vector<Vertex> &contour,int u,int v,int w,int n,int *V);


float Area(const vector<Vertex> &contour)
{
	int n = contour.size();
	float A=0.0f;

	for(int p=n-1,q=0; q<n; p=q++)
		A+= contour[p].get_x()*contour[q].get_y() - contour[q].get_x()*contour[p].get_y();
	return A*0.5f;
}

bool InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py)
{
	float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;

	ax = Cx - Bx;  ay = Cy - By;
	bx = Ax - Cx;  by = Ay - Cy;
	cx = Bx - Ax;  cy = By - Ay;
	apx= Px - Ax;  apy= Py - Ay;
	bpx= Px - Bx;  bpy= Py - By;
	cpx= Px - Cx;  cpy= Py - Cy;

	aCROSSbp = ax*bpy - ay*bpx;
	cCROSSap = cx*apy - cy*apx;
	bCROSScp = bx*cpy - by*cpx;

	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};


bool Snip(const vector<Vertex> &contour,int u,int v,int w,int n,int *V)
{
	int p;
	float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

	Ax = contour[V[u]].get_x();
	Ay = contour[V[u]].get_y();

	Bx = contour[V[v]].get_x();
	By = contour[V[v]].get_y();

	Cx = contour[V[w]].get_x();
	Cy = contour[V[w]].get_y();

	if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;

	for (p=0;p<n;p++)
	{
		if( (p == u) || (p == v) || (p == w) )
			continue;
		Px = contour[V[p]].get_x();
		Py = contour[V[p]].get_y();
		if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py))
			return false;
	}

	return true;
}


bool Process(const vector<Vertex> &contour,vector<Vertex> &result)
{
	/* allocate and initialize list of Vertices in polygon */

	int n = contour.size();
	if ( n < 3 )
		return false;

	int *V = new int[n];

	/* we want a counter-clockwise polygon in V */

	if ( 0.0f < Area(contour) )
		for (int v=0; v<n; v++)
			V[v] = v;
	else
		for(int v=0; v<n; v++)
			V[v] = (n-1)-v;

	int nv = n;

	/*  remove nv-2 Vertices, creating 1 triangle every time */
	int count = 2*nv;   /* error detection */

	for(int m=0, v=nv-1; nv>2; )
	{
		/* if we loop, it is probably a non-simple polygon */
		if (0 >= (count--))
		{
			//** Triangulate: ERROR - probable bad polygon!
			return false;
		}

		/* three consecutive vertices in current polygon, <u,v,w> */
		int u = v  ; if (nv <= u) u = 0;     /* previous */
		v = u+1; if (nv <= v) v = 0;     /* new v    */
		int w = v+1;
		if (nv <= w) w = 0;     /* next     */

		if ( Snip(contour,u,v,w,nv,V) )
		{
			int a,b,c,s,t;

			/* true names of the vertices */
			a = V[u]; b = V[v]; c = V[w];

			/* output Triangle */
			result.push_back( contour[a] );
			result.push_back( contour[b] );
			result.push_back( contour[c] );

			m++;

			/* remove v from remaining polygon */
			for(s=v,t=v+1;t<nv;s++,t++) {
				V[s] = V[t];
			}
			nv--;

			/* resest error detection counter */
			count = 2*nv;
		}
	}
	delete[] V;

	return true;
}

