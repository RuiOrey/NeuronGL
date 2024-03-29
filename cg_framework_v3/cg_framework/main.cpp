#include <Gl/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/*
	Computer Graphics Course - FCUP (LCC, MCC)
	Ver?nica Orvalho, Bruno Oliveira, 2012
	
	Step 12:
	Model matrix transformation

	Uncomment lines 1. and 2. in the vertex shader
		
	new code is marker as !![NEW]!!
	
	output: a cube rotated and in a diferent place
*/


bool inited = false; //Have we done initialization?

/*
	array of vertices that hold the geometry information. in this case, three vertice, forming a triangle
	please bear in mind that each vertice is a triple: (x, y, z)
	
	vao: will hold the VAO identifier (usually one per object)
	geomId: will hold the VBO identifier (one per attribute: position, normal, etc.)
*/


/*
	Indices for the geometry. Each number corresponds to a position in the vertices array. 
	So, the first triangle is corresponds to vertice[0], vertice[1], and vertice[2], the second to vertice[0], vertice[2] and vertice[3], and so on and so forth.
	Drawing with indices allows to drastically reduce the amount of geometry submitted to the graphics card.

	Pay attention to the vertices array, since it only holds unique vertices now.
*/

GLubyte indices[] =
{
	0, 1, 2,
	0, 2, 3,
	1, 5, 2,
	5, 6, 2,
	5, 7, 4,
	5, 6, 7,
	0, 4, 3,
	3, 4, 7,
	0, 1, 4,
	4, 1, 5,
	3, 7, 2,
	7, 2, 6
};

GLfloat vertices[] = 
{
	-1.0f, -1.0f, 1.0f, //llf
	1.0f, -1.0f, 1.0f, //lrf
	1.0f, 1.0f, 1.0f, //urf
	-1.0f, 1.0f, 1.0f, //ulf

	-1.0f, -1.0f, -1.0f, //llb
	1.0f, -1.0f, -1.0f, //lrb
	1.0f, 1.0f, -1.0f, //urb
	-1.0f, 1.0f, -1.0f, //ulb

}; 

GLfloat colors[] = 
{
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,

};

GLuint vao;
GLuint geomId;
GLuint colorId;

/*
	variables to hold the shaders related identifiers

	vertexShaderId: will hold the vertex shader identifier
	fragShaderId: will hold the fragment shader identifier
	programId: will hold the program identifier
*/
	
GLuint vertexShaderId;
GLuint fragShaderId;
GLuint programId;
	

/*
	Perspective and camera related variables.

	perspectiveMatrix: perspective matrix. this is a GLM type, representing a 4x4 matrix
	cameraMatrix: camera or view matrix.
	cameraPos: camera initial position
	cameraView: camera view direction
	cameraUp: camera up vector
	angle: initial viewing angle (-PI/2)
	velocity: camera's rotation and moving velocity

*/

glm::mat4 perspectiveMatrix;
glm::mat4 cameraMatrix;

