/*
  CSCI 420 Computer Graphics, USC
  Assignment 2: Roller Coaster
  C++ starter code

  Student username: joeyuhoc
*/

// Basic Includes
#include <vector>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// Include to Enhance Code
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// Header Files
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x, y Coordinate of the Mouse Position

int leftMouseButton = 0; // 1 if Pressed, 0 if Not 
int middleMouseButton = 0; // 1 if Pressed, 0 if Not
int rightMouseButton = 0; // 1 if Pressed, 0 if Not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

typedef enum { COLOR_RAIL, PHONG_RAIL} DISPLAY_TYPE;
DISPLAY_TYPE displayType = PHONG_RAIL;

// State of the World
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II Joe 8800444224";

OpenGLMatrix matrix;
BasicPipelineProgram pipelineProgram, pipelineProgramPhong, pipelineTextureProgram;
GLuint program, phongProgram, textureProgram;
GLint h_modelViewMatrix, h_projectionMatrix;
GLint h_modelViewMatrix_phong, h_projectionMatrix_phong, h_normalMatrix_phong;
GLint modelViewMatrix, projectionMatrix;

int fovy = 45;  // Field of View y
float aspect = 1280.0f / 720.0f; // Aspect Ratio
int screenshotCounter = 0; // Counter for Screenshot

GLuint loc; // Used to Obtain Location in Shader

// For Caculaing Spline Points
glm::mat4 BASIS_MATRIX;
glm::mat4x3 CONTROL_MATRIX;
vector<vector<glm::vec3>> SPLINE_POINTS;
vector<vector<glm::vec3>> SPLINE_TANGENTS;
vector<vector<glm::vec3>> SPLINE_NORMALS;
vector<vector<glm::vec3>> SPLINE_BINORMALS;

// For Color Shading Mode and For Phong Shading Mode
GLuint* VAO;
GLuint* VBO_POSITION;
GLuint* VBO_NORMALS;
vector<vector<glm::vec3>> LEFT_RAIL_VERTICES;
vector<vector<glm::vec3>> RIGHT_RAIL_VERTICES;
vector<vector<glm::vec4>> NORMALS;

GLuint* VAO_PHONG;
GLuint* VBO_PHONG_POSITION;
GLuint* VBO_PHONG_NORMALS;
vector<vector<glm::vec3>> PHONG_NORMALS;

// For Bottom Sky Box
GLuint TEXTURE_HANDLE_GROUND;
GLuint VAO_GROUND;
GLuint VBO_GROUND_POSITION;
GLuint VBO_GROUND_UV;
vector<glm::vec3> GROUND_VERTICES;
vector<glm::vec2> GROUND_UVS;

// For Top Sky Box
GLuint TEXTURE_HANDLE_SKY;
GLuint VAO_SKY;
GLuint VBO_SKY_POSITION;
GLuint VBO_SKY_UV;
vector<glm::vec3> SKY_VERTICES;
vector<glm::vec2> SKY_UVS;

// For Front Sky Box
GLuint TEXTURE_HANDLE_FRONT;
GLuint VAO_FRONT;
GLuint VBO_FRONT_POSITION;
GLuint VBO_FRONT_UV;
vector<glm::vec3> FRONT_VERTICES;
vector<glm::vec2> FRONT_UVS;

// For Right Sky Box
GLuint TEXTURE_HANDLE_RIGHT;
GLuint VAO_RIGHT;
GLuint VBO_RIGHT_POSITION;
GLuint VBO_RIGHT_UV;
vector<glm::vec3> RIGHT_VERTICES;
vector<glm::vec2> RIGHT_UVS;

// For Left Sky Box
GLuint TEXTURE_HANDLE_LEFT;
GLuint VAO_LEFT;
GLuint VBO_LEFT_POSITION;
GLuint VBO_LEFT_UV;
vector<glm::vec3> LEFT_VERTICES;
vector<glm::vec2> LEFT_UVS;

// For Back Sky Box
GLuint TEXTURE_HANDLE_BACK;
GLuint VAO_BACK;
GLuint VBO_BACK_POSITION;
GLuint VBO_BACK_UV;
vector<glm::vec3> BACK_VERTICES;
vector<glm::vec2> BACK_UVS;

// For Wooden Cross Bar
GLuint TEXTURE_HANDLE_CROSS;
GLuint* VAO_CROSS;
GLuint* VBO_CROSS_POSITION;
GLuint* VBO_CROSS_UV;
vector<vector<glm::vec3>> CROSS_VERTICES;
vector<vector<glm::vec2>> CROSS_UVS;

float degree = 1.0f; // Degree for Tangent
float degree2 = 0.0f; // Degree for Binormal
int step = 0; // Step for Camera
bool play = false; // Control for Animate
int ID = 0; // ID of Loaded Spline(s)

float add = 0; // Increased Distance
vector<int> pre; // Previous Step
vector<float> pre_add; // Previous Distance
float MAX_HEIGHT = 6.2f; // Max Height
float MAX_LENGTH = 0.01f; // Max Length
float dt = 1.0f / 400.0f; // Time Step
float g = 9.8f; // Gravity Constanct

// Represents One Control Point along the Spline 
struct Point {
	double x;
	double y;
	double z;
};

// Spline Struct, Contains How Many Control Points the Spline Has, and an Array of Control Points 
struct Spline {
	int numControlPoints;
	Point * points;
};

// The Spline Array 
Spline * splines;

// Total Number of Splines 
int numSplines;

