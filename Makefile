# A simple Makefile

CXX = g++
CPPFLAGS = -g -Wall
LDLIBS = -lGL -lglut

all:
	$(CXX) $(CPPFLAGS) project1.cpp $(LDLIBS) 