glm::vec3 cameraPos(0.0f, 0.0f, 15.0f);
glm::vec3 cameraView(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float angle = -1.57079633f;

const float velocity = 0.25f;


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
	creates the program
	1. Generate the program's id
	2. Attach shader
	3. Repeat 2. until all shaders attached
	4. Link the program
	5. Check for errors
	
*/

void createAndCompileProgram(GLuint vertexId, GLuint fragId, GLuint *programId)
{
	*programId = glCreateProgram(); //1.
	glAttachShader(*programId, vertexId); //2. Attach the shader vertexId to the program programId
	glAttachShader(*programId, fragId); //2. Attach the shader fragId to the program programId

	glLinkProgram (*programId);//4.

	//5. Until the end of the if clause, is to check for COMPILE errors, and only for these. *not* related with the checkError procedure
	GLint status;
	glGetProgramiv (*programId, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		char *infoLog;
		glGetProgramiv (*programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		infoLog = new char[infoLogLength];
		glGetProgramInfoLog (*programId, infoLogLength, NULL, infoLog);
		fprintf (stderr, "link log: %s\n", infoLog);
		delete [] infoLog;
	}
	checkError ("createAndCompileProgram");
}

/*
	creates the shader in OpenGL and compiles it.
	
	1. Generate the shader id
	2. Set the shader's source
	3. Compile the shader
	4. Check for errors
*/

void createAndCompileShader(GLuint* id, GLenum type, GLsizei count, const char **shaderSource)
{
	*id = glCreateShader(type); //1. create the shader with type. (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER. others will follow in the future)

	glShaderSource(*id, count, shaderSource, NULL); //2. the shader's source. *id: shader's id; count: memory size of the contents; shaderSource: shader contents; NULL ;)
	
	glCompileShader(*id); //3.

	//4. Until the end of the if clause, is to check for COMPILE errors, and only for these. *not* related with the checkError procedure
	GLint status;
	glGetShaderiv (*id, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		char *infoLog;
		glGetShaderiv (*id, GL_INFO_LOG_LENGTH, &infoLogLength);
		infoLog = new char[infoLogLength];
		glGetShaderInfoLog (*id, infoLogLength, NULL, infoLog);
		fprintf (stderr, "compile log: %s\n", infoLog);
		delete [] infoLog;
	}
	
	checkError ("createAndCompileShader");

}
/*
	loads the shader in file.
	1. load the file from disk into a memory block
	2. create the shader
	
*/

void loadShader (char *file, GLuint *id, GLenum type) 
{
	//Open e load  shader file
	FILE* f = fopen(file, "r");
	if (!f) {
		fprintf(stderr, "Unable to open shader file: %s", file);
		return;
	}

	fseek(f, 0, SEEK_END);

	int fileSize = ftell(f);
	rewind(f);

	char *fileContents = new char[fileSize];
	memset(fileContents, 0, fileSize);
	fread(fileContents, sizeof(char), fileSize, f);
	//up until here is to load the contents of the file

	const char* t = fileContents;
	
	//2. create the shader. arguments (future) shader id, type of shader, memory size of the file contents, file contents
	createAndCompileShader(id, type, sizeof(fileContents)/sizeof(char*), &t);

	//cleanup
	fclose(f);
	delete [] fileContents;
}
/*
	loads two shader files, one vertex shader and one fragment shader, and creates the program.
	the arguments are the path to the files, including file name.
*/

void loadShaderProgram(char* vertexFile, char* fragmentFile)
{
	//load each shader seperately. arguments: the file, (future) shader id, shader type
	
	loadShader(vertexFile, &vertexShaderId, GL_VERTEX_SHADER); 
	loadShader(fragmentFile, &fragShaderId, GL_FRAGMENT_SHADER);
	
	//one the shaders loaded, create the program with the two shaders. arguments: vertex shader id, fragment shader id, (future) program id
	createAndCompileProgram(vertexShaderId, fragShaderId, &programId);
}

/*
	Geometry initialization routine.
	
	1. Generate a VAO that holds that matchs the *ATTRIBUTES* (vertex position, normal, etc) to vertex buffer objects (VBO)(which hold the actual information)
	2. Active the VAO
	3. Active attribute 0 (first attribute in the vertex shader)
	4. Generate the VBO
	5. Activate the VBO
	6. Assign the geometry (the vertices variable previously defined) to the VBO (copying it to the graphics card's memory)
	7. Assign this VBO to the attribute
	8. Repeat steps 3-7 for remaining attributes
	9. Reset OpenGL's state
*/
	
void initGeometry()
{
	glGenVertexArrays(1, &vao); //1.
	glBindVertexArray(vao); //2.
	glEnableVertexAttribArray(0); //3.
	glGenBuffers(1, &geomId); //4.
	glBindBuffer(GL_ARRAY_BUFFER, geomId); //5.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //6. GL_ARRAY_BUFFER: the type of buffer; sizeof(vertices): the memory size; vertices: the pointer to data; GL_STATIC_DRAW: data will remain on the graphics card's memory and will not be changed
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //7. 0: the *ATTRIBUTE* number; 3: the number of coordinates; GL_FLOAT: the type of data; GL_FALSE: is the data normalized? (usually it isn't), 0: stride (forget for now); 0: data position (forget for now)

	glEnableVertexAttribArray(1);
	glGenBuffers(1, &colorId);
	glBindBuffer(GL_ARRAY_BUFFER, colorId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	checkError("initBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, 0); //9.
	glBindVertexArray(0); //9.
}

/*
	Initialization function
	
	All initialization procedures should be performed here.
*/

void init(void) 
{
	
	/*
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
	
	loadShaderProgram("../shaders/vertex_shader.vs", "../shaders/frag_shader.fs");
	initGeometry();
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Defines the clear color, i.e., the color used to wipe the display
	glEnable(GL_DEPTH_TEST);
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
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the display with the defined clear color

	
	glUseProgram(programId);

	GLuint loc = glGetUniformLocation(programId, "pMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&perspectiveMatrix[0]);

	loc = glGetUniformLocation(programId, "vMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&cameraMatrix[0]);

	/*
		!![NEW]!!
		Get the location of the model matrix in the shader
	*/
		
	loc = glGetUniformLocation(programId, "mMatrix");

	/* 
		end !![NEW]!!
	*/
	glBindVertexArray(vao);

	/*
		!![NEW]!!
		* Create a translation matrix, which translates 5 units to the left
		* Create a rotation matrix, which rotates 45 degrees in the YY axis

		note: these glm functions allow to concatenate matrices on-the-fly, passing the current matrix as the first argument.
		However, we opt for calculating each matrix independently and post-concatenate for ease of understanding.
	*/
	
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0), glm::vec3(-5.0f, 0.0f, 0.0f));
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), 45.0f, glm::vec3(1.0f, 1.0f, 0.0f)); 

	/* 
		Apply translation to the rotation
	*/
	glm::mat4 model = translateMatrix * rotateY;

	/*
		Pass the model matrix to the shader
	*/
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&model[0]);
	
	/*
		end !![NEW]!!
	*/
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices); //type of geometry; number of indices; type of indices array, indices pointer
	glBindVertexArray(0); //4.
	glUseProgram(0); //4.
	
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

   /* 
   This sets the perspective matrix. There's no need to compute this matrix at each frame. Only when the window changes size
   */

   float aspect = (float)w/(float)h; //aspect ratio.

   perspectiveMatrix = glm::perspective(45.0f, aspect, 1.0f, 1000.0f); //field of view; aspect ratio; z near; z far;

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
		case 'Q':
		case 'q':
			cameraPos.x += sin(angle) * velocity;
			cameraPos.z -= cos(angle) * velocity;
			break;
		case 'W':
		case 'w':
			cameraPos.x -= sin(angle) * velocity;
			cameraPos.z += cos(angle) * velocity;
			break;
   }
}