// Load Splines from File(s)
int loadSplines(char * argv) {
	char * cName = (char *)malloc(128 * sizeof(char));
	FILE * fileList;
	FILE * fileSpline;
	int iType, i = 0, j, iLength;

	// Load the Track File 
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf("can't open file\n");
		exit(1);
	}

	// Stores the Number of Splines in a Global Variable 
	fscanf(fileList, "%d", &numSplines);

	splines = (Spline*)malloc(numSplines * sizeof(Spline));

	// Reads Through the Spline Files 
	for (j = 0; j < numSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf("can't open file\n");
			exit(1);
		}

		// Gets Length for Spline File
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		// Allocate Memory for All the Points
		splines[j].points = (Point *)malloc(iLength * sizeof(Point));
		splines[j].numControlPoints = iLength;

		// Saves the Data to the Struct
		while (fscanf(fileSpline, "%lf %lf %lf",
			&splines[j].points[i].x,
			&splines[j].points[i].y,
			&splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

// Write a Screenshot to the Specified Filename
void saveScreenshot(const char * filename) {
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

// Initialize Texture Settings
int initTexture(const char * imageFilename, GLuint textureHandle)
{
	// Read the Texture Image
	ImageIO img;
	ImageIO::fileFormatType imgFormat;
	ImageIO::errorType err = img.load(imageFilename, &imgFormat);

	if (err != ImageIO::OK)
	{
		printf("Loading texture from %s failed.\n", imageFilename);
		return -1;
	}

	// Check that the Number Of Bytes is a Multiple of 4
	if (img.getWidth() * img.getBytesPerPixel() % 4)
	{
		printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
		return -1;
	}

	// Allocate Space for an Array of Pixels
	int width = img.getWidth();
	int height = img.getHeight();
	unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // We Will Use 4 Bytes Per Pixel, i.e., RGBA

	// Fill the pixelsRGBA Array with the Image Pixels
	memset(pixelsRGBA, 0, 4 * width * height); // Set All Bytes To 0
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
		{
			// Assign Some Default Byte Values (For the Case Where img.getBytesPerPixel() < 4)
			pixelsRGBA[4 * (h * width + w) + 0] = 0; // Red
			pixelsRGBA[4 * (h * width + w) + 1] = 0; // Green
			pixelsRGBA[4 * (h * width + w) + 2] = 0; // Blue
			pixelsRGBA[4 * (h * width + w) + 3] = 255; // Alpha Channel; Fully Opaque

			// Set the RGBA Channels, Based on the Loaded Image
			int numChannels = img.getBytesPerPixel();
			for (int c = 0; c < numChannels; c++) // Only Set As Many Channels as are Available in the Loaded Image; The Rest Get the Default Value
				pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
		}

	// Bind the Texture
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	// Initialize the Texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

	// Generate The Mipmaps for this Texture
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set the Texture Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Query Support for Anisotropic Texture Filtering
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	printf("Max available anisotropic samples for %s:\t%f\n", imageFilename, fLargest);
	// Set Anisotropic Texture Filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

	// Query for Any Errors
	GLenum errCode = glGetError();
	if (errCode != 0) {
		printf("Texture initialization error. Error code: %d.\n", errCode);
		return -1;
	}

	// De-allocate the Pixel Array -- it is no longer needed
	delete[] pixelsRGBA;

	return 0;
}

// Initalizing Modelview Matrix and Projection Matrix
void initializeMatrix(int mode) {

	matrix.SetMatrixMode(OpenGLMatrix::ModelView); // Set ModelView Matrix
	matrix.LoadIdentity(); // Load Identity Matrix

	// Set Camera Pointing Direction and Position
	glm::vec3 position = SPLINE_POINTS[ID][step] + SPLINE_NORMALS[ID][step] * 0.1f;
	glm::vec3 direction = SPLINE_TANGENTS[ID][step] * degree + SPLINE_POINTS[ID][step] +  SPLINE_BINORMALS[ID][step] * degree2;
	glm::vec3 normal = SPLINE_NORMALS[ID][step];
	matrix.LookAt(position.x, position.y, position.z, direction.x, direction.y, direction.z, normal.x, normal.y, normal.z);
	// cout << normal.x << " " << normal.y << " " << normal.z << endl;
	// matrix.LookAt(0, 0, 10, 0, 0, -1, 0, 1, 0);
	
	// Set Up Light Direction and Other Uniforms for Phong Shading
	if (mode == 0) {
		float view[16];
		matrix.GetMatrix(view); 
		GLint h_viewLightDirection = glGetUniformLocation(phongProgram, "viewLightDirection");
		glm::vec3 lightDirection = glm::vec3(0, 1, 1); // the "Sun" 
		glm::mat4 VIEW_MATRIX = glm::make_mat4(view);
		glm::vec4 result = VIEW_MATRIX * glm::vec4(lightDirection, 0.0);
        float viewLightDirection[3] = { result.x , result.y , result.z }; // Light Direction in the View Space
		glUniform3fv(h_viewLightDirection, 1, viewLightDirection); // Upload Light Direction


		// Set and Upload Uniforms for Phong Shading
		GLint h_La = glGetUniformLocation(phongProgram, "La");
		float La[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		glUniform4fv(h_La, 1, La);
		GLint h_Ld = glGetUniformLocation(phongProgram, "Ld");
		float Ld[4] = { 0.76f, 0.76f, 0.76f, 1.0f };
		glUniform4fv(h_Ld, 1, Ld);
		GLint h_Ls = glGetUniformLocation(phongProgram, "Ls");
		float Ls[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glUniform4fv(h_Ls, 1, Ls);

		GLint h_Ka = glGetUniformLocation(phongProgram, "ka");
		float Ka[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glUniform4fv(h_Ka, 1, Ka);
		GLint h_Kd = glGetUniformLocation(phongProgram, "kd");
		float Kd[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glUniform4fv(h_Kd, 1, Kd);
		GLint h_Ks = glGetUniformLocation(phongProgram, "ks");
		float Ks[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glUniform4fv(h_Ks, 1, Ks);
		GLint h_alpha = glGetUniformLocation(phongProgram, "alpha");
		float alpha = 2000.0;
		glUniform1f(h_alpha, alpha);
	}

	// Translate, Rotate, and Scale the Matrix
	matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	matrix.Scale(landScale[0], landScale[1], landScale[2]);

	float m[16];
	matrix.GetMatrix(m); // Obtatin Current Matrix
	// Upload Modelveiw Matrix to GPU
	if (mode == 0) { 
		glUniformMatrix4fv(h_modelViewMatrix_phong, 1, GL_FALSE, m); 

		// Upload Normal Matrix to GPU
		GLint h_normalMatrix = glGetUniformLocation(phongProgram, "normalMatrix"); 
		matrix.GetNormalMatrix(m); 
		glUniformMatrix4fv(h_normalMatrix, 1, GL_FALSE, m);
	}
	else if (mode == 1) {
		glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
	}
	else if (mode == 2) {
		glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, m);
	}

	matrix.SetMatrixMode(OpenGLMatrix::Projection); // Set Projection Matrix
	matrix.LoadIdentity(); // Load Identity Matrix
	matrix.Perspective(fovy, aspect, 0.01, 1000.0); // Set Perspective

	matrix.GetMatrix(m); // Obtatin Current Matrix
	// Upload Projection Matrix to GPU
	if (mode == 0) {
		glUniformMatrix4fv(h_projectionMatrix_phong, 1, GL_FALSE, m);
	}
	else if (mode == 1) {
		glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, m);
	}
	else if (mode == 2) {
		glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, m);
	}
}

// Initialize Texture Vertices and UVs
void initializeTexureVertices(glm::vec3 point1, glm::vec3 point2, vector<glm::vec3> &vertices, vector<glm::vec2> &uvs) {
    
    bool diffX = (point1.x == point2.x);
    bool diffY = (point1.y == point2.y);
    bool diffZ = (point1.z == point2.z);

    // Set Points for Texture
    glm::vec3 P0 = glm::vec3(point1.x, point1.y, point1.z);
    glm::vec3 P1 = glm::vec3(point2.x, point1.y, point1.z);
    glm::vec3 P2 = glm::vec3(point2.x, point2.y, point2.z);
    glm::vec3 P3 = glm::vec3(point1.x, point2.y, point2.z);
    
	// If Front and Back Image
    if(diffX){
        P0 = glm::vec3(point1.x, point1.y, point1.z);
        P1 = glm::vec3(point1.x, point2.y, point1.z);
        P2 = glm::vec3(point2.x, point2.y, point2.z);
        P3 = glm::vec3(point2.x, point1.y, point2.z);
    }
    
    vertices.push_back(P0);
    vertices.push_back(P1);
    vertices.push_back(P2);
    vertices.push_back(P2);
    vertices.push_back(P3);
    vertices.push_back(P0);
    
	// Set UVs for Texture
    glm::vec2 P0_UV = glm::vec2(1.0f, 0.0f);
    glm::vec2 P1_UV = glm::vec2(0.0f, 0.0f);
    glm::vec2 P2_UV = glm::vec2(0.0f, 1.0f);
    glm::vec2 P3_UV = glm::vec2(1.0f, 1.0f);
    
    uvs.push_back(P0_UV);
    uvs.push_back(P1_UV);
    uvs.push_back(P2_UV);
    uvs.push_back(P2_UV);
    uvs.push_back(P3_UV);
    uvs.push_back(P0_UV);
}

// Push Normals for Phong Mode
void pushPhong(int i, glm::vec3 normal) {
	for (int k = 0; k < 12; k++) {
		PHONG_NORMALS[i].push_back(normal);
	}
}

// Push Normals for Color Mode
void pushColor(int i, glm::vec3 normal) {
	for (int k = 0; k < 12; k++) {
		NORMALS[i].push_back(glm::vec4(normal, 1.0));
	}
}

// Push Left or Right Rail Vertices
void pushRail(int mode, int i, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3) {
	if (mode == 0) {
		LEFT_RAIL_VERTICES[i].push_back(P0);
		LEFT_RAIL_VERTICES[i].push_back(P1);
		LEFT_RAIL_VERTICES[i].push_back(P2);
		LEFT_RAIL_VERTICES[i].push_back(P2);
		LEFT_RAIL_VERTICES[i].push_back(P3);
		LEFT_RAIL_VERTICES[i].push_back(P0);
	}
	else if (mode == 1) {
		RIGHT_RAIL_VERTICES[i].push_back(P0);
		RIGHT_RAIL_VERTICES[i].push_back(P1);
		RIGHT_RAIL_VERTICES[i].push_back(P2);
		RIGHT_RAIL_VERTICES[i].push_back(P2);
		RIGHT_RAIL_VERTICES[i].push_back(P3);
		RIGHT_RAIL_VERTICES[i].push_back(P0);
	}
}

// Push Cross Bar Positions and UVs
void pushCross(int i, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3) {

	CROSS_VERTICES[i].push_back(P0);
	CROSS_VERTICES[i].push_back(P1);
	CROSS_VERTICES[i].push_back(P2);
	CROSS_VERTICES[i].push_back(P2);
	CROSS_VERTICES[i].push_back(P3);
	CROSS_VERTICES[i].push_back(P0);

	glm::vec2 BOTTOM_LEFT_UV = glm::vec2(1.0f, 1.0f);
	glm::vec2 TOP_LEFT_UV = glm::vec2(0.0f, 0.0f);
	glm::vec2 BOTTOM_RIGHT_UV = glm::vec2(0.0f, 1.0f);
	glm::vec2 TOP_RIGHT_UV = glm::vec2(1.0f, 0.0f);

	CROSS_UVS[i].push_back(BOTTOM_LEFT_UV);
	CROSS_UVS[i].push_back(BOTTOM_RIGHT_UV);
	CROSS_UVS[i].push_back(TOP_RIGHT_UV);
	CROSS_UVS[i].push_back(TOP_RIGHT_UV);
	CROSS_UVS[i].push_back(TOP_LEFT_UV);
	CROSS_UVS[i].push_back(BOTTOM_LEFT_UV);
}

// Subdivision for Calculating Spline Points
void subdivide(float u0, float u1, float maxLength, int i) {
	if (u0 != u1) {

		//u0 = roundf(u0 * 100.0f) / 100.0f;
		//u1 = roundf(u1 * 100.0f) / 100.0f;

		float mid = (u0 + u1) / 2.0f; // Middle Value

		// First Point
		glm::vec4 PARAMETER_VECTOR_1 = glm::vec4(pow(u0, 3), pow(u0, 2), u0, 1.0f);
		glm::vec3 SPLINE_POINT_1 = CONTROL_MATRIX * BASIS_MATRIX * PARAMETER_VECTOR_1;

		// Second Point
		glm::vec4 PARAMETER_VECTOR_2 = glm::vec4(pow(u1, 3), pow(u1, 2), u1, 1.0f);
		glm::vec3 SPLINE_POINT_2 = CONTROL_MATRIX * BASIS_MATRIX * PARAMETER_VECTOR_2;

		// Vector of the Points
		glm::vec3 POINT_VECTOR = SPLINE_POINT_1 - SPLINE_POINT_2;

		if (glm::length(POINT_VECTOR) > maxLength) {
			// If Greater than maxLength, Continue to Subdivede
			subdivide(u0, mid, maxLength, i);
			subdivide(mid, u1, maxLength, i);
		}
		else {
			// Else Store First Point and Calculate and Store Tangent of that Point
			SPLINE_POINTS[i].push_back(SPLINE_POINT_1);
			glm::vec4 PARAMETER_VECTOR = glm::vec4(3 * pow(u0, 2), 2 * u0, 1, 0.0f);
			glm::vec3 SPLINE_TANGENT = CONTROL_MATRIX * BASIS_MATRIX * PARAMETER_VECTOR;
			glm::vec3 UNIT_TANGENT = glm::normalize(SPLINE_TANGENT);
			SPLINE_TANGENTS[i].push_back(UNIT_TANGENT);
		}
	}
}

// Initialize Vertices
void initializeVertices() {

	// For Each Spline File
	for (int i = 0; i < numSplines; i++) {

		SPLINE_POINTS.push_back(vector<glm::vec3>());
		SPLINE_TANGENTS.push_back(vector<glm::vec3>());
		SPLINE_NORMALS.push_back(vector<glm::vec3>());
		SPLINE_BINORMALS.push_back(vector<glm::vec3>());
		LEFT_RAIL_VERTICES.push_back(vector<glm::vec3>());
		RIGHT_RAIL_VERTICES.push_back(vector<glm::vec3>());
		NORMALS.push_back(vector<glm::vec4>());
		PHONG_NORMALS.push_back(vector<glm::vec3>());
		CROSS_VERTICES.push_back(vector<glm::vec3>());
		CROSS_UVS.push_back(vector<glm::vec2>());

		// Calculate Spline Points
		for (int j = 0; j < splines[i].numControlPoints - 3; j++) {
			Point CONTROL_POINT_1 = splines[i].points[j];
			Point CONTROL_POINT_2 = splines[i].points[j + 1];
			Point CONTROL_POINT_3 = splines[i].points[j + 2];
			Point CONTROL_POINT_4 = splines[i].points[j + 3];

			float s = 0.5;
			BASIS_MATRIX[0] = glm::vec4(-1.0f * s, 2.0f - s, s - 2.0f, s);
			BASIS_MATRIX[1] = glm::vec4(2.0f * s, s - 3.0f, 3.0f - 2.0f * s, -s);
			BASIS_MATRIX[2] = glm::vec4(-s, 0.0f, s, 0.0f);
			BASIS_MATRIX[3] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

			CONTROL_MATRIX[0] = glm::vec3(CONTROL_POINT_1.x, CONTROL_POINT_1.y, CONTROL_POINT_1.z);
			CONTROL_MATRIX[1] = glm::vec3(CONTROL_POINT_2.x, CONTROL_POINT_2.y, CONTROL_POINT_2.z);
			CONTROL_MATRIX[2] = glm::vec3(CONTROL_POINT_3.x, CONTROL_POINT_3.y, CONTROL_POINT_3.z);
			CONTROL_MATRIX[3] = glm::vec3(CONTROL_POINT_4.x, CONTROL_POINT_4.y, CONTROL_POINT_4.z);
			
			// Brute Force Method
			/*for (float u = 0.0f; u < 1.0f; u += 0.01f) {
				glm::vec4 PARAMETER_VECTOR = glm::vec4(pow(u, 3), pow(u, 2), u, 1.0f);
				// column-major matrix
				glm::vec3 SPLINE_POINT = CONTROL_MATRIX * BASIS_MATRIX * PARAMETER_VECTOR;
				SPLINE_POINTS.push_back(SPLINE_POINT);

				PARAMETER_VECTOR = glm::vec4(3 * pow(u, 2), 2 * u, 1, 0.0f);
				glm::vec3 SPLINE_TANGENT = CONTROL_MATRIX * BASIS_MATRIX * PARAMETER_VECTOR;
				glm::vec3 UNIT_TANGENT = glm::normalize(SPLINE_TANGENT);
				SPLINE_TANGENTS.push_back(UNIT_TANGENT);
			}*/

			// Subdivision Recursive Method
			float u0 = 0.0f;
			float u1 = 1.0f;
			subdivide(u0, u1, MAX_LENGTH, i);
		}

		// Milestone
		/*for (int k = 0; k < SPLINE_POINTS.size() - 1; k++) {
			GLfloat POINT_1[3] = { SPLINE_POINTS[k].x,  SPLINE_POINTS[k].y ,  SPLINE_POINTS[k].z };
			GLfloat POINT_2[3] = { SPLINE_POINTS[k + 1].x,  SPLINE_POINTS[k + 1].y ,  SPLINE_POINTS[k + 1].z };

			VERTICES.insert(VERTICES.end(), POINT_1, POINT_1 + 3);
			VERTICES.insert(VERTICES.end(), POINT_2, POINT_2 + 3);

			GLfloat singleColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			NORMALS.insert(NORMALS.end(), singleColor, singleColor + 4);
			NORMALS.insert(NORMALS.end(), singleColor, singleColor + 4);

		}*/

		// Calculate and Store Normals and Binormals using Sloan's Method
		for (int k = 0; k < SPLINE_TANGENTS[i].size(); k++) {
			//Sloan's Method
			glm::vec3 SPLINE_NORMAL;
			glm::vec3 SPLINE_BINORMAL;
			if (k == 0) {
				glm::vec3 VECTOR_V = glm::vec3(0.0f, 1.0f, 0.0f); // Y Vector
				SPLINE_NORMAL = glm::normalize(glm::cross(SPLINE_TANGENTS[i][k], VECTOR_V));
				SPLINE_BINORMAL = glm::normalize(glm::cross(SPLINE_TANGENTS[i][k], SPLINE_NORMAL));
			}
			else {
				SPLINE_NORMAL = glm::normalize(glm::cross(SPLINE_BINORMALS[i][k - 1], SPLINE_TANGENTS[i][k]));
				SPLINE_BINORMAL = glm::normalize(glm::cross(SPLINE_TANGENTS[i][k], SPLINE_NORMAL));
			}
			SPLINE_NORMALS[i].push_back(SPLINE_NORMAL);
			SPLINE_BINORMALS[i].push_back(SPLINE_BINORMAL);
		}

		// Caculate Tube Positions (T Shape)
		for (int k = 0 ; k < SPLINE_POINTS[i].size() ; k++) {

			GLfloat alpha = 0.01f; // For Bottom Track
			GLfloat beta = 0.002f; // For Top Track (Small)
			glm::vec3 OFFSET = SPLINE_BINORMALS[i][k] * 0.05f; // Offset for Moving Rails
			glm::vec3 BOTTOM_LEFT = SPLINE_POINTS[i][k] + alpha * (-SPLINE_NORMALS[i][k] - SPLINE_BINORMALS[i][k]);
			glm::vec3 BOTTOM_RIGHT = SPLINE_POINTS[i][k] + alpha * (-SPLINE_NORMALS[i][k] + SPLINE_BINORMALS[i][k]);
			glm::vec3 TOP_LEFT = SPLINE_POINTS[i][k] + alpha * (-SPLINE_BINORMALS[i][k]);
			glm::vec3 TOP_RIGHT = SPLINE_POINTS[i][k] + alpha * (SPLINE_BINORMALS[i][k]);
			glm::vec3 BOTTOM_LEFT_SMALL = SPLINE_POINTS[i][k] + beta * (-SPLINE_BINORMALS[i][k]);
			glm::vec3 BOTTOM_RIGHT_SMALL = SPLINE_POINTS[i][k] + beta * (SPLINE_BINORMALS[i][k]);
			glm::vec3 TOP_LEFT_SMALL = SPLINE_POINTS[i][k] + alpha * (SPLINE_NORMALS[i][k]) + beta * (-SPLINE_BINORMALS[i][k]);
			glm::vec3 TOP_RIGHT_SMALL = SPLINE_POINTS[i][k] + alpha * (SPLINE_NORMALS[i][k]) + beta * (SPLINE_BINORMALS[i][k]);

			// Front Face
			pushRail(0, i, BOTTOM_LEFT - OFFSET, BOTTOM_RIGHT - OFFSET, TOP_RIGHT - OFFSET, TOP_LEFT - OFFSET);
			pushRail(0, i, BOTTOM_LEFT_SMALL - OFFSET, BOTTOM_RIGHT_SMALL - OFFSET, TOP_RIGHT_SMALL - OFFSET, TOP_LEFT_SMALL - OFFSET);
			pushRail(1, i, BOTTOM_LEFT + OFFSET, BOTTOM_RIGHT + OFFSET, TOP_RIGHT + OFFSET, TOP_LEFT + OFFSET);
			pushRail(1, i, BOTTOM_LEFT_SMALL + OFFSET, BOTTOM_RIGHT_SMALL + OFFSET, TOP_RIGHT_SMALL + OFFSET, TOP_LEFT_SMALL + OFFSET);
			glm::vec3 normal = glm::normalize(glm::cross((BOTTOM_LEFT - BOTTOM_RIGHT), (TOP_RIGHT - BOTTOM_RIGHT)));
			pushColor(i, normal);
			pushPhong(i, normal);

			if ((k + 1) != SPLINE_TANGENTS[i].size()) {
				glm::vec3 NEXT_OFFSET = SPLINE_BINORMALS[i][k + 1] * 0.05f; // Offset for Moving Rails
				glm::vec3 NEXT_BOTTOM_LEFT = SPLINE_POINTS[i][k + 1] + alpha * (-SPLINE_NORMALS[i][k + 1] - SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_BOTTOM_RIGHT = SPLINE_POINTS[i][k + 1] + alpha * (-SPLINE_NORMALS[i][k + 1] + SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_TOP_LEFT = SPLINE_POINTS[i][k + 1] + alpha * (-SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_TOP_RIGHT = SPLINE_POINTS[i][k + 1] + alpha * (SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_BOTTOM_LEFT_SMALL = SPLINE_POINTS[i][k + 1] + beta * (-SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_BOTTOM_RIGHT_SMALL = SPLINE_POINTS[i][k + 1] + beta * (SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_TOP_LEFT_SMALL = SPLINE_POINTS[i][k + 1] + alpha * (SPLINE_NORMALS[i][k + 1]) + beta * (-SPLINE_BINORMALS[i][k + 1]);
				glm::vec3 NEXT_TOP_RIGHT_SMALL = SPLINE_POINTS[i][k + 1] + alpha * (SPLINE_NORMALS[i][k + 1]) + beta * (SPLINE_BINORMALS[i][k + 1]);

				//Left Face
				pushRail(0, i, TOP_LEFT - OFFSET, BOTTOM_LEFT - OFFSET, NEXT_BOTTOM_LEFT - NEXT_OFFSET, NEXT_TOP_LEFT - NEXT_OFFSET);
				pushRail(0, i, TOP_LEFT_SMALL - OFFSET, BOTTOM_LEFT_SMALL - OFFSET, NEXT_BOTTOM_LEFT_SMALL - NEXT_OFFSET, NEXT_TOP_LEFT_SMALL - NEXT_OFFSET);
				pushRail(1, i, TOP_LEFT + OFFSET, BOTTOM_LEFT + OFFSET, NEXT_BOTTOM_LEFT + NEXT_OFFSET, NEXT_TOP_LEFT + NEXT_OFFSET);
				pushRail(1, i, TOP_LEFT_SMALL + OFFSET, BOTTOM_LEFT_SMALL + OFFSET, NEXT_BOTTOM_LEFT_SMALL + NEXT_OFFSET, NEXT_TOP_LEFT_SMALL + NEXT_OFFSET);
				normal = glm::normalize(glm::cross((NEXT_BOTTOM_LEFT - BOTTOM_LEFT), (TOP_LEFT - BOTTOM_LEFT)));
				pushColor(i, normal);
				pushPhong(i, normal);

				//Right Face
				pushRail(0, i, TOP_RIGHT - OFFSET, BOTTOM_RIGHT - OFFSET, NEXT_BOTTOM_RIGHT - NEXT_OFFSET, NEXT_TOP_RIGHT - NEXT_OFFSET);
				pushRail(0, i, TOP_RIGHT_SMALL - OFFSET, BOTTOM_RIGHT_SMALL - OFFSET, NEXT_BOTTOM_RIGHT_SMALL - NEXT_OFFSET, NEXT_TOP_RIGHT_SMALL - NEXT_OFFSET);
				pushRail(1, i, TOP_RIGHT + OFFSET, BOTTOM_RIGHT + OFFSET, NEXT_BOTTOM_RIGHT + NEXT_OFFSET, NEXT_TOP_RIGHT + NEXT_OFFSET);
				pushRail(1, i, TOP_RIGHT_SMALL + OFFSET, BOTTOM_RIGHT_SMALL + OFFSET, NEXT_BOTTOM_RIGHT_SMALL + NEXT_OFFSET, NEXT_TOP_RIGHT_SMALL + NEXT_OFFSET);
				normal = glm::normalize(glm::cross((NEXT_BOTTOM_RIGHT - BOTTOM_RIGHT), (TOP_RIGHT - BOTTOM_RIGHT)));
				pushColor(i, normal);
				pushPhong(i, normal);

				//Top Face
				pushRail(0, i, TOP_LEFT - OFFSET, TOP_RIGHT - OFFSET, NEXT_TOP_RIGHT - NEXT_OFFSET, NEXT_TOP_LEFT - NEXT_OFFSET);
				pushRail(0, i, TOP_LEFT_SMALL - OFFSET, TOP_RIGHT_SMALL - OFFSET, NEXT_TOP_RIGHT_SMALL - NEXT_OFFSET, NEXT_TOP_LEFT_SMALL - NEXT_OFFSET);
				pushRail(1, i, TOP_LEFT + OFFSET, TOP_RIGHT + OFFSET, NEXT_TOP_RIGHT + NEXT_OFFSET, NEXT_TOP_LEFT + NEXT_OFFSET);
				pushRail(1, i, TOP_LEFT_SMALL + OFFSET, TOP_RIGHT_SMALL + OFFSET, NEXT_TOP_RIGHT_SMALL + NEXT_OFFSET, NEXT_TOP_LEFT_SMALL + NEXT_OFFSET);
				normal = glm::normalize(glm::cross((NEXT_TOP_RIGHT - TOP_RIGHT), (TOP_LEFT - TOP_RIGHT)));
				pushColor(i, normal);
				pushPhong(i, normal);

				//Bottom Face
				pushRail(0, i, BOTTOM_LEFT - OFFSET, BOTTOM_RIGHT - OFFSET, NEXT_BOTTOM_RIGHT - NEXT_OFFSET, NEXT_BOTTOM_LEFT - NEXT_OFFSET);
				pushRail(0, i, BOTTOM_LEFT_SMALL - OFFSET, BOTTOM_RIGHT_SMALL - OFFSET, NEXT_BOTTOM_RIGHT_SMALL - NEXT_OFFSET, NEXT_BOTTOM_LEFT_SMALL - NEXT_OFFSET);
				pushRail(1, i, BOTTOM_LEFT + OFFSET, BOTTOM_RIGHT + OFFSET, NEXT_BOTTOM_RIGHT + NEXT_OFFSET, NEXT_BOTTOM_LEFT + NEXT_OFFSET);
				pushRail(1, i, BOTTOM_LEFT_SMALL + OFFSET, BOTTOM_RIGHT_SMALL + OFFSET, NEXT_BOTTOM_RIGHT_SMALL + NEXT_OFFSET, NEXT_BOTTOM_LEFT_SMALL + NEXT_OFFSET);
				normal = glm::normalize(glm::cross((NEXT_BOTTOM_RIGHT - BOTTOM_RIGHT), (BOTTOM_LEFT - BOTTOM_RIGHT)));
				pushColor(i, normal);
				pushPhong(i, normal);
			}
		}

		// Calculate Cross Section
		for (int k = 0; k < SPLINE_POINTS[i].size() - 5; k++) {
			if (k % 10 == 0) {

				// Obtain Left and Right Rail Positions
				glm::vec3 LEFT_RAIL = LEFT_RAIL_VERTICES[i][k * 60];
				glm::vec3 RIGHT_RAIL = RIGHT_RAIL_VERTICES[i][k * 60];
				glm::vec3 LEFT_RAIL_2 = LEFT_RAIL_VERTICES[i][(k + 5) * 60];
				glm::vec3 RIGHT_RAIL_2 = RIGHT_RAIL_VERTICES[i][(k + 5) * 60];

				glm::vec3 BINORMAL = SPLINE_BINORMALS[i][k];
				glm::vec3 NORMAL = SPLINE_NORMALS[i][k];
				glm::vec3 BINORMAL_2 = SPLINE_BINORMALS[i][k + 2];
				glm::vec3 NORMAL_2 = SPLINE_NORMALS[i][k + 2];

				// Calculate Bar Points
				glm::vec3 BOTTOM_LEFT = LEFT_RAIL + 0.15f * BINORMAL - 0.002f * NORMAL;
				glm::vec3 TOP_LEFT = LEFT_RAIL_2 + 0.15f * BINORMAL_2 - 0.002f * NORMAL_2;
				glm::vec3 BOTTOM_RIGHT = RIGHT_RAIL - 0.15f * BINORMAL - 0.002f * NORMAL;
				glm::vec3 TOP_RIGHT = RIGHT_RAIL_2 - 0.15f * BINORMAL_2 - 0.002f * NORMAL_2;

				// Push Cross Bar Vertices
				pushCross(i, BOTTOM_LEFT, BOTTOM_RIGHT, TOP_RIGHT, TOP_LEFT);
				pushCross(i, BOTTOM_LEFT - 0.004f * NORMAL, BOTTOM_RIGHT - 0.004f * NORMAL, TOP_RIGHT - 0.004f * NORMAL_2, TOP_LEFT - 0.004f * NORMAL_2);
				pushCross(i, BOTTOM_LEFT, TOP_LEFT, TOP_LEFT - 0.004f * NORMAL_2, BOTTOM_LEFT - 0.004f * NORMAL);
				pushCross(i, BOTTOM_RIGHT, TOP_RIGHT, TOP_RIGHT - 0.004f * NORMAL_2, BOTTOM_RIGHT - 0.004f * NORMAL);
				pushCross(i, BOTTOM_LEFT, BOTTOM_RIGHT, BOTTOM_RIGHT - 0.004f * NORMAL_2, BOTTOM_LEFT - 0.004f * NORMAL);
				pushCross(i, TOP_LEFT, TOP_RIGHT, TOP_RIGHT - 0.004f * NORMAL_2, TOP_LEFT - 0.004f * NORMAL);
			}
		}
	}
}

// Initialize VAOs and VBOs
void initilaizeVBOs() {

	// Initailize Spaces for VAO and VBO for Rail and Cross Bar
	VAO = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_POSITION = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_NORMALS = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VAO_PHONG = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_PHONG_POSITION = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_PHONG_NORMALS = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VAO_CROSS = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_CROSS_POSITION = (GLuint*)malloc(numSplines * sizeof(GLuint));
	VBO_CROSS_UV = (GLuint*)malloc(numSplines * sizeof(GLuint));

	// Set Up VAOs and VBOs for Tracks and Cross Bars
	for (int i = 0; i < numSplines; i++) {
		// Generate and Bind VAO
		glGenVertexArrays(numSplines, &VAO[i]);
		glBindVertexArray(VAO[i]);

		// Generate and Bind VBO_POSITION and Store Rail Data
		glGenBuffers(numSplines, &VBO_POSITION[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION[i]);
		glBufferData(GL_ARRAY_BUFFER, (LEFT_RAIL_VERTICES[i].size() + RIGHT_RAIL_VERTICES[i].size()) * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, LEFT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), &LEFT_RAIL_VERTICES[i][0]);
		glBufferSubData(GL_ARRAY_BUFFER, LEFT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), RIGHT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), &RIGHT_RAIL_VERTICES[i][0]);

		// Generate and Bind VBO_NORMALS and Store Rail Data
		glGenBuffers(numSplines, &VBO_NORMALS[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_NORMALS[i]);
		glBufferData(GL_ARRAY_BUFFER, NORMALS[i].size() * 2 * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NORMALS[i].size() * sizeof(glm::vec4), &NORMALS[i][0]);
		glBufferSubData(GL_ARRAY_BUFFER, NORMALS[i].size() * sizeof(glm::vec4), NORMALS[i].size() * sizeof(glm::vec4), &NORMALS[i][0]);

		// Enable and Set Shader for VBO_POSITION
		glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION[i]);
		GLuint loc = glGetAttribLocation(program, "position"); //get the location of the "position" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Enable and Set Shader for VBO_NORMALS
		glBindBuffer(GL_ARRAY_BUFFER, VBO_NORMALS[i]);
		loc = glGetAttribLocation(program, "color"); //get the location of the "color" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

		// Generate and Bind VAO_PHONG
		glGenVertexArrays(numSplines, &VAO_PHONG[i]);
		glBindVertexArray(VAO_PHONG[i]);

		// Generate and Bind VBO_PHONG_POSITION and Store Rail Data
		glGenBuffers(numSplines, &VBO_PHONG_POSITION[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_PHONG_POSITION[i]);
		glBufferData(GL_ARRAY_BUFFER, (LEFT_RAIL_VERTICES[i].size() + RIGHT_RAIL_VERTICES[i].size()) * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, LEFT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), &LEFT_RAIL_VERTICES[i][0]);
		glBufferSubData(GL_ARRAY_BUFFER, LEFT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), RIGHT_RAIL_VERTICES[i].size() * sizeof(glm::vec3), &RIGHT_RAIL_VERTICES[i][0]);

		// Generate and Bind VBO_PHONG_NORMALS and Store Rail Data
		glGenBuffers(numSplines, &VBO_PHONG_NORMALS[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_PHONG_NORMALS[i]);
		glBufferData(GL_ARRAY_BUFFER, PHONG_NORMALS[i].size() * 2 * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, PHONG_NORMALS[i].size() * sizeof(glm::vec3), &PHONG_NORMALS[i][0]);
		glBufferSubData(GL_ARRAY_BUFFER, PHONG_NORMALS[i].size() * sizeof(glm::vec3), PHONG_NORMALS[i].size() * sizeof(glm::vec3), &PHONG_NORMALS[i][0]);

		// Enable and Set Shader for VBO_PHONG_POSITION
		glBindBuffer(GL_ARRAY_BUFFER, VBO_PHONG_POSITION[i]);
		loc = glGetAttribLocation(phongProgram, "position"); //get the location of the "position" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Enable and Set Shader for VBO_PHONG_NORMALS
		glBindBuffer(GL_ARRAY_BUFFER, VBO_PHONG_NORMALS[i]);
		loc = glGetAttribLocation(phongProgram, "normal"); //get the location of the "normal" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);


		// Generate and Bind VAO_CROSS
		glGenVertexArrays(numSplines, &VAO_CROSS[i]);
		glBindVertexArray(VAO_CROSS[i]);

		// Generate and Bind VBO_CROSS_POSITION
		glGenBuffers(numSplines, &VBO_CROSS_POSITION[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_CROSS_POSITION[i]);
		glBufferData(GL_ARRAY_BUFFER, CROSS_VERTICES[i].size() * sizeof(glm::vec3), &CROSS_VERTICES[i][0], GL_STATIC_DRAW);

		// Generate and Bind VBO_CROSS_UV
		glGenBuffers(numSplines, &VBO_CROSS_UV[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_CROSS_UV[i]);
		glBufferData(GL_ARRAY_BUFFER, CROSS_UVS[i].size() * sizeof(glm::vec2), &CROSS_UVS[i][0], GL_STATIC_DRAW);

		//glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_CROSS_POSITION[i]);
		loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_CROSS_UV[i]);
		loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}


	// Generate Texture Handle for Cross Bar Texture
	glGenTextures(1, &TEXTURE_HANDLE_CROSS);
	int code = initTexture("textures/Wood.jpg", TEXTURE_HANDLE_CROSS);
	if (code != 0) {
		printf("Error loading the texture image.\n");
		exit(EXIT_FAILURE);
	}

	
	// Generate Texture Handle for Ground Texture
    glGenTextures(1, &TEXTURE_HANDLE_GROUND);
    code = initTexture("textures/SkyBottom.jpg", TEXTURE_HANDLE_GROUND);
    if (code != 0) {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
    }
    // Generate and Bind VAO_GROUND
    glGenVertexArrays(1, &VAO_GROUND);
    glBindVertexArray(VAO_GROUND);
    
    // Generate and Bind VBO_GROUND_POSITION
    glGenBuffers(1, &VBO_GROUND_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_GROUND_POSITION);
    glBufferData(GL_ARRAY_BUFFER, GROUND_VERTICES.size() * sizeof(glm::vec3), &GROUND_VERTICES[0], GL_STATIC_DRAW);
    
    // Generate and Bind VBO_GROUND_UV
    glGenBuffers(1, &VBO_GROUND_UV);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_GROUND_UV);
    glBufferData(GL_ARRAY_BUFFER, GROUND_UVS.size() * sizeof(glm::vec2), &GROUND_UVS[0], GL_STATIC_DRAW);
    
    //glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_GROUND_POSITION);
    loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_GROUND_UV);
    loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);


	// Generate Texture Handle for Sky Texture
    glGenTextures(1, &TEXTURE_HANDLE_SKY);
    code = initTexture("textures/SkyTop.jpg", TEXTURE_HANDLE_SKY);
    if (code != 0) {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
    }
    
    // Generate and Bind VAO_SKY
    glGenVertexArrays(1, &VAO_SKY);
    glBindVertexArray(VAO_SKY);
    
    // Generate and Bind VBO_SKY_POSITION
    glGenBuffers(1, &VBO_SKY_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_SKY_POSITION);
    glBufferData(GL_ARRAY_BUFFER, SKY_VERTICES.size() * sizeof(glm::vec3), &SKY_VERTICES[0], GL_STATIC_DRAW);
    
    // Generate and Bind VBO_SKY_UV
    glGenBuffers(1, &VBO_SKY_UV);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_SKY_UV);
    glBufferData(GL_ARRAY_BUFFER, SKY_UVS.size() * sizeof(glm::vec2), &SKY_UVS[0], GL_STATIC_DRAW);
    
    //glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_SKY_POSITION);
    loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_SKY_UV);
    loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
 

	// Generate Texture Handle for Front Texture
    glGenTextures(1, &TEXTURE_HANDLE_FRONT);
    code = initTexture("textures/SkyFront.jpg", TEXTURE_HANDLE_FRONT);
    if (code != 0) {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
    }
    
    // Generate and Bind VAO_FRONT
    glGenVertexArrays(1, &VAO_FRONT);
    glBindVertexArray(VAO_FRONT);
    
    // Generate and Bind VBO_FRONT_POSITION
    glGenBuffers(1, &VBO_FRONT_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_FRONT_POSITION);
    glBufferData(GL_ARRAY_BUFFER, FRONT_VERTICES.size() * sizeof(glm::vec3), &FRONT_VERTICES[0], GL_STATIC_DRAW);
    
    // Generate and Bind VBO_SKY_UV
    glGenBuffers(1, &VBO_FRONT_UV);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_FRONT_UV);
    glBufferData(GL_ARRAY_BUFFER, FRONT_UVS.size() * sizeof(glm::vec2), &FRONT_UVS[0], GL_STATIC_DRAW);
    
    //glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_FRONT_POSITION);
    loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_FRONT_UV);
    loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);


	// Generate Texture Handle for Right Texture
    glGenTextures(1, &TEXTURE_HANDLE_RIGHT);
    code = initTexture("textures/SkyRight.jpg", TEXTURE_HANDLE_RIGHT);
    if (code != 0) {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
    }
    
    // Generate and Bind VAO_RIGHT
    glGenVertexArrays(1, &VAO_RIGHT);
    glBindVertexArray(VAO_RIGHT);
    
    // Generate and Bind VBO_RIGHT_POSITION
    glGenBuffers(1, &VBO_RIGHT_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_RIGHT_POSITION);
    glBufferData(GL_ARRAY_BUFFER, RIGHT_VERTICES.size() * sizeof(glm::vec3), &RIGHT_VERTICES[0], GL_STATIC_DRAW);
    
    // Generate and Bind VBO_RIGHT_UV
    glGenBuffers(1, &VBO_RIGHT_UV);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_RIGHT_UV);
    glBufferData(GL_ARRAY_BUFFER, RIGHT_UVS.size() * sizeof(glm::vec2), &RIGHT_UVS[0], GL_STATIC_DRAW);
    
    //glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_RIGHT_POSITION);
    loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_RIGHT_UV);
    loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);


	// Generate Texture Handle for Left Texture
	glGenTextures(1, &TEXTURE_HANDLE_LEFT);
	code = initTexture("textures/SkyLeft.jpg", TEXTURE_HANDLE_LEFT);
	if (code != 0) {
		printf("Error loading the texture image.\n");
		exit(EXIT_FAILURE);
	}

	// Generate and Bind VAO_LEFT
	glGenVertexArrays(1, &VAO_LEFT);
	glBindVertexArray(VAO_LEFT);

	// Generate and Bind VBO_LEFT_POSITION
	glGenBuffers(1, &VBO_LEFT_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_LEFT_POSITION);
	glBufferData(GL_ARRAY_BUFFER, LEFT_VERTICES.size() * sizeof(glm::vec3), &LEFT_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_LEFT_UV
	glGenBuffers(1, &VBO_LEFT_UV);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_LEFT_UV);
	glBufferData(GL_ARRAY_BUFFER, LEFT_UVS.size() * sizeof(glm::vec2), &LEFT_UVS[0], GL_STATIC_DRAW);

	//glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_LEFT_POSITION);
	loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_LEFT_UV);
	loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);


	// Generate Texture Handle for Back Texture
	glGenTextures(1, &TEXTURE_HANDLE_BACK);
	code = initTexture("textures/SkyBack.jpg", TEXTURE_HANDLE_BACK);
	if (code != 0) {
		printf("Error loading the texture image.\n");
		exit(EXIT_FAILURE);
	}

	// Generate and Bind VAO_BACK
	glGenVertexArrays(1, &VAO_BACK);
	glBindVertexArray(VAO_BACK);

	// Generate and Bind VBO_BACK_POSITION
	glGenBuffers(1, &VBO_BACK_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BACK_POSITION);
	glBufferData(GL_ARRAY_BUFFER, BACK_VERTICES.size() * sizeof(glm::vec3), &BACK_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_BACK_UV
	glGenBuffers(1, &VBO_BACK_UV);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BACK_UV);
	glBufferData(GL_ARRAY_BUFFER, BACK_UVS.size() * sizeof(glm::vec2), &BACK_UVS[0], GL_STATIC_DRAW);

	//glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BACK_POSITION);
	loc = glGetAttribLocation(textureProgram, "position"); //get the location of the "position" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_BACK_UV);
	loc = glGetAttribLocation(textureProgram, "texCoord"); //get the location of the "texCoord" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

