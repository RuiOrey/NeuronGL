#include <Gl/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FreeImage.h>
#include <vector>
#include <iostream>


#include "objloader.h"

/*****************************************************************************************************

						Vector initializer template for initializing vectors 
						with different values

*****************************************************************************************************/
template<typename T>
struct initializer
{
   std::vector<T> items;
   initializer(const T & item) { items.push_back(T(item)); }
   initializer& operator()(const T & item) 
   {
      items.push_back(item);
      return *this;
   }
   operator std::vector<T>&() { return items ; }
};

/****************************************************************************************************/

using namespace std;



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
Computer Graphics Course - FCUP (LCC, MCC)
Ver�nica Orvalho, Bruno Oliveira, 2012

Step 14:
Gouraud ambient + diffuse shading

output: a textured teapot with ambient + diffuse shading
*/


bool inited = false; //Have we done initialization?
int mousex = 0;
int mousey = 0;
bool lbuttonDown = false;

/*
vao: will hold the VAO identifier (usually one per object)
geomId: will hold the VBO identifier (one per attribute: position, normal, etc.)
*/

GLfloat translatex=0.0f;
bool txup=true;
GLfloat translatey=0.0f;
bool tyup=true;
GLfloat translatez=0.0f;
bool tzup=true;

int vaoglobal=0;
bool increment=false;
bool incrementy=true;
float line_vertex[]=
{
    0,0, 3, 100
};


int size=10;
int total=2;
vector<int> elements(size);
vector <glm::mat4> translationsM(size);
vector <glm::mat4> rotationsY(size);



GLuint *vao=(GLuint*) malloc (sizeof (GLuint) * total);
GLuint *geomId=(GLuint*) malloc (sizeof (GLuint) * total);
GLuint *normalsId=(GLuint*) malloc (sizeof (GLuint) * total);
GLuint *texUVId=(GLuint*) malloc (sizeof (GLuint) * total);


GLuint vao2;
GLuint geomId2;
GLuint normalsId2;
GLuint texUVId2;

/**********************************************************************

		CLASS VAO

**********************************************************************/

class Vao {
    GLuint vao;
	GLuint geomId;
	GLuint normalsId;
	GLuint texUVId;
	int vaoID;
  public:
    void initialize_vao (OBJLoader);
    GLuint getvao () {return (vao);}
	GLuint getgeom () {return (geomId);}	
	GLuint getnormals () {return (normalsId);}
	GLuint gettex () {return (texUVId);}
	int getid () {return (vaoID);}
};

