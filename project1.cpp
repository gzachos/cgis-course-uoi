#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

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
enum option_e {MENU_EXIT = 2*NUM_OF_COLORS, MENU_POLYGON, MENU_MOVE_VERTEX, MENU_CLIPPING, MENU_EXTRUDE};
enum state_e  {NORMAL, DRAWING_POLYGON, MOVING_VERTEX, CLIPPING, EXTRUSION};
enum clipstate_e {CLIPPING_START, CLIPPING_END};

typedef struct color_s
{
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
public:
	GLint x;
	GLint y;
	GLint z = 0;
	Vertex(int, int);
	Vertex(int, int, int);
	Vertex(const Vertex&);
	void update(int, int, int);
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
	update(x,y,0);
}

Vertex::Vertex(int x, int y, int z)
{
	update(x,y,z);
}

Vertex::Vertex(const Vertex &v)
{
	update(v.x, v.y, v.z);
}

void Vertex::update(int x, int y, int z)
{
	this->x = x;
	this->y = y;
	this->z = z;
//	cout << "(" << x << " " << y << ") [" << x << " " << y << "]" << endl << endl;
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
public:
	Vertex v0;
	Vertex v1;
	Vertex v2;
	Triangle(Vertex v0, Vertex v1, Vertex v2);
	bool inside_triangle(Vertex v);
};

Triangle::Triangle(Vertex v0, Vertex v1, Vertex v2)
	: v0(v0), v1(v1), v2(v2)
{
	// nothing else to do here!
};

inline float crossproduct(Vertex v1, Vertex v2);

bool Triangle::inside_triangle(Vertex v)
{
	float a, b, denom = crossproduct(v1, v2);
	a = (crossproduct(v, v2) - crossproduct(v0, v2)) / denom;
	b = -(crossproduct(v, v1) - crossproduct(v0, v1)) / denom;
	return (a >= 0 && b >= 0 && a+b < 1);
}

class Polygon
{
public:
	vector<Vertex> vertices;
	vector<Triangle> triangles;
	color_e line_clr;
	color_e fill_clr;
	Polygon(color_e, color_e);
	Vertex *contains(Vertex v);
	void maxminy(int *maxp, int *minp);
#if 0
	vector<Vertex> get_intersection_points(Vertex v0, Vertex v1);
	vector<Vertex> get_intersection_points_cleanedup(Vertex v0, Vertex v1);
#endif
};

Polygon::Polygon(color_e line, color_e fill)
{
	line_clr = line;
	fill_clr = fill;
}

Vertex *Polygon::contains(Vertex v)
{
	unsigned int i;
	for (i = 0; i < vertices.size(); i++)
		if (vertices[i] == v)
			return &(vertices[i]);
	for (i = 0; i < vertices.size(); i++)
		if (vertices[i].in_x_range(v.x, v.y, 3))
			return &(vertices[i]);
	return NULL;
}

void Polygon::maxminy(int *maxp, int *minp)
{
	int min, max;
	min = max = vertices[0].y;
	for (unsigned int i = 1; i < vertices.size(); i++)
	{
		if (vertices[i].y > max)
			max = vertices[i].y;
		if (vertices[i].y < min)
			min = vertices[i].y;
	}
	*maxp = max; // Reduces memory accesses (dereferencig)
	*minp = min;
}

#if 0
vector<Vertex> Polygon::get_intersection_points(Vertex v1, Vertex v2)
{
	vector<Vertex> points;
	unsigned int vnum = vertices.size();
	Vertex *ipp;

	// Function prototype is required here too
	Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, bool ignore_edge_points);

	for (unsigned int i = 0; i < vnum; i++)
	{
		/*
		 * Intersection points of collinear edges are ignored
		 */
		ipp = intersection(&v1, &v2, &(vertices[i]), &(vertices[(i+1) % vnum]), false);
		/*
		 * Each intersection point that is also a polygon vertex
		 * is added twice in the points list.
		 */
		if (ipp != NULL)
			points.push_back(*ipp);
	}
	return points;
}

