#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

#define TEACUP_PATCHES 26
#define TEACUP_VERTICES 251
#define VERTEX_ORDER 3

#define RESOLUTION_U 20
#define RESOLUTION_V 20

//--------------------------------------------------------------------------------------------------
int screen_width = 640, screen_height = 640;

//vbos and ibos
GLuint vbo_teacupVertices, vbo_teacupColors, ibo_teacupElements,
vbo_teacupCPVertices, vbo_teacupCPColors, ibo_teacupCPElements;
GLuint shaderProgram;
GLint attribute_coord3d, attribute_v_color;
GLint uniform_mvp;
struct struct_vertex { GLfloat x, y, z; };

struct struct_vertex teacup_vertices[TEACUP_PATCHES * RESOLUTION_U*RESOLUTION_V];
GLfloat teacup_colors[TEACUP_PATCHES * RESOLUTION_U*RESOLUTION_V * VERTEX_ORDER];
GLushort teacup_elements[TEACUP_PATCHES * (RESOLUTION_U - 1)*(RESOLUTION_V - 1) * 2 * VERTEX_ORDER];

GLfloat teacup_cp_colors[TEACUP_VERTICES * VERTEX_ORDER];
GLushort teacup_cp_elements[TEACUP_PATCHES][VERTEX_ORDER + 1][VERTEX_ORDER + 1];

void build_control_points_k(int p, struct struct_vertex control_points_k[][VERTEX_ORDER + 1]);
struct struct_vertex compute_position(struct struct_vertex control_points_k[][VERTEX_ORDER + 1], float u, float v);
float bernstein_polynomial(int i, int n, float u);
float binomial_coefficient(int i, int n);
int factorial(int n);
//--------------------------------------------------------------------------------------------------
struct struct_vertex teacupCPVertices[TEACUP_VERTICES];
int teacupPatches[TEACUP_PATCHES][VERTEX_ORDER + 1][VERTEX_ORDER + 1];

//---------------------------------------------------------------------------------------------------
void fillArrays()
{
	FILE *fin = fopen("teacup", "r");

	int patchSize, verticesSize;

	int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
	float x, y, z;

	while (!feof(fin))
	{
		fscanf(fin, "%i", &patchSize);
		for (int fines = 0; fines < patchSize; fines++)
		{
			(void)fscanf(fin, "%i, %i, %i, %i,", &a, &b, &c, &d);
			(void)fscanf(fin, "%i, %i, %i, %i,", &e, &f, &g, &h);
			(void)fscanf(fin, "%i, %i, %i, %i,", &i, &j, &k, &l);
			(void)fscanf(fin, "%i, %i, %i, %i\n", &m, &n, &o, &p);

			for (int fir = 0; fir < 4; fir++)
			{
				for (int sec = 0; sec < 4; sec++)
				{
					if (fir == 0 && sec == 0)
						teacupPatches[fines][fir][sec] = a;
					else if (fir == 0 && sec == 1)
						teacupPatches[fines][fir][sec] = b;
					else if (fir == 0 && sec == 2)
						teacupPatches[fines][fir][sec] = c;
					else if (fir == 0 && sec == 3)
						teacupPatches[fines][fir][sec] = d;
					else if (fir == 1 && sec == 0)
						teacupPatches[fines][fir][sec] = e;
					else if (fir == 1 && sec == 1)
						teacupPatches[fines][fir][sec] = f;
					else if (fir == 1 && sec == 2)
						teacupPatches[fines][fir][sec] = g;
					else if (fir == 1 && sec == 3)
						teacupPatches[fines][fir][sec] = h;
					else if (fir == 2 && sec == 0)
						teacupPatches[fines][fir][sec] = i;
					else if (fir == 2 && sec == 1)
						teacupPatches[fines][fir][sec] = j;
					else if (fir == 2 && sec == 2)
						teacupPatches[fines][fir][sec] = k;
					else if (fir == 2 && sec == 3)
						teacupPatches[fines][fir][sec] = l;
					else if (fir == 3 && sec == 0)
						teacupPatches[fines][fir][sec] = m;
					else if (fir == 3 && sec == 1)
						teacupPatches[fines][fir][sec] = n;
					else if (fir == 3 && sec == 2)
						teacupPatches[fines][fir][sec] = o;
					else if (fir == 3 && sec == 3)
						teacupPatches[fines][fir][sec] = p;
				}
			}
		}

		fscanf(fin, "%i", &verticesSize);

		for (int fines = 0; fines < verticesSize; fines++)
		{
			(void)fscanf(fin, "%f, %f, %f\n", &x, &y, &z);
			teacupCPVertices[fines] = { x, y, z };
		}
	}

	fclose(fin);
}