/*
	[FREEGLUT]
	Special keys handling function (for "normal" keys, use the previously defined function)

	This function not only handles the special keys, but also controls the camera's position and rotation angle based on the input.	
*/

void keyboardSpecialKeys(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_LEFT:
			angle -= velocity/10;
			break;
		case GLUT_KEY_RIGHT:
			angle += velocity/10;
			break;
		case GLUT_KEY_UP:
			cameraPos.x += cos(angle) * velocity;
			cameraPos.z += sin(angle) * velocity;
			break;
		case GLUT_KEY_DOWN:
			cameraPos.x -= cos(angle) * velocity;
			cameraPos.z -= sin(angle) * velocity;
			break;
   }
}

/* 
	Camera setup function.
	This creates the camera or view matrix based on the position of the player and the camera type.
	In this case, this we are going to create a first person camera.

	note: the lookAt function from GLM works the same as the *old* gluLookAt function
*/

void setupCamera(void)
{

	//Define the view direction based on the camera's position
	cameraView.x = cameraPos.x + cos(angle);
	cameraView.y = cameraPos.y;
	cameraView.z = cameraPos.z + sin(angle);


	//Creates the view matrix based on the position, the view direction and the up vector
	cameraMatrix = glm::lookAt(cameraPos,
								cameraView,
								cameraUp);
}

/* 
This is a function to be issued on every cycle.

It performs all that is needed for an interactive simulation.
*/

void mainLoop(void)
{
	setupCamera();
	display();
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
	glutSpecialFunc(keyboardSpecialKeys);
	glutIdleFunc(mainLoop);

	checkError ("main");
	glutMainLoop(); //starts processing


	return 0;
}

