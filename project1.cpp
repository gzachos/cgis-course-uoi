#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* Global Definitions */
#define WINDOW_WIDTH		600
#define WINDOW_HEIGHT		500
#define DISTANCE(x,y)		abs((x)-(y))
#define MIN_VERTEX_NUM		3

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
enum option_e {MENU_EXIT = 2*NUM_OF_COLORS, MENU_POLYGON, MENU_MOVE_VERTEX, MENU_CLIPPING};
enum state_e  {NORMAL, DRAWING_POLYGON, MOVING_VERTEX, CLIPPING};

typedef struct color_s
{
	GLubyte r;
	GLubyte g;
	GLubyte b;
} color_t;

using namespace std;

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
	Vertex(int, int);
	Vertex(const Vertex&);
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
	update(x,y);
}

Vertex::Vertex(const Vertex &v)
{
	update(v.x, v.y);
}

void Vertex::update(int x, int y)
{
	this->x = x;
	this->y = y;
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

class Polygon
{
public:
	vector<Vertex> vertices;
	color_e line_clr;
	color_e fill_clr;
	Polygon(color_e, color_e);
	Vertex *contains(Vertex v);
	void maxminy(int *maxp, int *minp);
	void scanline_fill(void);
	vector<Vertex> get_intersection_points(Vertex v0, Vertex v1);
	vector<Vertex> get_intersection_points_cleanedup(Vertex v0, Vertex v1);
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

void Polygon::scanline_fill(void)
{
	int maxy, miny, y;
	unsigned int i;
	vector<Vertex> points;

	if (vertices.size() < MIN_VERTEX_NUM)
		return;

	maxminy(&maxy, &miny);
	glLineWidth(1.0f);
	glColor3ub(COLOR_TO_RGB(this->fill_clr));
	for (y = maxy; y >= miny; y--)
	{
		points = get_intersection_points_cleanedup(Vertex(0, y), Vertex(WINDOW_WIDTH, y));
		if (points.size() == 0)
			continue;
#undef DEBUG

#ifdef DEBUG
		if (points.size() % 2 != 0)
		{
			cout << "ERROR: ";
			cout << "y: " <<  y << ": " << points.size() << endl;
			glColor3ub(COLOR_TO_RGB(RED));
			vector<Vertex> ps = get_intersection_points(Vertex(0, y), Vertex(WINDOW_WIDTH, y));
			cout << "Points: " << endl;
			for (unsigned int i = 0; i < ps.size(); i++)
				cout << ps[i];
			cout << endl;
			cout << "Vertices: " << endl;
			for (unsigned int i = 0; i < vertices.size(); i++)
				cout << vertices[i];
			cout << endl << endl;

		}
#endif
		for (i = 0; i < points.size()-1; i += 2)
		{
			glVertex3i(points[i].x, y, 0);
			glVertex3i(points[i+1].x, y, 0);
		}
#if DEBUG
		glColor3ub(COLOR_TO_RGB(this->fill_clr));
#endif
	}
}

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


/* Function Prototypes */
void window_display(void);
void resize_window(int width, int height);
void menu_handler(int value);
void keyboard_event_handler(unsigned char key, int x, int y);
void mouse_event_handler(int button, int state, int x, int y);
inline void leave_current_state(void);
void finalize(void);
void draw_polygons(void);
inline float crossproduct(Vertex v1, Vertex v2);
bool intersecting_polygon(Polygon *p);
Vertex *intersection(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, bool ignore_edge_points);
bool sort_by_x(Vertex v0, Vertex v1);

/* Global Data */
int window_id, state = NORMAL;
vector<Polygon> polygons;
color_e line_clr = BLACK, fill_clr = WHITE;
Vertex *cmin, *cmax;
bool show_triangles = false;

int main(int argc, char **argv)
{
	int action_smenuid, lineclr_smenuid, fillclr_smenuid;

	// Initialize GLUT lib and negotiate a session with the window system
	glutInit(&argc, argv);
	// Set initial display mode to RGBA
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
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


void window_display()
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0 );
	draw_polygons();
	glutSwapBuffers();
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
		default:
			if (value >= BLACK && value < BLACK + NUM_OF_COLORS)
				line_clr = (color_e) value;
			else if (value >= BLACK + NUM_OF_COLORS && value < (NUM_OF_COLORS<<1))
				fill_clr = (color_e) (value - NUM_OF_COLORS);
	}
}

void keyboard_event_handler(unsigned char key, int x, int y)
{
	if (key == 'T' || key == 't')
	{
		show_triangles = !show_triangles;
//		cout << "Show triangles: " << ((show_triangles == true) ? "yes" : "no") << endl;
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
//			cout << x << "," << y << " - " << endl;
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
				moving_vertex->update(x, y);
				if (intersecting_polygon(&(polygons[editing_polygon_index])) == true)
				{
					moving_vertex->update(old_vertex->x, old_vertex->y);
					cerr << "Cannot move vertex. Resulted in an intersecting polygon!" << endl;
				}
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
//					cout << "In range of: (" << moving_vertex->x << ", " <<
//						moving_vertex->y << ")" << endl;
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
			else if (*v0 < *v1)
			{
				cmin = v0;
				cmax = v1;
			}
			else
			{
				cmin = v1;
				cmax = v0;
			}
			if (cmin != NULL && cmax != NULL)
			{
				cout << "(" << cmin->x << "," << cmin->y << ") - (" << cmax->x << "," << cmax->y << ")" << endl;
				// TODO call the Hodgeman-Sutherland polygon clipping algorithm function
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
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (vector<Polygon>::iterator p = polygons.begin(); p != polygons.end(); p++)
	{
		p->scanline_fill();
		glColor3ub(COLOR_TO_RGB(p->line_clr));
		for (unsigned int i = 0, j; i < p->vertices.size(); i++)
		{
			glVertex3i(p->vertices[i].x, p->vertices[i].y, 0);
			j = (i + 1) % p->vertices.size();
			glVertex3i(p->vertices[j].x, p->vertices[j].y, 0);
//			cout << "(" << p->vertices[i].x << "," << p->vertices[i].y << ") ";
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

bool sort_by_x(Vertex v0, Vertex v1)
{
	return (v0.x < v1.x);
}
