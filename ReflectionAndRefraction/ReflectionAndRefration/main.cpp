/*
Title: Reflection and refraction
File Name: main.cpps
Copyright � 2015
Original authors: Srinivasan T
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This program demonstrates the implementation of reflection and refraction 
in games using skyboxes.
In games, the reflections are rendered using skyboxes. We calculate the 
vector from the camera to the opint (vertex) to be rendered. This vector
is then reflected off the surface with respect to the surface normal at that 
point. Then that vector is used as the texture coordinates to sample the 
cube map of the skybox. Then that color is used to display that pixel. 
This process does not have much effect on the performance as most of the 
work is done in the shader including the reflecting and refracting.

To refract light rays, the same process is applied as for reflection. In this 
program, the background image is the skybox. So we can use the refracted ray 
as a texture coordinate for the cube map of the skybox. But if you wish to do 
this more realistically, you should render the object behind the refractive object
to a texture using a frame buffer. then sample that texture using the refracted
ray as texture coordinates.

Use the mouse to move the sphere around in xy plane.
Comment out the reflective or refractive component in the fragment shader to see the
effects more vividly.

References:
OpenGL 4 shading language cookbook by David Wolff
*/



//In this example, we use two separate programs with different shaders. 
//VertexShaderSkybox and FragmentShaderSkybox.glsl are the shaders used to render skybox
//VertexShader.glsl and FragmentShader.glsl are the shaders used to render the sphere


#include "GLIncludes.h"

// Global data members
#pragma region Base_data
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;
GLuint programSB;

//These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

//Vertex and Fragment shader references to the skyboxshaders. 
GLuint vertex_shaderSB;
GLuint fragment_shaderSB;

GLuint camPosUniform;

//A reference to the texture stored in the GPU
GLuint skybox;
// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniPV;
GLuint uniTranslation;

glm::mat4 PV;

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion Base_data								  


struct stuff_for_drawing{

	GLuint vao;

	//This stores the address the buffer/memory in the GPU. It acts as a handle to access the buffer memory in GPU.
	GLuint vbo;

	//This will be used to tell the GPU, how many vertices will be needed to draw during drawcall.
	int numberOfVertices;

	//This function gets the number of vertices and all the vertex values and stores them in the buffer.
	void initBuffer(int numVertices, VertexFormat* vertices, GLuint programID)
	{
		numberOfVertices = numVertices;

		glGenVertexArrays(1, &vao);
		// This generates buffer object names
		// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
		//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
		//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
		//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
		//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//// Creates and initializes a buffer object's data.
		//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
		//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
		//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
		//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
		//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
		//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);

		//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
		//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
		//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
		glEnableVertexAttribArray(glGetAttribLocation(program, "in_position"));

		//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
		//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
		//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
		//// then the pointer parameter is treated as a byte offset into the buffer object's data.
		glVertexAttribPointer(glGetAttribLocation(program,"in_position"), 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*) (0 * sizeof(float)));
		//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
		//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
		//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
		//// For more info on padding, Google it.

		glEnableVertexAttribArray(glGetAttribLocation(program, "in_normal"));
		glVertexAttribPointer(glGetAttribLocation(program, "in_normal"), 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(3 * sizeof(float)));

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

//Since the skybox does not need any transformations as of now (since the camera is stationary). Only translations which affect the camera need to be applied onto the skybox.
// For more info, refer to the skybox example.
stuff_for_drawing skyBox;

struct sphere
{
	glm::mat4 Translation;
	glm::vec3 origin;
	GLuint lightingtype;			//This will hold the location of the function we want for the type of lighting.
	stuff_for_drawing base;
}sphere1;

void setupSphere()
{
	//Set up sphere 
	std::vector<VertexFormat> vertexSet;
	
	vertexSet.clear();

	float radius = 0.25f;
	float DIVISIONS = 40;

	float pitch, yaw;
	yaw = 0.0f;
	pitch = 0.0f;
	int i, j;
	float pitchDelta = 360 / DIVISIONS;
	float yawDelta = 360 / DIVISIONS;

	VertexFormat p1, p2, p3, p4;

	for (i = 0; i < DIVISIONS; i++)
	{
		for (j = 0; j < DIVISIONS; j++)
		{

			//Since the shape is a sphere, the surface normal would be the vector joining the surface and the center.
			//when the center is at the origin, the normal will be equal to the position vector of the point.
			p1.position.x = radius * sin((pitch)* PI / 180.0) * cos((yaw)* PI / 180.0);
			p1.position.y = radius * sin((pitch)* PI / 180.0) * sin((yaw)* PI / 180.0);;
			p1.position.z = radius * cos((pitch)* PI / 180.0);
			p1.normal = p1.position;
			
			p2.position.x = radius * sin((pitch)* PI / 180.0) * cos((yaw + yawDelta)* PI / 180.0);
			p2.position.y = radius * sin((pitch)* PI / 180.0) * sin((yaw + yawDelta)* PI / 180.0);;
			p2.position.z = radius * cos((pitch)* PI / 180.0);
			p2.normal = p2.position;
			
			p3.position.x = radius * sin((pitch + pitchDelta)* PI / 180.0) * cos((yaw + yawDelta)* PI / 180.0);
			p3.position.y = radius * sin((pitch + pitchDelta)* PI / 180.0) * sin((yaw + yawDelta)* PI / 180.0);;
			p3.position.z = radius * cos((pitch + pitchDelta)* PI / 180.0);
			p3.normal = p3.position;
			
			p4.position.x = radius * sin((pitch + pitchDelta)* PI / 180.0) * cos((yaw)* PI / 180.0);
			p4.position.y = radius * sin((pitch + pitchDelta)* PI / 180.0) * sin((yaw)* PI / 180.0);;
			p4.position.z = radius * cos((pitch + pitchDelta)* PI / 180.0);
			p4.normal = p4.position;
			
			vertexSet.push_back(p1);
			vertexSet.push_back(p2);
			vertexSet.push_back(p3);
			vertexSet.push_back(p1);
			vertexSet.push_back(p3);
			vertexSet.push_back(p4);

			yaw = yaw + yawDelta;
		}
		pitch += pitchDelta;
	}

	sphere1.base.initBuffer(vertexSet.size(), &vertexSet[0],program);
	sphere1.origin = glm::vec3(0.0f, 0.0f, 0.0f);
	sphere1.Translation = glm::translate(glm::mat4(1), sphere1.origin);

}

void setupSkyBox()
{
	std::vector<VertexFormat> vertexSet;

	vertexSet.clear();

	// Setup the skybox
	//xy plane (+z axis)
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f),  glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f),glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0)));

	//xy plane (-z axis)
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));

	//yz plane (+x axis)
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(0)));

	//yz plane (-x axis)
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, 1.0f),  glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0)));

	//xz plane (+y axis)
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, 1.0f, -1.0f),glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, 1.0f, -1.0f),  glm::vec3(0)));

	//xz plane (-y axis)
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, 1.0f),  glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));

	vertexSet.push_back(VertexFormat(glm::vec3(1.0f, -1.0f, 1.0f),  glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0)));
	vertexSet.push_back(VertexFormat(glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(0)));

	skyBox.initBuffer(vertexSet.size(), &vertexSet[0],programSB);
}

