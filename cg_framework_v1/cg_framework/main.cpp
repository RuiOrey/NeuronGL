#include <Gl/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>


/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Ver?nica Orvalho, Bruno Oliveira, 2012
	
	Step 2: 
	GLEW initialization;
	
	GLEW is library that allows an easy access to OpenGL extensions. Microsoft Windows only exposes OpenGL 1.1. The remaining functions must be obtained from the driver itself, and GLEW does that automatically.

	new code is marker as !![NEW]!!
	
	output: blank screen
*/


bool inited = false; //Have we done initialization?


/* 
	Error checking function:

	It checks for error in the OpenGL pipeline;
*/

void checkError(const char *functionName)
{
   GLenum error;
   while (( error = glGetError() ) != GL_NO_ERROR) {
      fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
   }
}

/*
	Prints the information regarding the OpenGL context
*/

void dumpInfo(void)
{
   printf ("Vendor: %s\n", glGetString (GL_VENDOR));
   printf ("Renderer: %s\n", glGetString (GL_RENDERER));
   printf ("Version: %s\n", glGetString (GL_VERSION));
   printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
   checkError ("dumpInfo");
}

/*
	Initialization function
	
	All initialization procedures should be performed here.
*/

void init(void) 
{
	
	/*
	
	!![NEW]!!
	
	GLEW initialization.
	activate GLEW experimental features to have access to the most recent OpenGL, and then call glewInit.
	it is important to know that this must be done only when a OpenGL context already exists, meaning, in this case, glutCreateWindow has already been called.
	
	*/
	
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
	} else {
		if (GLEW_VERSION_3_3)
		{
			fprintf(stderr, "Driver supports OpenGL 3.3\n");
		}
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	/*
	end !![NEW]!!
	*/
	
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Defines the clear color, i.e., the color used to wipe the display
	checkError("init");
}


/*
	[FREEGLUT]
	
	display function;
	This function is called to paint the display. This is where the drawing functions should be placed.
	Why is the init inside display? Because some drivers require that the display window to be visible to work correctly; therefore, the initialization is done once, but on the first call to 
	display, ensuring that the window is visible;
*/
	
void display(void)
{
	if (!inited) {
		init();
		inited = true;
	}
	
	glClear(GL_COLOR_BUFFER_BIT); //Clears the display with the defined clear color
	

	glFlush(); //Instructes OpenGL to finish all rendering operations
	glutSwapBuffers(); //Swaps the display in a double buffering scenario. In double buffering, rendering is done in a offline buffer (not directly on the screen); this avoid flickering 
	checkError ("display");
}

/*
	[FREEGLUT]
	This function is called whenever the user changes the size of the display window;
	
	The size of the OpenGL's viewport is mapped to the same size of the window.
*/

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   checkError ("reshape");
}


/*
	[FREEGLUT]
	Keyboard handling function
*/
	
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         exit(0);
         break;
   }
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); //Double buffer; Color display

	glutInitContextVersion(3, 3); //OpenGL 3.3
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG); //Forward compatible profile (Old Opengl (pre-3.x) functions can be used, but SHOULDN'T) 

	glutInitWindowSize(800, 600); 
	glutCreateWindow(argv[0]);

	dumpInfo();
	
	/*
		Sets the specific functions
	*/
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	checkError ("main");
	glutMainLoop(); //starts processing


	return 0;
}