void Vao::initialize_vao (OBJLoader object_) {
	vaoID=vaoglobal;
	const float *vertices = object_.getVerticesArray();
	const float *textureCoords = object_.getTextureCoordinatesArray();
	const float *normals = object_.getNormalsArray();
	glGenVertexArrays(1, &vao); //1.
	glBindVertexArray(vao); //2.
	glEnableVertexAttribArray(0); //3.
	glGenBuffers(1, &geomId); //4.
	glBindBuffer(GL_ARRAY_BUFFER, geomId); //5.
	glBufferData(GL_ARRAY_BUFFER, object_.getNVertices() * 3 * sizeof(float), vertices, GL_STATIC_DRAW); //6. GL_ARRAY_BUFFER: the type of buffer; sizeof(vertices): the memory size; vertices: the pointer to data; GL_STATIC_DRAW: data will remain on the graphics card's memory and will not be changed
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //7. 0: the *ATTRIBUTE* number; 3: the number of coordinates; GL_FLOAT: the type of data; GL_FALSE: is the data normalized? (usually it isn't), 0: stride (forget for now); 0: data position (forget for now)

	glEnableVertexAttribArray(1);
	glGenBuffers(1, &texUVId);
	glBindBuffer(GL_ARRAY_BUFFER, texUVId);
	glBufferData(GL_ARRAY_BUFFER, object_.getNVertices() * 2 * sizeof(float), textureCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &normalsId);
	glBindBuffer(GL_ARRAY_BUFFER, normalsId);
	glBufferData(GL_ARRAY_BUFFER, object_.getNVertices() * 3 * sizeof(float), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	checkError("initBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, 0); //9.
	glBindVertexArray(0); //9.
	vaoglobal++;
}

/**********************************************************************

		END CLASS VAO!

**********************************************************************/

/*
variables to hold the shaders related identifiers

vertexShaderId: will hold the vertex shader identifier
fragShaderId: will hold the fragment shader identifier
programId: will hold the program identifier
*/

GLuint vertexShaderId;
GLuint fragShaderId;
GLuint programId;

GLuint baseTextureId;
GLuint specularTextureId;

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


GLfloat lightDir[] = {1.0f, -0.5f, 2.0};
GLfloat lightIntensity[] = {1.9f, 1.9f, 1.9f, 1.0f};

GLfloat ambientComponent[] = {0.4f, 0.4f, 0.4f, 1.0f};
GLfloat diffuseColor[] = {1.0f, 1.0f, 1.0f, 1.0f};


/* objects[0]=new OBJLoader("../models/haywagon/haywagon_tris.obj"); 
OBJLoader object1("../models/textured_cube.obj");
OBJLoader object2("../models/cube.obj"); */

/*****************************************************************************************************

								Vector with all object models

*****************************************************************************************************/

vector <OBJLoader> objects(initializer<OBJLoader>(OBJLoader("../models/cube.obj"))(OBJLoader("../models/textured_cube.obj")));





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
	//Vao TesteVao;

	const float *vertices;
	const float *textureCoords; 
	const float *normals;

	for (int i=0;i<total;i++){

	vertices= objects[i].getVerticesArray();
	textureCoords=objects[i].getTextureCoordinatesArray();
	normals=objects[i].getNormalsArray();
	glGenVertexArrays(1, &vao[i]); //1.
	glBindVertexArray(vao[i]); //2.
	glEnableVertexAttribArray(0); //3.
	glGenBuffers(1, &geomId[i]); //4.
	glBindBuffer(GL_ARRAY_BUFFER, geomId[i]); //5.
	glBufferData(GL_ARRAY_BUFFER, objects[i].getNVertices() * 3 * sizeof(float), vertices, GL_STATIC_DRAW); //6. GL_ARRAY_BUFFER: the type of buffer; sizeof(vertices): the memory size; vertices: the pointer to data; GL_STATIC_DRAW: data will remain on the graphics card's memory and will not be changed
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //7. 0: the *ATTRIBUTE* number; 3: the number of coordinates; GL_FLOAT: the type of data; GL_FALSE: is the data normalized? (usually it isn't), 0: stride (forget for now); 0: data position (forget for now)

	glEnableVertexAttribArray(1);
	glGenBuffers(1, &texUVId[i]);
	glBindBuffer(GL_ARRAY_BUFFER, texUVId[i]);
	glBufferData(GL_ARRAY_BUFFER, objects[i].getNVertices() * 2 * sizeof(float), textureCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &normalsId[i]);
	glBindBuffer(GL_ARRAY_BUFFER, normalsId[i]);
	glBufferData(GL_ARRAY_BUFFER, objects[i].getNVertices() * 3 * sizeof(float), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	checkError("initBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, 0); //9.
	glBindVertexArray(0); //9.
	}

	// TEST 
	/*
	const float *vertices2 = objects[1].getVerticesArray();
	const float *textureCoords2 = objects[1].getTextureCoordinatesArray();
	const float *normals2 = objects[1].getNormalsArray();

	glGenVertexArrays(1, &vao2); //1.
	glBindVertexArray(vao2); //2.
	glEnableVertexAttribArray(0); //3.
	glGenBuffers(1, &geomId2); //4.
	glBindBuffer(GL_ARRAY_BUFFER, geomId2); //5.
	glBufferData(GL_ARRAY_BUFFER, objects[1].getNVertices() * 3 * sizeof(float), vertices2, GL_STATIC_DRAW); //6. GL_ARRAY_BUFFER: the type of buffer; sizeof(vertices): the memory size; vertices: the pointer to data; GL_STATIC_DRAW: data will remain on the graphics card's memory and will not be changed
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //7. 0: the *ATTRIBUTE* number; 3: the number of coordinates; GL_FLOAT: the type of data; GL_FALSE: is the data normalized? (usually it isn't), 0: stride (forget for now); 0: data position (forget for now)

	glEnableVertexAttribArray(1);
	glGenBuffers(1, &texUVId2);
	glBindBuffer(GL_ARRAY_BUFFER, texUVId2);
	glBufferData(GL_ARRAY_BUFFER, objects[1].getNVertices() * 2 * sizeof(float), textureCoords2, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &normalsId2);
	glBindBuffer(GL_ARRAY_BUFFER, normalsId2);
	glBufferData(GL_ARRAY_BUFFER, objects[1].getNVertices() * 3 * sizeof(float), normals2, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	checkError("initBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, 0); //9.
	glBindVertexArray(0); //9.*/
}


GLuint loadTexture(char* textureFile)
{
	GLuint tId;

	FIBITMAP *tf = FreeImage_Load(FIF_DDS, textureFile);
	if (tf) {

		fprintf(stderr, "Texture: %s loaded\n", textureFile);

		glGenTextures(1, &tId);
		glBindTexture(GL_TEXTURE_2D, tId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(tf), FreeImage_GetHeight(tf), 0, GL_BGRA, GL_UNSIGNED_BYTE, FreeImage_GetBits(tf));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		checkError("loadTexture");
		glBindTexture(GL_TEXTURE_2D, 0);

		FreeImage_Unload(tf);

		return tId;
	}
	return 0;
}

/*
Initialization function

All initialization procedures should be performed here.
*/

void init(void) 
{
	

	/******************************************************************
	Delegates which vao/model should be used by each object on-screen
	*******************************************************************/
	GLfloat a=0.0f;
	if (translatex>=2){
		txup=false;}
	if (translatex<=-2){
		txup=true;}
	
	if (txup){
		translatex++;
	}
	
	if (!txup){
		translatex--;
		}
	
	GLfloat b=0.0f;
	for(int i=0;i<size;i++)
		{
			if (i % 2==0){ 
				elements[i]=0;
				translationsM[i] = glm::translate(glm::mat4(1.0), glm::vec3(translatex, b, 0.0f));
							}
			else {
				elements[i]=1;
				translationsM[i] = glm::translate(glm::mat4(1.0), glm::vec3(b, translatex, 1.0f));}
			rotationsY[i] = glm::rotate(glm::mat4(1.0), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		}

	
	
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

	FreeImage_Initialise();

	loadShaderProgram("../shaders/vertex_shader.vs", "../shaders/frag_shader.fs");

	objects[0].init();
	objects[1].init();
	initGeometry();
	//objects[0].print();

	baseTextureId = loadTexture("../models/textures/base.dds");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Defines the clear color, i.e., the color used to wipe the display
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_SAMPLE_SHADING);
	
	checkError("init");
}


/*
[FREEGLUT]

display function;
This function is called to paint the display. This is where the drawing functions should be placed.
Why is the init inside display? Because some drivers require that the display window to be visible to work correctly; therefore, the initialization is done once, but on the first call to 
display, ensuring that the window is visible;

transform the next funtion so argument is index of element (with it can get matrixes and vao)
*/

void display_at(glm::mat4 translateMatrix,GLint v)					//Displays same model at different translations
	
	{
	
	GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baseTextureId);

	GLuint loc = glGetUniformLocation(programId, "tex");
	glUniform1i(loc, 0); 


	loc = glGetUniformLocation(programId, "pMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&perspectiveMatrix[0]);

	loc = glGetUniformLocation(programId, "vMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&cameraMatrix[0]);

	loc = glGetUniformLocation(programId, "mMatrix");

	glBindVertexArray(v);




	
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f)); 

	glm::mat4 model = translateMatrix * rotateY;
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&model[0]);


	glm::mat4 cameraModelMatrix = cameraMatrix * model;

	loc = glGetUniformLocation(programId, "nMatrix");
	glm::mat3 normalMatrix = glm::mat3(cameraMatrix);
	glUniformMatrix3fv(loc, 1, GL_FALSE, (GLfloat *)&normalMatrix[0]); 
	/*
	if using non-uniform scaling:

	glm::mat4 cameraModelMatrix = cameraMatrix * model;
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cameraModelMatrix)));
	*/


	loc = glGetUniformLocation(programId, "lightDir");
	glm::vec4 transformedLightDir = cameraMatrix * glm::vec4(lightDir[0], lightDir[1], lightDir[2], 0.0f);
	glUniform3fv(loc, 1, (GLfloat *)&transformedLightDir[0]);
		

	loc = glGetUniformLocation(programId, "lightIntensity");
	glUniform4fv(loc, 1, lightIntensity);


	loc = glGetUniformLocation(programId, "ambientIntensity");
	glUniform4fv(loc, 1, ambientComponent);


	loc = glGetUniformLocation(programId, "diffuseColor");
	glUniform4fv(loc, 1, diffuseColor);


	/***********************************************************************************
	
	// DRAW ALL!!!!!!!

	**********************************************************************************/


	const unsigned int *indices = objects[0].getIndicesArray();
	glDrawElements(GL_TRIANGLES, objects[0].getNIndices(), GL_UNSIGNED_INT, indices); //type of geometry; number of indices; type of indices array, indices pointer

	//TEST
	const unsigned int *indices2 = objects[1].getIndicesArray();
	glDrawElements(GL_TRIANGLES, objects[1].getNIndices(), GL_UNSIGNED_INT, indices2); //type of geometry; number of indices; type of indices array, indices pointer


}

void display_at2(int indexmatrix,int indexelem)					//Displays same model at different translations
	
	{
	glm::mat4 translateMatrix=translationsM[indexmatrix];
	
	GLint vaot=vao[indexelem];
	
	GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baseTextureId);

	GLuint loc = glGetUniformLocation(programId, "tex");
	glUniform1i(loc, 0); 


	loc = glGetUniformLocation(programId, "pMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&perspectiveMatrix[0]);

	loc = glGetUniformLocation(programId, "vMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&cameraMatrix[0]);

	loc = glGetUniformLocation(programId, "mMatrix");

	glBindVertexArray(vaot);


	
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f)); 

	glm::mat4 model = translateMatrix * rotateY;
	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *)&model[0]);


	glm::mat4 cameraModelMatrix = cameraMatrix * model;

	loc = glGetUniformLocation(programId, "nMatrix");
	glm::mat3 normalMatrix = glm::mat3(cameraMatrix);
	glUniformMatrix3fv(loc, 1, GL_FALSE, (GLfloat *)&normalMatrix[0]); 
	/*
	if using non-uniform scaling:

	glm::mat4 cameraModelMatrix = cameraMatrix * model;
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cameraModelMatrix)));
	*/


	loc = glGetUniformLocation(programId, "lightDir");
	glm::vec4 transformedLightDir = cameraMatrix * glm::vec4(lightDir[0], lightDir[1], lightDir[2], 0.0f);
	glUniform3fv(loc, 1, (GLfloat *)&transformedLightDir[0]);
		

	loc = glGetUniformLocation(programId, "lightIntensity");
	glUniform4fv(loc, 1, lightIntensity);


	loc = glGetUniformLocation(programId, "ambientIntensity");
	glUniform4fv(loc, 1, ambientComponent);


	loc = glGetUniformLocation(programId, "diffuseColor");
	glUniform4fv(loc, 1, diffuseColor);


	/***********************************************************************************
	
	// DRAW ALL!!!!!!!

	**********************************************************************************/


	const unsigned int *indices = objects[indexelem].getIndicesArray();
	glDrawElements(GL_TRIANGLES, objects[indexelem].getNIndices(), GL_UNSIGNED_INT, indices); //type of geometry; number of indices; type of indices array, indices pointer

	//TEST
	

}