//--------------------------------------------------------------------------------------------------

void buildTeacup() {
	// Vertices
	for (int p = 0; p < TEACUP_PATCHES; p++) {
		struct struct_vertex control_points_k[VERTEX_ORDER + 1][VERTEX_ORDER + 1];
		build_control_points_k(p, control_points_k);
		for (int ru = 0; ru <= RESOLUTION_U - 1; ru++) {
			float u = 1.0 * ru / (RESOLUTION_U - 1);
			for (int rv = 0; rv <= RESOLUTION_V - 1; rv++) {
				float v = 1.0 * rv / (RESOLUTION_V - 1);
				teacup_vertices[p*RESOLUTION_U*RESOLUTION_V + ru * RESOLUTION_V + rv] = compute_position(control_points_k, u, v);
				teacup_colors[p*RESOLUTION_U*RESOLUTION_V * 3 + ru * RESOLUTION_V * 3 + rv * 3 + 0] = 1.0 * p / TEACUP_PATCHES;
				teacup_colors[p*RESOLUTION_U*RESOLUTION_V * 3 + ru * RESOLUTION_V * 3 + rv * 3 + 1] = 1.0 * p / TEACUP_PATCHES;
				teacup_colors[p*RESOLUTION_U*RESOLUTION_V * 3 + ru * RESOLUTION_V * 3 + rv * 3 + 2] = 0.8;
			}
		}
	}

	// Elements
	int n = 0;
	for (int p = 0; p < TEACUP_PATCHES; p++)
		for (int ru = 0; ru < RESOLUTION_U - 1; ru++)
			for (int rv = 0; rv < RESOLUTION_V - 1; rv++) {
				// 1 square ABCD = 2 triangles ABC + CDA
				// ABC
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + ru * RESOLUTION_V + rv; n++;
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + ru * RESOLUTION_V + (rv + 1); n++;
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + (ru + 1)*RESOLUTION_V + (rv + 1); n++;
				// CDA
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + (ru + 1)*RESOLUTION_V + (rv + 1); n++;
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + (ru + 1)*RESOLUTION_V + rv; n++;
				teacup_elements[n] = p * RESOLUTION_U*RESOLUTION_V + ru * RESOLUTION_V + rv; n++;
			}

	// Control points elements for debugging
	memset(teacup_cp_colors, 0, sizeof(teacup_cp_colors)); // black
	for (int p = 0; p < TEACUP_PATCHES; p++)
		for (int i = 0; i < (VERTEX_ORDER + 1); i++)
			for (int j = 0; j < (VERTEX_ORDER + 1); j++)
				teacup_cp_elements[p][i][j] = teacupPatches[p][i][j] - 1;
}

//--------------------------------------------------------------------------------------------------

void build_control_points_k(int p, struct struct_vertex control_points_k[][VERTEX_ORDER + 1]) {
	for (int i = 0; i <= VERTEX_ORDER; i++)
		for (int j = 0; j <= VERTEX_ORDER; j++)
			control_points_k[i][j] = teacupCPVertices[teacupPatches[p][i][j] - 1];
}

//--------------------------------------------------------------------------------------------------

struct struct_vertex compute_position(struct struct_vertex control_points_k[][VERTEX_ORDER + 1], float u, float v) {
	struct struct_vertex result = { 0.0, 0.0, 0.0 };
	for (int i = 0; i <= VERTEX_ORDER; i++) {
		float poly_i = bernstein_polynomial(i, VERTEX_ORDER, u);
		for (int j = 0; j <= VERTEX_ORDER; j++) {
			float poly_j = bernstein_polynomial(j, VERTEX_ORDER, v);
			result.x += poly_i * poly_j * control_points_k[i][j].x;
			result.y += poly_i * poly_j * control_points_k[i][j].y;
			result.z += poly_i * poly_j * control_points_k[i][j].z;
		}
	}
	return result;
}

//--------------------------------------------------------------------------------------------------

float bernstein_polynomial(int i, int n, float u) {
	return binomial_coefficient(i, n) * powf(u, i) * powf(1 - u, n - i);
}

//--------------------------------------------------------------------------------------------------

float binomial_coefficient(int i, int n) {
	assert(i >= 0); assert(n >= 0);
	return 1.0f * factorial(n) / (factorial(i) * factorial(n - i));
}
int factorial(int n) {
	assert(n >= 0);
	int result = 1;
	for (int i = n; i > 1; i--)
		result *= i;
	return result;
}