vector<Vertex> Polygon::get_intersection_points_cleanedup(Vertex v0, Vertex v1)
{
	bool sort_by_x(Vertex v0, Vertex v1);
	vector<Vertex> points = get_intersection_points(v0, v1);
	Vertex *curr_vp, *prev_vp, *next_vp;
	unsigned int vnum = vertices.size(), currindex;

	sort(points.begin(), points.end(), sort_by_x);

	/*
	 * At this point of time we have to sort all points and
	 * remove double entries as required by the scanline filling
	 * algorithm.
	 */
	for (vector<Vertex>::iterator pi = points.begin(); pi < points.end(); pi++)
	{
		if ((curr_vp = contains(*pi)) != NULL)
		{
			currindex = curr_vp - &(vertices[0]);
			prev_vp = &(vertices[(currindex == 0) ? (vnum-1) : (currindex-1)]);
			next_vp = &(vertices[(currindex+1) % vnum]);
			/*
			 * If the two edges that meet in (pi->x, pi->y) are both
			 * above or below the line y = v0.y = v1.y then (pi->x, pi->y)
			 * should be (and actually are) twice in the points list.
			 * In the opposite case, we should remove the second Vertex instance.
			 */
			if (!((prev_vp->y > curr_vp->y && next_vp->y > curr_vp->y) ||
					(prev_vp->y < curr_vp->y && next_vp->y < curr_vp->y))) // Equality?
			{
				points.erase(pi++);
			}
		}
	}
	return points;
}

#endif

/* Function Prototypes */
void window_display(void);
void resize_window(int width, int height);
void menu_handler(int value);
void keyboard_event_handler(unsigned char key, int x, int y);
void mouse_event_handler(int button, int state, int x, int y);
inline void leave_current_state(void);
void finalize(void);
void draw_polygons(void);
void draw_polygon_bounds(void);
void draw_polygon_quads(void);
void extrude_polygons(void);
void draw_polygon_area(GLint z);
void draw_polygon_triangles(void);
void draw_clipping_polygon(void);
void draw_grid(void);
inline float crossproduct(Vertex v1, Vertex v2);
bool intersecting_polygon(Polygon *p);
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, bool ignore_edge_points);
#if 0
bool sort_by_x(Vertex v0, Vertex v1);
bool sort_by_y(Vertex v0, Vertex v1);
#endif
void sh_clip(Polygon *p);
bool inside_clip_edge(Vertex p, Vertex cp1, Vertex cp2);
void triangulate(Polygon *p);

// Given Triangulation Code function prototype
bool Process(const vector<Vertex> &contour, vector<Vertex> &result);

/* Global Data */
int window_id, state = NORMAL, extrusion_length;
vector<Polygon> polygons;
color_e line_clr = BLACK, fill_clr = WHITE;
Vertex *cmin, *cmax;
bool show_triangles = false, show_clipping_polygon = false;

double posx=WINDOW_WIDTH/2, posy=WINDOW_HEIGHT/2, posz=15.0, lookx=0, looky=1, lookz=0, upx=0, upy=0, upz=1;


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

	lineclr_smenuid = glutCreateMenu(&menu_handler);
	ADD_COLOR_ENTRIES(0);
	fillclr_smenuid = glutCreateMenu(&menu_handler);
	ADD_COLOR_ENTRIES(NUM_OF_COLORS);

	glutCreateMenu(&menu_handler);
	glutAddSubMenu("ACTION", action_smenuid);
	glutAddSubMenu("LINE_COLOR", lineclr_smenuid);
	glutAddSubMenu("FILL_COLOR", fillclr_smenuid);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

#if 0
	// For checking intersection(...);
//	Vertex v1(0,0), v2(600,500), v3(0,500), v4(600,0);
//	Vertex v1(0,0), v2(600,500), v3(600,0), v4(0,500);
	Vertex v1(0,250), v2(600,250), v3(50,250), v4(100,251); // Vertical line segments
//	Vertex v1(0,0), v2(600,500), v3(300,0), v4(600,250); // Parallel line segments
//	Vertex v1(0,0), v2(600,500), v4(300,0), v3(600,250); // Parallel line segments
	if (intersection(&v1, &v2, &v3, &v4, false) != NULL)\
		cout << "INTERSECTS"<< endl;
	else
		cout << "DOESN'T"<< endl;
#else
	// Enter the GLUT event processing loop.
	// This function should never return.
	glutMainLoop();