void display(void)
{
	if (!inited) {
		init();
		inited = true;
	}
	else{
		// ROTATES ALL FRAMES
		if (translatex>=10.0f){
		txup=false;}
	if (translatex<=-10.0f){
		txup=true;}
	if (txup){
		translatex=translatex+0.01f;
	}
	else{
		translatex=translatex-0.01f;		}
	
	GLfloat b=0.0f;
	for(int i=0;i<size;i++)
		{
			if (i % 2==0){ 
				elements[i]=0;
				translationsM[i] = glm::translate(glm::mat4(1.0), glm::vec3(translatex, b--, 0.0f));
							}
			else {
				elements[i]=1;
				translationsM[i] = glm::translate(glm::mat4(1.0), glm::vec3(b--, translatex, 1.0f));}
			rotationsY[i] = glm::rotate(glm::mat4(1.0), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		} 
	// END
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the display with the defined clear color
	glUseProgram(programId);

	//TEST
	if (increment){
		lightDir[0]=lightDir[0]+0.001f;
		}

	if (lightDir[0]>=1.0)
	{increment=false;}
	
	if (!increment)
	{		
		lightDir[0]=lightDir[0]-0.001f;
		
	}
	if (lightDir[0]<=-1.0)
	{increment=true;}


		if (incrementy){
		lightDir[1]=lightDir[1]+0.001f;
		}

	if (lightDir[1]>=1.0)
	{incrementy=false;}
	
	if (!incrementy)
	{		
		lightDir[1]=lightDir[1]-0.001f;
		
	}
	if (lightDir[1]<=-1.0)
	{incrementy=true;}


	/*****************************************************************************************************************************************
	ATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HERE
	ATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HEREATTENTION HE
	display_at receives a index that indicates the vao index of the object and indexes and prints his object
	need and object or struct with two ids
	1- for know which object and vao
	2- for know

	array of ints - index= id of matrixes of translation, etc - value :id of vao and object
	******************************************************************************************************************************************/

	/*
display_at(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)),vao[0]);
display_at(glm::translate(glm::mat4(10.0), glm::vec3(1.0f, 2.0f, 2.0f)),vao2);
display_at(glm::translate(glm::mat4(10.0), glm::vec3(0.0f, 1.0f, 2.0f)),vao2);
display_at(glm::translate(glm::mat4(10.0), glm::vec3(1.0f, 2.0f, 3.0f)),vao2); */
	for (int i=0;i<size;i++)
	display_at2(i,elements[i]);
//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
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

	perspectiveMatrix = glm::perspective(45.0f, aspect, 0.001f, 1000.0f); //field of view; aspect ratio; z near; z far;

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
	case 'A':
	case 'a':
		cameraPos.x += sin(angle) * velocity;
		cameraPos.z -= cos(angle) * velocity;
		break;
	case 'D':
	case 'd':
		cameraPos.x -= sin(angle) * velocity;
		cameraPos.z += cos(angle) * velocity;
		break;
	case 'W':
	case 'w':
		cameraPos.y += velocity;
		break;
	case 'S':
	case 's':
		cameraPos.y -= velocity;
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
		angle -= velocity;
		break;
	case GLUT_KEY_RIGHT:
		angle += velocity;
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
//MOUSE FUNCTION
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON)
	{
		/*
		if (state == GLUT_DOWN)
			cout << "Right button pressed"
			<< endl;
		else
			cout << "Right button lifted "
			<< "at (" << x << "," << y
			<< ")" << endl;
			
			*/
	}
	else 
		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
				lbuttonDown = true;
			else
				lbuttonDown = false;
		}
}
void motion(int x, int y)
{
	if (lbuttonDown){
/*		cout << "Mouse dragged with left button at "
			<< "(" << x << "," << y << ")" << endl;*/
		if (mousex>x) angle += velocity/100;
			else if (mousex<x) angle -= velocity/100;
		if (mousey>y) cameraPos.y -= velocity/10;
		else if (mousey<y) cameraPos.y += velocity/10;
			
	}
	mousey=y;
	mousex=x;
}
void motionPassive(int x, int y)
{
	/*
	cout << "Mouse moved at "
		<< "(" << x << "," << y << ")" << endl;*/
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

	// MOUSE
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motionPassive);
	//

	glutIdleFunc(mainLoop);

	checkError ("main");
	glutMainLoop(); //starts processing


	return 0;
}

