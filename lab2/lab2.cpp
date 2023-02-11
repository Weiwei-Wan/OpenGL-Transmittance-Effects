// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <map>
#include <vector> // STL dynamic memory.
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

// Project includes
#include "maths_funcs.h"
#define GLT_IMPLEMENTATION
#include "gltext.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_TEAPOT "./models/teapot.dae"
#define MESH_MONKEY "./models/monkeyhead_smooth.dae"
#define MESH_SPHERE "./models/sphere.dae"

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

struct ModelData
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
};

ModelData mesh_teapot;
ModelData mesh_monkey;
ModelData mesh_sphere;

using namespace std;
GLuint SkyBoxID, ReflectionID, RefractionID, FresnelID, DispersionID;

unsigned int mesh_vao = 0;
int width = 1600;
int height = 1200;

glm::mat4 persp_proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
glm::mat4 view;

// Camera pos
GLfloat camera_pos_x = 10.0f;
GLfloat camera_pos_z = 0.0f;
GLfloat camera_pos_y = 0.0f;
GLfloat camera_dir_x = -camera_pos_x;
GLfloat camera_dir_z = -camera_pos_z;
GLfloat camera_dir_y = -camera_pos_y;
GLfloat pitch = 0.0f;
GLfloat roll = 0.0f;
GLfloat yaw = 0.0f;

GLuint loc1, loc2;
GLfloat rotate_x = 0.0f;
GLfloat Delta = 1.0f;
GLuint MODE = 6;
GLfloat Eta = 0.66f;
GLfloat Power = 2.0f;

// ------------ SKYBOX ------------
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
vector<std::string> faces
{
	"./skybox/px2.jpg",
	"./skybox/nx2.jpg",
	"./skybox/py2.jpg",
	"./skybox/ny2.jpg",
	"./skybox/pz2.jpg",
	"./skybox/nz2.jpg"
};

float skyboxVertices[] = {
	-200.0f,  200.0f, -200.0f,
	-200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	-200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f
};
#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vshadername, const char* fshadername)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vshadername, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fshadername, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS


unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int skyboxTextureID;
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return skyboxTextureID;
}

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(GLuint& ID, ModelData mesh_data) {
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(ID, "vertex_position");
	loc2 = glGetAttribLocation(ID, "vertex_normal");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	unsigned int vao = 0;
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void generateSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
#pragma endregion VBO_FUNCTIONS

void drawText(const char* str, GLfloat size, glm::vec3 pos) {
	// Initialize glText
	gltInit();
	// Creating text
	GLTtext* text = gltCreateText();
	gltSetText(text, str);
	// Begin text drawing (this for instance calls glUseProgram)
	gltBeginDraw();
	// Draw any amount of text between begin and end
	gltColor(1.0f, 1.0f, 1.0f, 1.0f);
	gltDrawText2DAligned(text, 70 * (pos.x + 1), 450 - pos.y * 70, size, GLT_CENTER, GLT_CENTER);
	// Finish drawing text
	gltEndDraw();
	// Deleting text
	gltDeleteText(text);
	// Destroy glText
	gltTerminate();
}

void displayNormalObject(GLuint& ID, glm::vec3 pos, ModelData mesh_data, GLuint type, float scale) {
	glUseProgram(ID);
	generateObjectBufferMesh(ID, mesh_data);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	if (type==1) {
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotate_x), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotate_x), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	glUniformMatrix4fv(glGetUniformLocation(ID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform3f(glGetUniformLocation(ID, "cameraPos"), camera_pos_x, camera_pos_y, camera_pos_z);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
}

void displayFersnelObject(GLuint& ID, glm::vec3 pos, ModelData mesh_data, GLuint type, float scale, float eta, float power, boolean has_dispersion, float eta_r, float eta_g, float eta_b) {
	glUseProgram(ID);
	generateObjectBufferMesh(ID, mesh_data);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	if (type == 1) {
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotate_x), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotate_x), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	glUniformMatrix4fv(glGetUniformLocation(ID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ID, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform3f(glGetUniformLocation(ID, "cameraPos"), camera_pos_x, camera_pos_y, camera_pos_z);
	glUniform1f(glGetUniformLocation(ID, "Eta"), eta);
	glUniform1f(glGetUniformLocation(ID, "FresnelPower"), power);
	if (has_dispersion) {
		glUniform1f(glGetUniformLocation(ID, "EtaR"), eta_r);
		glUniform1f(glGetUniformLocation(ID, "EtaG"), eta_g);
		glUniform1f(glGetUniformLocation(ID, "EtaB"), eta_b);
	}
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);
}

void display() {
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	rotate_x += Delta;

	view = glm::lookAt(glm::vec3(camera_pos_x, camera_pos_y, camera_pos_z), // Camera is at (x,y,z), in World Space
		               glm::vec3(camera_pos_x + camera_dir_x, camera_pos_y + camera_dir_y, camera_pos_z + camera_dir_z), // and looks at the origin 
		               glm::vec3(0, 1, 0));  // Head is up (set to 0,-1,0 to look upside-down)

	// skybox
	cubemapTexture = loadCubemap(faces);
	glDepthFunc(GL_LEQUAL);
	glUseProgram(SkyBoxID);
	generateSkybox();
	glUniformMatrix4fv(glGetUniformLocation(SkyBoxID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(SkyBoxID, "proj"), 1, GL_FALSE, &persp_proj[0][0]);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	char array[10];

	if (MODE == 1 || MODE == 6) {
		// teapot reflection
		displayNormalObject(ReflectionID, glm::vec3(0.0f, 0.0f, 5.5f), mesh_teapot, 1, 0.1f);
		// teapot refraction
		displayNormalObject(RefractionID, glm::vec3(0.0f, 0.0f, 2.0f), mesh_teapot, 1, 0.1f);
		// teapot fresnel
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, -2.0f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, false, 0.0f, 0.0f, 0.0f);
		// teapot chromatic dispersion
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, -5.5f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, true, 0.65f, 0.67f, 0.69f);
	}
	else if (MODE == 2) {
		displayNormalObject(ReflectionID, glm::vec3(0.0f, 0.0f, 5.5f), mesh_monkey, 2, 0.8f);
		displayNormalObject(RefractionID, glm::vec3(0.0f, 0.0f, 2.0f), mesh_monkey, 2, 0.8f);
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, -2.0f), mesh_monkey, 2, 0.8f, 0.66f, 2.0f, false, 0.0f, 0.0f, 0.0f);
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, -5.5f), mesh_monkey, 2, 0.8f, 0.66f, 2.0f, true, 0.65f, 0.67f, 0.69f);
	}
	else if (MODE == 3) {
		displayNormalObject(ReflectionID, glm::vec3(0.0f, 0.0f, 5.5f), mesh_sphere, 3, 0.8f);
		displayNormalObject(RefractionID, glm::vec3(0.0f, 0.0f, 2.0f), mesh_sphere, 3, 0.8f);
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, -2.0f), mesh_sphere, 3, 0.8f, 0.66f, 2.0f, false, 0.0f, 0.0f, 0.0f);
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, -5.5f), mesh_sphere, 3, 0.8f, 0.66f, 2.0f, true, 0.65f, 0.67f, 0.69f);
	}
	else if (MODE == 4) {
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, 5.5f), mesh_teapot, 1, 0.1f, Eta, Power, false, 0.0f, 0.0f, 0.0f);
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, 2.0f), mesh_monkey, 2, 0.8f, Eta, Power, false, 0.0f, 0.0f, 0.0f);
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, -2.0f), mesh_sphere, 3, 0.8f, Eta, Power, false, 0.0f, 0.0f, 0.0f);
		displayFersnelObject(FresnelID, glm::vec3(0.0f, 0.0f, -5.5f), mesh_teapot, 1, 0.1f, Eta, Power, false, 0.0f, 0.0f, 0.0f);
		drawText("Fresnel", 4, glm::vec3(10.0f, 4.0f, 0.0f));
		drawText("Eta:", 2, glm::vec3(10.0f, 3.0f, 0.0f));
		snprintf(array, sizeof(array), "%1.1f", Eta);
		drawText(array, 2, glm::vec3(11.5f, 3.0f, 0.0f));
		drawText("Power:", 2, glm::vec3(10.0f, 2.0f, 0.0f));
		snprintf(array, sizeof(array), "%1.1f", Power);
		drawText(array, 2, glm::vec3(11.5f, 2.0f, 0.0f));

	}
	else if (MODE == 5) {
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, 5.5f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, true, 0.15f, 0.17f, 0.19f);
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, 2.0f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, true, 0.35f, 0.37f, 0.39f);
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, -2.0f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, true, 0.65f, 0.67f, 0.69f);
		displayFersnelObject(DispersionID, glm::vec3(0.0f, 0.0f, -5.5f), mesh_teapot, 1, 0.1f, 0.66f, 2.0f, true, 0.95f, 0.97f, 0.99f);
		drawText("Chromatic Dispersion", 4, glm::vec3(10.0f, 5.0f, 0.0f));
		drawText("R:0.15", 2, glm::vec3(2.0f, 3.5f, 0.0f));
		drawText("G:0.17", 2, glm::vec3(2.0f, 3.0f, 0.0f));
		drawText("B:0.19", 2, glm::vec3(2.0f, 2.5f, 0.0f));
		drawText("R:0.35", 2, glm::vec3(7.5f, 3.5f, 0.0f));
		drawText("G:0.37", 2, glm::vec3(7.5f, 3.0f, 0.0f));
		drawText("B:0.39", 2, glm::vec3(7.5f, 2.5f, 0.0f));
		drawText("R:0.55", 2, glm::vec3(13.5f, 3.5f, 0.0f));
		drawText("G:0.57", 2, glm::vec3(13.5f, 3.0f, 0.0f));
		drawText("B:0.59", 2, glm::vec3(13.5f, 2.5f, 0.0f));
		drawText("R:0.95", 2, glm::vec3(19.0f, 3.5f, 0.0f));
		drawText("G:0.97", 2, glm::vec3(19.0f, 3.0f, 0.0f));
		drawText("B:0.99", 2, glm::vec3(19.0f, 2.5f, 0.0f));
	}
	if (MODE < 4) {
		drawText("Reflection", 3, glm::vec3(2.0f, 4.0f, 0.0f));
		drawText("Refraction", 3, glm::vec3(7.5f, 4.0f, 0.0f));
		drawText("Fresnel", 3, glm::vec3(13.5f, 4.0f, 0.0f));
		drawText("Chromatic", 3, glm::vec3(19.0f, 4.0f, 0.0f));
		drawText("Dispersion", 3, glm::vec3(19.0f, 3.0f, 0.0f));
	}

	glutPostRedisplay();
	glutSwapBuffers();
}