#endif

	return (EXIT_FAILURE);
}

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
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* projection matrix (camera) */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (state == EXTRUSION)
	{
		glClearDepth(1.0);          // Set background depth to farthest
		glDepthFunc(GL_LESS);   
	  	glEnable(GL_DEPTH_TEST);	// this breaks the triangulation for some reason

		GLfloat aspect = (GLfloat) WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT;
		glViewport(0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, aspect, 1.0, 1000.0);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// glTranslatef(0, 0, -100);
		// glRotatef(40, 1, 1, 0);
		gluLookAt(posx, posy, posz, lookx, looky, lookz, upx, upy, upz);
		// draw_grid();

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
		if (i >= 20)
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
	if (cmin && cmax)
	{
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glColor3ub(COLOR_TO_RGB(RED));
		glVertex3i(cmin->x, cmin->y, 0.0f);
		glVertex3i(cmax->x, cmin->y, 0.0f);
		glColor3ub(COLOR_TO_RGB(BLUE));
		glVertex3i(cmax->x, cmin->y, 0.0f);
		glVertex3i(cmax->x, cmax->y, 0.0f);
		glColor3ub(COLOR_TO_RGB(BLACK));
		glVertex3i(cmax->x, cmax->y, 0.0f);
		glVertex3i(cmin->x, cmax->y, 0.0f);
		glVertex3i(cmin->x, cmax->y, 0.0f);
		glVertex3i(cmin->x, cmin->y, 0.0f);
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
			state = DRAWING_POLYGON;
			polygons.push_back(Polygon(line_clr, fill_clr));
			break;
		case MENU_MOVE_VERTEX:
			state = MOVING_VERTEX;
			break;
		case MENU_CLIPPING:
			state = CLIPPING;
			break;
		case MENU_EXTRUDE:
			state = EXTRUSION;
			cout << "Please provide an extrusion length: ";
			cin >> extrusion_length; // Check for positive or something else?
			cout << "You selected " << extrusion_length << " as the extrusion length." << endl;
			// extrude_polygons();
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
		case 'w':
			// move camera up
			posz+=2.0;
			break;
		case 'S':
		case 's':
			// move camera down
			posz-=2.0;
			break;
		case 'A':
		case 'a':
			// move camera left
			posx+=2.0;
			break;
		case 'D':
		case 'd':
			// move camera right
			posx-=2.0;
			break;
		case 'Q':
		case 'q':
			// move camera right
			posy-=2.0;
			break;
		case 'E':
		case 'e':
			// move camera right
			posy+=2.0;
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
	if (::state == DRAWING_POLYGON)
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			polygons.back().vertices.push_back(Vertex(x, y));
			triangulate(&(polygons.back())); // To allow area coloring
			if (intersecting_polygon(&polygons.back()) == true)
			{
				cerr << "You created an intersecting polygon." <<
					" Nothing to be saved!" << endl;
				polygons.pop_back();
				leave_current_state();
			}
			created_polygon = true;
		}
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		{
			leave_current_state();
			if (created_polygon == true && polygons.back().vertices.size() < MIN_VERTEX_NUM)
			{
				polygons.pop_back();
				cerr << "At least " << MIN_VERTEX_NUM << " vertices are required to" <<
					" create a polygon. Nothing to be saved!" << endl;
			}
			created_polygon = false;
		}
	}
	else if (::state == MOVING_VERTEX)
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		if (mouse_event_count++ == 0 && state == GLUT_UP)
			return; // ignore GLUT_UP generated by menu option selection

		if (state == GLUT_UP)
		{
			if (moving_vertex)
			{
				Vertex *old_vertex = new Vertex(*moving_vertex);
				moving_vertex->update(x, y, 0);
				if (intersecting_polygon(&(polygons[editing_polygon_index])) == true)
				{
					moving_vertex->update(old_vertex->x, old_vertex->y, 0);
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

		for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
			for (unsigned int i = 0; i < p->vertices.size(); i++)
			{
				if (p->vertices[i].in_range(x, y, 10))
				{
					editing_polygon_index = p - polygons.begin();
					moving_vertex = &(p->vertices[i]);
				}
			}
	}
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
				cmin = new Vertex(min(v0->x,v1->x), min(v0->y, v1->y));
				cmax = new Vertex(max(v0->x,v1->x), max(v0->y, v1->y));
				cout << "(" << cmin->x << "," << cmin->y << ") - (" << cmax->x << "," << cmax->y << ")" << endl;
				glutTimerFunc(0, timer_func, CLIPPING_START);
				for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
				{
					sh_clip(&(*p));
					// Triangulate each polygon after clipping
					triangulate(&(*p));
				}
			}
			leave_current_state();
			mouse_event_count = 0;
		}
	}
	else if (::state == EXTRUSION)
	{
		// change to 3D view
		// leave_current_state();
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
	draw_polygon_area(0);
	if (show_triangles == true)
		draw_polygon_triangles();

	if (state == EXTRUSION)
		draw_polygon_quads();
	else
		draw_polygon_bounds();
}