// Select Texture Unit to be Used
void setTextureUnit(GLint unit) {
	// Select the Active Texture Unit
	glActiveTexture(unit); 
	// Get Handle to the "textureImage" Shader Variable
	GLint h_textureImage = glGetUniformLocation(textureProgram, "textureImage");
	// Deem the Shader Variable "textureImage" to Read From Texture Unit "unit"
	glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

// Display According to Display Mode
void displayFunc() {

	// Clear Color and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (displayType) {
		case COLOR_RAIL:

			// Bind the Pipeline Program
			pipelineProgram.Bind();

			// Initalize Modelview and Perspective Matrices
			initializeMatrix(1);

			glBindVertexArray(VAO[ID]);
			// Using vec3 thus do not have to be divided by 3
			glDrawArrays(GL_TRIANGLES, 0, LEFT_RAIL_VERTICES[ID].size());
			glDrawArrays(GL_TRIANGLES, LEFT_RAIL_VERTICES[ID].size(), RIGHT_RAIL_VERTICES[ID].size());

		break;

		case PHONG_RAIL:

			// Bind the Pipeline Program
			pipelineProgramPhong.Bind();

			// Initalize Modelview, Perspective, Normal Matrices and Other Uniforms
			initializeMatrix(0);

			glBindVertexArray(VAO_PHONG[ID]);
			// Using vec3 thus do not have to be divided by 3
			glDrawArrays(GL_TRIANGLES, 0, LEFT_RAIL_VERTICES[ID].size());
			glDrawArrays(GL_TRIANGLES, LEFT_RAIL_VERTICES[ID].size(), RIGHT_RAIL_VERTICES[ID].size());

		break;
	}

	// Bind the Texture Pipeline Program
	pipelineTextureProgram.Bind();

	// Initalize Modelview and Perspective Matrices
	initializeMatrix(2);

	// Draw Ground and Sky
	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_GROUND);
	glBindVertexArray(VAO_GROUND);
	glDrawArrays(GL_TRIANGLES, 0, GROUND_VERTICES.size());
    
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_SKY);
    glBindVertexArray(VAO_SKY);
    glDrawArrays(GL_TRIANGLES, 0, SKY_VERTICES.size());
    
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_FRONT);
    glBindVertexArray(VAO_FRONT);
    glDrawArrays(GL_TRIANGLES, 0, FRONT_VERTICES.size());
    
    setTextureUnit(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_RIGHT);
    glBindVertexArray(VAO_RIGHT);
    glDrawArrays(GL_TRIANGLES, 0, RIGHT_VERTICES.size());

	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_LEFT);
	glBindVertexArray(VAO_LEFT);
	glDrawArrays(GL_TRIANGLES, 0, LEFT_VERTICES.size());

	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_BACK);
	glBindVertexArray(VAO_BACK);
	glDrawArrays(GL_TRIANGLES, 0, BACK_VERTICES.size());

	setTextureUnit(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_HANDLE_CROSS);
	glBindVertexArray(VAO_CROSS[ID]);
	glDrawArrays(GL_TRIANGLES, 0, CROSS_VERTICES[ID].size());

	// Unbind VAO
	glBindVertexArray(0);

	// Swap Buffers
	glutSwapBuffers();
}