void init()
{
	mesh_teapot = load_mesh(MESH_TEAPOT);
	mesh_monkey = load_mesh(MESH_MONKEY);
	mesh_sphere = load_mesh(MESH_SPHERE);
	SkyBoxID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");
	ReflectionID = CompileShaders("./shaders/reflectionVertexShader.txt", "./shaders/reflectionFragmentShader.txt");
	RefractionID = CompileShaders("./shaders/reflectionVertexShader.txt", "./shaders/refractionFragmentShader.txt");
	FresnelID = CompileShaders("./shaders/reflectionVertexShader.txt", "./shaders/fresnelFragmentShader.txt");
	DispersionID = CompileShaders("./shaders/reflectionVertexShader.txt", "./shaders/dispersionFragmentShader.txt");
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == '1') {
		MODE = 1;
	}
	else if (key == '2') {
		MODE = 2;
	}
	else if (key == '3') {
		MODE = 3;
	}
	else if (key == '4') {
		MODE = 4;
	}
	else if (key == '5') {
		MODE = 5;
	}
	else if (key == 'a') {
		Eta += 0.1f;
	}
	else if (key == 'z') {
		Eta -= 0.1f;
	}
	else if (key == 's') {
		Power += 0.5f;
	}
	else if (key == 'x') {
		Power -= 0.5f;
	}
	else if (key == 'q') {
		camera_dir_x = -camera_pos_x;
		camera_dir_z = -camera_pos_z;
		camera_dir_y = -camera_pos_y;
	}
}

void mousePress(int button, int state, int xpos, int ypos) {
	// Wheel reports as button 3(scroll up) and button 4(scroll down)
	if (button == 3) // It's a wheel event
	{
		// Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", xpos, ypos);
		camera_pos_x += 1.0f;
	}
	else if (button == 4) 
	{
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", xpos, ypos);
		camera_pos_x -= 1.0f;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {  // normal button event
		yaw += (xpos - float(width) / 2.0) / width;
		yaw = glm::mod(yaw + 180.0f, 360.0f) - 180.0f;  
		pitch -= (ypos - float(height) / 2.0) / height;
		pitch = glm::clamp(pitch, -89.0f, 89.0f);
		//glutWarpPointer(width / 2.0, height / 2.0);	
		camera_dir_x = cos(pitch) * sin(yaw);
		camera_dir_y = sin(pitch);
		camera_dir_z = -cos(pitch) * cos(yaw); 
	}
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("lab2");

	//texure
	glEnable(GL_DEPTH_TEST);

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutKeyboardFunc(keypress);
	glutMouseFunc(mousePress);
	//glutMotionFunc(mouseMotion);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