void draw_polygon_bounds(void)
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		glColor3ub(COLOR_TO_RGB(p->line_clr));
		for (unsigned int i = 0, j; i < p->vertices.size(); i++)
		{
			glVertex3i(p->vertices[i].x, p->vertices[i].y, p->vertices[i].z);
			j = (i + 1) % p->vertices.size();
			glVertex3i(p->vertices[j].x, p->vertices[j].y, p->vertices[j].z);
		}
	}
	glEnd();
}

/**
 * Extrudes the polygons to 3D
 * 
 * Draws the original polygon, a copy of the original polygon (with z = extrusion_length)
 * and draws a quad for each edge of the original polygon
 * 
 */
void draw_polygon_quads(void)
{
	glLineWidth(2.0f);
	/* For each polygon */
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		/* 
		 * Original & copy (base & top) 
		 */
		glBegin(GL_LINES);
		glColor3ub(COLOR_TO_RGB(p->fill_clr));
		for (unsigned int i = 0, j; i < p->vertices.size(); i++)
		{
			j = (i + 1) % p->vertices.size();

			Vertex v0 = p->vertices[i];
			Vertex v1 = p->vertices[j];

			/* Base */
			glVertex3i(v0.x, v0.y, v0.z);
			glVertex3i(v1.x, v1.y, v1.z);

			/* Top */
			glVertex3i(v0.x, v0.y, v0.z - extrusion_length);
			glVertex3i(v1.x, v1.y, v0.z - extrusion_length);
			draw_polygon_area(v0.z - extrusion_length);
		}
		glEnd();

		/* 
		 * Sides 
		 */
		glBegin(GL_QUADS);
		glColor3ub(COLOR_TO_RGB(p->line_clr));
		for (unsigned int i = 0, j; i < p->vertices.size(); i++)
		{
			j = (i + 1) % p->vertices.size();

			Vertex v0 = p->vertices[i];
			Vertex v1 = p->vertices[j];

			glVertex3i(v0.x, v0.y, v0.z - extrusion_length);
			glVertex3i(v1.x, v1.y, v1.z - extrusion_length);
			glVertex3i(v0.x, v0.y, v0.z);
			glVertex3i(v1.x, v1.y, v1.z);
		}
		glEnd();
	}
}

void draw_polygon_area(GLint z)
{
	glLineWidth(1.0f);
	glBegin(GL_TRIANGLES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		glColor3ub(COLOR_TO_RGB(p->fill_clr));
		for (unsigned int i = 0; i < p->triangles.size(); i++)
		{
			Triangle *t = &(p->triangles[i]);
			glVertex3i(t->v0.x, t->v0.y, z);
			glVertex3i(t->v1.x, t->v1.y, z);
			glVertex3i(t->v2.x, t->v2.y, z);
		}
	}
	glEnd();
}


void draw_polygon_triangles(void)
{
	glLineWidth(2.0f);
	glColor3ub(COLOR_TO_RGB(GREEN));
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		for (unsigned int i = 0; i < p->triangles.size(); i++)
		{
			Triangle *t = &(p->triangles[i]);
			glBegin(GL_LINES);
			glVertex3i(t->v0.x, t->v0.y, t->v0.z);
			glVertex3i(t->v1.x, t->v1.y, t->v1.z);
			glVertex3i(t->v1.x, t->v1.y, t->v1.z);
			glVertex3i(t->v2.x, t->v2.y, t->v2.z);
			glVertex3i(t->v2.x, t->v2.y, t->v2.z);
			glVertex3i(t->v0.x, t->v0.y, t->v0.z);
			glEnd();
		}
	}
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

