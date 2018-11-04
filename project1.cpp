#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* Global Definitions */
#define WINDOW_WIDTH	600
#define WINDOW_HEIGHT	500

enum option_e {MENU_EXIT};
enum state_e  {NORMAL};

using namespace std;

/* Function Prototypes */
void window_display(void);
void menu_handler(int value);
void finalize(void);
void resize_window(int width, int height);

/* Global Data */
int window_id;


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
	// Disable window resizing
	glutReshapeFunc(resize_window);

	// Create Menu and Sub-menus to be displayed on right click
	action_smenuid = glutCreateMenu(&menu_handler);
	glutAddMenuEntry("Exit", MENU_EXIT);

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
	glFlush();
}


void menu_handler(int value)
{
//	cout << "menu_handler: " << value << endl;
	switch (value)
	{
		case MENU_EXIT:
			finalize();
			break;
	}
}


void resize_window(int width, int height)
{
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
}


void finalize(void)
{
	glutDestroyWindow(window_id);
	exit(EXIT_SUCCESS);
}

