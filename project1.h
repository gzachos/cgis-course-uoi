
#ifndef PROJECT1_H

#define PROJECT1_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

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

#endif