// Animation at the Beginning
void idleFunc() {

	// u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du)
	float height = SPLINE_POINTS[ID][step].z;
	glm::vec3 tangent = SPLINE_TANGENTS[ID][step];

	if (step < SPLINE_POINTS[ID].size() && play) {
		add += dt * (sqrt(2 * g * (MAX_HEIGHT - height)) / glm::length(tangent));
		pre.push_back(step);
		pre_add.push_back(add);
		if ((int)(add / MAX_LENGTH) < SPLINE_POINTS[ID].size()) {
			step = (int)(add / MAX_LENGTH);
		}
		/*if (screenshotCounter < 999 && step % 10 == 0) {
			char anim_num[5];
			sprintf(anim_num, "%03d", ++screenshotCounter);
			saveScreenshot(("./animation/" + string(anim_num) + ".jpg").c_str());
		}*/
	}
	
	// Make the Screen Update 
	glutPostRedisplay();
}

// Set Viewport, Prepare Projection Matrix and Set Back to ModelView Mode in the End of the Function
void reshapeFunc(int w, int h) {
	glViewport(0, 0, w, h);

	// Set Projection Matrix
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	// Load Identity Matrix
	matrix.LoadIdentity();
	// Set Prespective to Field of View = 45 degrees, Aspect Ratio = 1280:720, zNear = 0.01, zFar = 1000
	matrix.Perspective(fovy, aspect, 0.01, 1000.0);
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

// Set Action when Mouse Dragged
void mouseMotionDragFunc(int x, int y) {
	// Mouse has Moved and One of the Mouse Buttons is Pressed (Dragging)
	// The Change in Mouse Position Since the Last Invocation of This Function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState) {
		// Translate the Landscape
	case TRANSLATE:
		if (leftMouseButton) {
			// Control x, y Translation via the Left Mouse Button
			landTranslate[0] += mousePosDelta[0] * 0.01f;
			landTranslate[1] -= mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton) {
			// Control z Translation via the Middle Mouse Button
			landTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		break;

		// Rotate the Landscape
	case ROTATE:
		if (leftMouseButton) {
			// Control x, y Rotation via the Left Mouse Button
			landRotate[0] += mousePosDelta[1];
			landRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton) {
			// control z Rotation via the Middle Mouse Button
			landRotate[2] += mousePosDelta[1];
		}
		break;

		// Scale the Landscape
	case SCALE:
		if (leftMouseButton) {
			// Control x, y Scaling via the Left Mouse Button
			landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton) {
			// Control z Scaling via the Middle Mouse Button
			landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// Store the New Mouse Position
	mousePos[0] = x;
	mousePos[1] = y;
}

// Set Action when Mouse Moved
void mouseMotionFunc(int x, int y) {
	// Mouse has Moved
	// Store the New Mouse Position
	mousePos[0] = x;
	mousePos[1] = y;
}

// Set Action when Mouse Clicked
void mouseButtonFunc(int button, int state, int x, int y) {
	// A Mouse Button has been Pressed or Unpressed

	// Keep Track of the Mouse Button State, in leftMouseButton, middleMouseButton, rightMouseButton Variables
	switch (button) {
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// Keep Track of Whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers()) {
		// If CTRL is Pressed, We are in Translate Mode
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

		// If SHIFT is Pressed, We are in Scale Mode
	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// If CTRL and SHIFT are Not Pressed, We are in Rotate Mode
	default:
		controlState = ROTATE;
		break;
	}

	// Store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

// Set Action when Key Pressed
void keyboardFunc(unsigned char key, int x, int y) {

	float height = SPLINE_POINTS[ID][step].z;
	glm::vec3 tangent = SPLINE_TANGENTS[ID][step];

	switch (key) {
	case 27: // ESC key
		exit(0); // Exit the program
		break;

	case 'x':
		// Take a Screenshot
		saveScreenshot("screenshot.jpg");
		break;

	case 'w':
		add += dt * (sqrt(2 * g * (MAX_HEIGHT - height)) / glm::length(tangent));
		pre.push_back(step);
		pre_add.push_back(add);
		if ((int)(add / MAX_LENGTH) < SPLINE_POINTS[ID].size()) {
			step = (int)(add / MAX_LENGTH);
		}
		break;

	case 's':
		if (pre.size() != 0) {
			step = pre[pre.size() - 1];
			add = pre_add[pre_add.size() - 1];
			cout << step << " " << add << endl;
			pre.pop_back();
			pre_add.pop_back();
		}
		break;
        
	case 'a':
		if (degree2 > -1 && (int)degree == 1) {
			degree2 -= 0.1f;
		}
		else if ((int)degree2 == 1 && degree < 1) {
			degree += 0.1f;
		}
		else if (degree2 < 1 && (int)degree == -1) {
			degree2 += 0.1f;
		}
		else if ((int)degree2 == -1 && degree > -1) {
			degree -= 0.1f;
		}
        break;
        
    case 'd':
		if (degree2 < 1 && (int)degree == 1) {
			degree2 += 0.1f;
		}
		else if ((int)degree2 == 1 && degree > -1) {
			degree -= 0.1f;
		}
		else if (degree2 > -1 && (int)degree == -1) {
			degree2 -= 0.1f;
		}
		else if ((int)degree2 == -1 && degree < 1) {
			degree += 0.1f;
		}
        break;

	case 'e':
		if (ID < numSplines - 1) { ID++; }
		landTranslate[0] = 0;
		landTranslate[1] = 0;
		landTranslate[2] = 0;
		landRotate[0] = 0;
		landRotate[1] = 0;
		landRotate[2] = 0;
		landScale[0] = 1;
		landScale[1] = 1;
		landScale[2] = 1;
		step = 0;
		add = 0;
		play = false;
		break;

	case 'q':
		if (ID > 0) { ID--; }
		landTranslate[0] = 0;
		landTranslate[1] = 0;
		landTranslate[2] = 0;
		landRotate[0] = 0;
		landRotate[1] = 0;
		landRotate[2] = 0;
		landScale[0] = 1;
		landScale[1] = 1;
		landScale[2] = 1;
		step = 0;
		add = 0;
		play = false;
		break;

	case '1':
		displayType = COLOR_RAIL;
		break;

	case '2':
		displayType = PHONG_RAIL;
		break;

	case 'r':
		landTranslate[0] = 0;
		landTranslate[1] = 0;
		landTranslate[2] = 0;
		landRotate[0] = 0;
		landRotate[1] = 0;
		landRotate[2] = 0;
		landScale[0] = 1;
		landScale[1] = 1;
		landScale[2] = 1;
		step = 0;
		add = 0;
		play = false;
		break;

	case ' ':
		play = play ? false : true;
		break;
	}
}

// Initialize All Required Materials before Display
void initScene(int argc, char *argv[]) {

	// Set Background to Black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable Depth Test
	glEnable(GL_DEPTH_TEST); 

	// Initialize and Get Handle of the Pipeline Program of 3 Shaders
	pipelineProgram.Init("../openGLHelper-starterCode", "basic.vertexShader.glsl", "basic.fragmentShader.glsl");
	program = pipelineProgram.GetProgramHandle();

	pipelineProgramPhong.Init("../openGLHelper-starterCode", "basic.phongVertexShader.glsl", "basic.phongFragmentShader.glsl");
	phongProgram = pipelineProgramPhong.GetProgramHandle();

	pipelineTextureProgram.Init("../openGLHelper-starterCode", "basic.textureVertexShader.glsl", "basic.textureFragmentShader.glsl");
	textureProgram = pipelineTextureProgram.GetProgramHandle();

	// Get Uniform Matrix Location for 3 Shaders
	h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
	h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

	h_modelViewMatrix_phong = glGetUniformLocation(phongProgram, "modelViewMatrix");
	h_projectionMatrix_phong = glGetUniformLocation(phongProgram, "projectionMatrix");
	h_normalMatrix_phong = glGetUniformLocation(phongProgram, "normalMatrix");

	modelViewMatrix = glGetUniformLocation(textureProgram, "modelViewMatrix");
	projectionMatrix = glGetUniformLocation(textureProgram, "projectionMatrix");

	// Initialize Spline Points
	initializeVertices();

	// Initialize Textures
    initializeTexureVertices(glm::vec3(-64, -64, -1), glm::vec3(64, 64, -1),  GROUND_VERTICES, GROUND_UVS);
    initializeTexureVertices(glm::vec3(-64, 64, 63), glm::vec3(64, -64, 63), SKY_VERTICES, SKY_UVS);
    initializeTexureVertices(glm::vec3(-64, 64, -1), glm::vec3(64, 64, 63), FRONT_VERTICES, FRONT_UVS); 
    initializeTexureVertices(glm::vec3(-64, -64, -1), glm::vec3(-64, 64, 63), RIGHT_VERTICES, RIGHT_UVS);  
    initializeTexureVertices(glm::vec3(64, 64, -1), glm::vec3(64, -64, 63), LEFT_VERTICES, LEFT_UVS);
    initializeTexureVertices(glm::vec3(64, -64, -1), glm::vec3(-64, -64, 63), BACK_VERTICES, BACK_UVS);

	// Initialize VAOs and VBOs
	initilaizeVBOs();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	// load the splines from the provided filename
	loadSplines(argv[1]);

	printf("Loaded %d spline(s).\n", numSplines);
	for (int i = 0; i < numSplines; i++)
		printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
	// nothing is needed on Apple
#else
	// Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK) {
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();

	return 0;
}


