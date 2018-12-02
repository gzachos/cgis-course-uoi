# A simple Makefile

CXX = g++
CPPFLAGS = -g -Wall
LDLIBS = -lGL -lGLU -lglut

const-extrusion-length:
	$(CXX) $(CPPFLAGS) -UVARIABLE_EXTRUSION_LENGTH project1.cpp $(LDLIBS)

var-extrusion-length:
	$(CXX) $(CPPFLAGS) -DVARIABLE_EXTRUSION_LENGTH project1.cpp $(LDLIBS)