void setup()
{
	setupSphere();
	setupSkyBox();

	camPosUniform = glGetUniformLocation(program, "camPos");

	PV = glm::perspective(45.0f, 1.0f, 0.01f, 100.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	//Set up the texture.
	glGenTextures(1, &skybox);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

	GLint w, h;

	unsigned char *data;

	//send the 6 textures for the cube maps
	for (int i = 0; i < 6; i++) {
		std::string texName = (std::string)suffixes[i] +".jpg";
		data = SOIL_load_image(texName.c_str(), &w, &h, 0, SOIL_LOAD_RGBA);
		glTexImage2D(targets[i], 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		SOIL_free_image_data(data);
	}

	// Typical cube map settings
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE);
}

// Functions called only once every time the program is executed.
#pragma region Helper_functions
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	std::string SBvertShader = readShader("VertexShaderSkyBox.glsl");
	std::string SBfragShader = readShader("FragmentShaderSkyBox.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	vertex_shaderSB = createShader(SBvertShader, GL_VERTEX_SHADER);
	fragment_shaderSB = createShader(SBfragShader, GL_FRAGMENT_SHADER);

	programSB = glCreateProgram();
	glAttachShader(programSB, vertex_shaderSB);		// This attaches our vertex shader to our program.
	glAttachShader(programSB, fragment_shaderSB);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(programSB);


	// This gets us a reference to the uniform variable in the vertex shader, which is called "MVP".
	// We're using this variable as a 4x4 transformation matrix
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniPV = glGetUniformLocation(program, "PV");
	uniTranslation = glGetUniformLocation(program, "translation");

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CCW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	glEnable(GL_CULL_FACE);

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	sphere1.origin.x = ((x / 800.0f)*2.0f) - 1.0f;
	sphere1.origin.y = -(((y / 800.0f)*2.0f) - 1.0f);

	sphere1.Translation = glm::translate(glm::mat4(1), sphere1.origin);
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.3, 0.3, 0.3, 1.0);

	//render the skubox
	glUseProgram(programSB);
	//Disable depth buffer
	glDepthMask(GL_FALSE);
	glBindVertexArray(skyBox.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	glDrawArrays(GL_TRIANGLES, 0, skyBox.numberOfVertices);
	//Enable the depth buffer
	glDepthMask(GL_TRUE);
	glBindVertexArray(0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);
	glBindVertexArray(sphere1.base.vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	glUniformMatrix4fv(uniPV, 1, GL_FALSE, glm::value_ptr(PV));						//Set the uniform PV
	glUniformMatrix4fv(uniTranslation, 1, GL_FALSE, glm::value_ptr(sphere1.Translation));
	glUniform3f(camPosUniform, 0.0f, 0.0f, 2.0f);									//Set the uniform cameraPosition
	glDrawArrays(GL_TRIANGLES, 0, sphere1.base.numberOfVertices);					// Draw the sphere
	glBindVertexArray(0);
	//Do the same for the second sphere
}

#pragma endregion Helper_functions


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Reflection and refraction", nullptr, nullptr);

	std::cout << "\n\n\n\n This program demonstrates the implementation of reflection adn refraction using skybox in the shaders.";
	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	setup();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.


	// Frees up GLFW memory
	glfwTerminate();
}