//	cout << "Ap1: " << p1 <<"Ap2: " << p2 << "Ap3: " <<p3 <<"Ap4: " << p4 << endl;
	denom = crossproduct(p2-p1, p4-p3);

	l = crossproduct(p3-p1, p4-p3) /  denom;
	k = crossproduct(p1-p3, p2-p1) / -denom;

	if (l >= 0 && l <= 1 && k >= 0 && k <= 1 && denom != 0)
	{
		if (ignore_edge_points && (l == 0 || l == 1 || k == 0 || k == 1))
			return NULL;
#if 0
		cout << "p1: " << p1 <<"p2: " << p2 << "p3: " <<p3 <<"p4: " << p4 << endl;
		cout << "l: " << l << " k: "<< k << " denom: " << denom << endl;
		cout << p1 + ((p2-p1)*l) << endl;
		cout << p3 + (p4-p3)*k;
#endif
		return new Vertex(p1 + (p2-p1)*l);
	}

	return NULL;
}

inline float crossproduct(Vertex v1, Vertex v2)
{
	return ((v2.y * v1.x) - (v2.x * v1.y));
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
						&(p->vertices[(j+1) % vnum]), true) != NULL)
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

#if 0
bool sort_by_x(Vertex v0, Vertex v1)
{
	return (v0.x < v1.x);
}

bool sort_by_y(Vertex v0, Vertex v1)
{
	return (v0.y < v1.y);
}
#endif

void sh_clip(Polygon *p)
{
	if (cmin == NULL || cmax == NULL || p == NULL)
		return;

	Vertex cp[8] = {
		Vertex(0, cmin->y),
		Vertex(WINDOW_WIDTH, cmin->y),
		Vertex(cmax->x, 0),
		Vertex(cmax->x, WINDOW_HEIGHT),
		Vertex(WINDOW_WIDTH, cmax->y),
		Vertex(0, cmax->y),
		Vertex(cmin->x, WINDOW_HEIGHT),
		Vertex(cmin->x, 0)
	};

	vector<Vertex> output_list = p->vertices;
	Vertex *cp0, *cp1, *ip;
//	unsigned int count = 0;

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
//					count--;
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
//				count++;
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
#if 1
	for (unsigned int i = 0; i < output_list.size(); i++)
	{
		if (i == 0)
			cout << "CLIP: " << endl;
		cout << output_list[i];
	}
#endif
	p->vertices = output_list;
}

bool inside_clip_edge(Vertex p, Vertex cp1, Vertex cp2)
{
	if (cp1.x == cp2.x)
	{
		if (cp1.y < cp2.y)
			return p.x < cp1.x;
		else
			return p.x > cp1.x;
	}
	else if (cp1.y == cp2.y)
	{
		if (cp1.x < cp2.x)
			return p.y > cp1.y;
		else
			return p.y < cp1.y;
	}
	return false;
}


void triangulate(Polygon *p)
{
	unsigned int i;
	vector<Vertex> result;

	if (p == NULL)
		return;

	p->triangles.clear();
	Process(p->vertices, result);

	for (i = 0; i < result.size(); i += 3)
	{
		p->triangles.push_back(Triangle(
					result[i],
					result[i+1],
					result[i+2]));
	}
//	cout << "Triangulated: " << p->triangles.size() << endl;
}


///////////////////////////////////////////////////////////////////////////////
//                      Given Triangulation Code (Modified)
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
		A+= contour[p].x*contour[q].y - contour[q].x*contour[p].y;
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

	Ax = contour[V[u]].x;
	Ay = contour[V[u]].y;

	Bx = contour[V[v]].x;
	By = contour[V[v]].y;

	Cx = contour[V[w]].x;
	Cy = contour[V[w]].y;

	if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;

	for (p=0;p<n;p++)
	{
		if( (p == u) || (p == v) || (p == w) )
			continue;
		Px = contour[V[p]].x;
		Py = contour[V[p]].y;
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