//--------------------------------------------------------------------------------------------------

int init_resources()
{
	buildTeacup();

	glGenBuffers(1, &vbo_teacupVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teacup_vertices), teacup_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_teacupColors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupColors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teacup_colors), teacup_colors, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo_teacupElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teacupElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teacup_elements), teacup_elements, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_teacupCPVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupCPVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teacupCPVertices), teacupCPVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_teacupCPColors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupCPColors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teacup_cp_colors), teacup_cp_colors, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo_teacupCPElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teacupCPElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teacup_cp_elements), teacup_cp_elements, GL_STATIC_DRAW);

	GLint link_ok = GL_FALSE;

	GLuint vs, fs;
	if ((vs = create_shader("vertexShader.glsl", GL_VERTEX_SHADER)) == 0) return 0;
	if ((fs = create_shader("fragmentShader.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		print_log(shaderProgram);
		return 0;
	}

	const char* attribute_name;
	attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(shaderProgram, attribute_name);
	if (attribute_coord3d == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
	attribute_name = "v_color";
	attribute_v_color = glGetAttribLocation(shaderProgram, attribute_name);
	if (attribute_v_color == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
	const char* uniform_name;
	uniform_name = "mvp";
	uniform_mvp = glGetUniformLocation(shaderProgram, uniform_name);
	if (uniform_mvp == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------------------------------

void onIdle() {
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * glm::radians(15.0); 
	glm::mat4 anim =
		glm::rotate(glm::mat4(1.0f), 1 * angle, glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1.0f), 2 * angle, glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1.0f), 3 * angle, glm::vec3(0, 0, 1)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.5));

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 6.5), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 projection = glm::perspective(43.0f, 1.0f*screen_width / screen_height, 0.1f, 10.0f);

	glm::mat4 modelViewProjection = projection * view * model * anim;

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
	glutPostRedisplay();
}

//--------------------------------------------------------------------------------------------------

void onDisplay()
{
	glClearColor(1.0, 1.0, 0.93, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Teacup
	glUseProgram(shaderProgram);
	glEnableVertexAttribArray(attribute_coord3d);

	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupVertices);
	glVertexAttribPointer(
		attribute_coord3d, // attribute
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	glEnableVertexAttribArray(attribute_v_color);

	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupColors);
	glVertexAttribPointer(
		attribute_v_color,
		3,                
		GL_FLOAT,          
		GL_FALSE,          
		0,                
		0                
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teacupElements);
	glDrawElements(GL_TRIANGLES, sizeof(teacup_elements) / sizeof(teacup_elements[0]), GL_UNSIGNED_SHORT, 0);


	// Draw Control points
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupCPVertices);
	glVertexAttribPointer(
		attribute_coord3d, 
		3,                
		GL_FLOAT,          
		GL_FALSE,        
		0,                 
		0                
	);

	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_teacupCPColors);
	glVertexAttribPointer(
		attribute_v_color, 
		3,                 
		GL_FLOAT,        
		GL_FALSE,         
		0,             
		0                  
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teacupCPElements);
	size_t offset = 0;  // size_t instead of GLushort to fix 'warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]'
	for (int p = 0; p < TEACUP_PATCHES; p++)
		for (int i = 0; i < VERTEX_ORDER + 1; i++, offset += (VERTEX_ORDER + 1) * sizeof(GLushort))
			glDrawElements(GL_LINE_LOOP, VERTEX_ORDER + 1, GL_UNSIGNED_SHORT, (GLvoid*)offset);

	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);
	glutSwapBuffers();
}

//--------------------------------------------------------------------------------------------------

void onReshape(int width, int height) {
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}

//--------------------------------------------------------------------------------------------------

void free_resources()
{
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo_teacupVertices);
	glDeleteBuffers(1, &vbo_teacupColors);
	glDeleteBuffers(1, &ibo_teacupElements);
	glDeleteBuffers(1, &vbo_teacupCPVertices);
	glDeleteBuffers(1, &vbo_teacupCPColors);
	glDeleteBuffers(1, &ibo_teacupCPElements);
}

//--------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {

	fillArrays();


	glutInit(&argc, argv);
	glutInitContextVersion(2, 0);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("Rotating Teacup");

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}

	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
		return 1;
	}

	if (init_resources()) {
		glutDisplayFunc(onDisplay);
		glutReshapeFunc(onReshape);
		glutIdleFunc(onIdle);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glutMainLoop();
	}

	free_resources();
	return 0;
}
