# A simple Makefile

CXX = g++
CPPFLAGS = -g -Wall
LDLIBS = -lGL -lGLU -lglut

all:
	$(CXX) $(CPPFLAGS) project1.cpp $(LDLIBS) 

