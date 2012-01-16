#include <stdafx.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>
#include <vector>

struct punktowe_swiatlo {
   float position[3];
   float intensity_diffuse[3];
   float intensity_specular[3];
   float attenuation[3];

   void set_position(float x, float y, float z) {
  	position[0] = x;
  	position[1] = y;
  	position[2] = z;
   }

   void set_intensity_diffuse(float r, float g, float b) {
  	intensity_diffuse[0] = r;
  	intensity_diffuse[1] = g;
  	intensity_diffuse[2] = b;
   }

   void set_intensity_specular(float r, float g, float b) {
  	intensity_specular[0] = r;
  	intensity_specular[1] = g;
  	intensity_specular[2] = b;
   }

     void set_attenuation(float attenuation_0, float attenuation_1, float attenuation_2) {
  	attenuation[0] = attenuation_0;
  	attenuation[1] = attenuation_1;
  	attenuation[2] = attenuation_2;
   }
};

struct Material {
   float r_ambient; 
   float r_diffuse; 
   float r_spectacular;  
   float alpha;

   void set_parameters(float r_ambient, float r_diffuse, float r_spectacular, float alpha) {
  	this->r_ambient = r_ambient;
  	this->r_diffuse = r_diffuse;
  	this->r_spectacular = r_spectacular;
  	this->alpha = alpha;
   }
};
GLuint shader_swiatlo,
   	ShaderColor,
   	MVPMatrixLocation_ShaderColor,
   	MVPMatrixLocation,
   	mvMatrix_location,
   	vMatrix_location,
   	NormalMatrix_location,
   	intensity_ambient_component_location,
   	swiatlo_0_position_location,
   	swiatlo_0_intensity_diffuse_location,
   	swiatlo_0_intensity_specular_location,
   	swiatlo_0_attenuation_location,
   	material_0_r_ambient_location,
   	material_0_r_diffuse_location,
   	material_0_r_spectacular_location,
   	material_0_alpha_location;
GLGeometryTransform geometry_pipeline;
GLMatrixStack p_stack;
GLMatrixStack mv_stack;
GLFrame cameraFrame;
GLFrustum viewFrustum;

//ustawienie poczatkowe kamery
float location[] = {1.5f, -1.0f, -1.5f};
float target[] = {0.0f, 0.0f, 0.0f};
float up_dir[] = {0.0f, 0.0f, 1.0f};
float camera_matrix[16];
float intensity_ambient_component[] = {0.2f, 0.2f, 0.2f};


CStopWatch timer;
punktowe_swiatlo swiatlo_0;
Material material_0;
std::vector<float> vertices;
std::vector<unsigned int> faces;
GLuint vertex_buffer;
GLuint faces_buffer;

void load_vertices(const char * filename, std::vector<float> & out_vertices) {
   FILE * fvertices = fopen(filename, "r");
   char line[120];
   float x, y, z, magnitude;
   while (fgets(line, 120, fvertices) != NULL) {
  	sscanf(line,"%f %f %f", &x, &y, &z);
  	magnitude = sqrt(x * x + y * y + z * z);
  	out_vertices.push_back(x);
  	out_vertices.push_back(y);
  	out_vertices.push_back(z);
  	out_vertices.push_back(1.0f);
  	out_vertices.push_back(x / magnitude);
  	out_vertices.push_back(y / magnitude);
  	out_vertices.push_back(z / magnitude);
   }
}

void load_faces(const char * filename, std::vector<unsigned int> & out_faces) {
   FILE * ffaces = fopen(filename, "r");
   char line[120];
   int i, j, k;
   while (fgets(line, 120, ffaces) != NULL) {
  	sscanf(line, "%u %u %u", &i, &j, &k);
  	out_faces.push_back(i - 1);
  	out_faces.push_back(j - 1);
  	out_faces.push_back(k - 1);
   }
}

void SetUpFrame(GLFrame & frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
   frame.SetOrigin(origin);
   frame.SetForwardVector(forward);
   M3DVector3f side, oUp;
   m3dCrossProduct3(side, forward, up);
   m3dCrossProduct3(oUp, side, forward);
   frame.SetUpVector(oUp);
   frame.Normalize();
}
void LookAt(GLFrame & frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
   M3DVector3f forward;
   m3dSubtractVectors3(forward, at, eye);
   SetUpFrame(frame, eye, forward, up);
}

void change_size(int w, int h) {
   viewFrustum.SetPerspective(70.0f, (float)w / h, 1.0f, 150.0f);
   glViewport(0, 0, w, h);
}
void SetupRC() {
   load_vertices("geode_vertices.dat", vertices);
   load_faces("geode_faces.dat", faces);
   glGenBuffers(1, &vertex_buffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
   glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 4 ,GL_FLOAT,GL_FALSE, sizeof(float) * 7, (const GLvoid *)0);
   glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const GLvoid *)(4*sizeof(float)) );
   glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
   glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
   //---
   glGenBuffers(1, &faces_buffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_buffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint), &faces[0], GL_STATIC_DRAW);
   //---
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader_swiatlo = gltLoadShaderPairWithAttributes("gouraud_shader.vp", "gouraud_shader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
   ShaderColor = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor");
   MVPMatrixLocation_ShaderColor = glGetUniformLocation(ShaderColor, "mvpMatrix");
   MVPMatrixLocation = glGetUniformLocation(shader_swiatlo, "mvpMatrix");
   mvMatrix_location = glGetUniformLocation(shader_swiatlo, "mvMatrix");
   vMatrix_location = glGetUniformLocation(shader_swiatlo, "vMatrix");
   NormalMatrix_location = glGetUniformLocation(shader_swiatlo, "NormalMatrix");
   intensity_ambient_component_location = glGetUniformLocation(shader_swiatlo, "intensity_ambient_component");
   swiatlo_0_position_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.position");
   swiatlo_0_intensity_diffuse_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_diffuse");
   swiatlo_0_intensity_specular_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_specular");
   swiatlo_0_attenuation_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.attenuation");
   material_0_r_ambient_location = glGetUniformLocation(shader_swiatlo, "material_0.r_ambient");
   material_0_r_diffuse_location = glGetUniformLocation(shader_swiatlo, "material_0.r_diffuse");
   material_0_r_spectacular_location = glGetUniformLocation(shader_swiatlo, "material_0.r_spectacular");
   material_0_alpha_location = glGetUniformLocation(shader_swiatlo, "material_0.alpha");
   swiatlo_0.set_position(0.0f, 0.0f, 0.0f);
   swiatlo_0.set_intensity_diffuse(1.0f, 1.0f, 1.0f);
   swiatlo_0.set_intensity_specular(0.0f, 0.0f, 0.0f);
   swiatlo_0.set_attenuation(0.1f, 0.1f, 0.0f);// Jasnoœæ œwiat³a od obiektu
   material_0.set_parameters(1.0f, 1.0f, 1.0f, 200.0f);
   geometry_pipeline.SetMatrixStacks(mv_stack, p_stack);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   glFrontFace(GL_CCW);
}

void rysuj_tr(const float A[], const float B[], const float C[], float r = 1.0f, float g = 1.0f, float b = 1.0f) {	// potrzebne do oswietlenia
   glBegin(GL_TRIANGLES);
  	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
  	glVertex3fv(A);
  	glVertex3fv(B);
  	glVertex3fv(C);
   glEnd();
}

void wierzcholki(float out_array[], float a0, float a1,float a2) {
   out_array[0] = a0;
   out_array[1] = a1;
   out_array[2] = a2;
}

void rysuj_swiatlo() {
   float A[3], B[3], C[3], wektor_normalny[3];
   // podstawa
	wierzcholki(A, 1.0f, -1.0f, 0.0f);
   wierzcholki(B, -1.0f, -1.0f, 0.0f);
   wierzcholki(C,  1.0f,  1.0f, 0.0f);
   rysuj_tr(A, B, C, 1.0f, 0.5f, 0.2f);
  wierzcholki(A, -1.0f, 1.0f, 0.0f);
   wierzcholki(B, 1.0f, 1.0f, 0.0f);
   wierzcholki(C,-1.0f,-1.0f, 0.0f);
   rysuj_tr(A, B, C, 1.0f, 0.5f, 0.2f);
   //// sciany
   wierzcholki(A, 1.0f,-1.0f, 0.0f);
   wierzcholki(B, 1.0f, 1.0f, 0.0f);
   wierzcholki(C, 0.0f, 0.0f, 3.0f);
   rysuj_tr(A, B, C, 1.0f, 0.5f, 0.2f);

   wierzcholki(A, 1.0f, 1.0f, 0.0f);
   wierzcholki(B,-1.0f, 1.0f, 0.0f);
   wierzcholki(C, 0.0f, 0.0f, 3.0f);
   rysuj_tr(A, B, C, 1.0f, 0.5f, 0.2f);

   wierzcholki(A,-1.0f, -1.0f, 0.0f);
   wierzcholki(B, 0.0f, 0.0f, 3.0f);
   wierzcholki(C, -1.0f, 1.0f, 0.0f);
   rysuj_tr(A, B, C,  1.0f, 0.5f, 0.2f);
 
   wierzcholki(A, 1.0f, -1.0f, 0.0f);
   wierzcholki(B, 0.0f, 0.0f, 3.0f);
   wierzcholki(C, -1.0f, -1.0f, 0.0f);
   rysuj_tr(A, B, C,  1.0f, 0.5f, 0.2f);
}

void render_scene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f / 5.0f;
   location[0] = 10.0f * cos(angle / 2.0f);//ustawienie kamery
   location[1] = 10.0f * sin(angle / 2.0f);
   location[2] = 8.0f;
   // swiatlo w stalym punkcie
   //swiatlo_0.position[0] = 5.0f;
   //swiatlo_0.position[1] = 5.0f;
   // ruchome swiatlo
   swiatlo_0.position[0] = 5.0f * cos(angle);  // swiatlo odwrotnie do obrotu kamery
   swiatlo_0.position[1] = 5.0f * sin(angle);
   swiatlo_0.position[2] = 5.0f;
   LookAt(cameraFrame, location, target, up_dir);
   cameraFrame.GetCameraMatrix(camera_matrix);
   p_stack.LoadMatrix(viewFrustum.GetProjectionMatrix());
   mv_stack.LoadMatrix(camera_matrix);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
 
   glUseProgram(ShaderColor);
   mv_stack.PushMatrix();
   mv_stack.Translate(swiatlo_0.position[0], swiatlo_0.position[1], swiatlo_0.position[2]);
   mv_stack.Scale(0.25f, 0.25f, 0.25f);
   glUniformMatrix4fv(MVPMatrixLocation_ShaderColor, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   rysuj_swiatlo();
   mv_stack.PopMatrix();
 
   glUseProgram(shader_swiatlo);
   glUniformMatrix3fv(NormalMatrix_location, 1, GL_FALSE, geometry_pipeline.GetNormalMatrix());
   glUniformMatrix4fv(vMatrix_location, 1, GL_FALSE, camera_matrix);
   glUniform3fv(intensity_ambient_component_location, 1, intensity_ambient_component);
   glUniform3fv(swiatlo_0_position_location, 1, swiatlo_0.position);
   glUniform3fv(swiatlo_0_intensity_diffuse_location, 1, swiatlo_0.intensity_diffuse);
   glUniform3fv(swiatlo_0_intensity_specular_location, 1, swiatlo_0.intensity_specular);
   glUniform3fv(swiatlo_0_attenuation_location, 1, swiatlo_0.attenuation);
   glUniform1f(material_0_r_ambient_location, material_0.r_ambient);
   glUniform1f(material_0_r_diffuse_location, material_0.r_diffuse);
   glUniform1f(material_0_r_spectacular_location, material_0.r_spectacular);
   glUniform1f(material_0_alpha_location, material_0.alpha);
 
   mv_stack.PushMatrix();
   mv_stack.Translate(0.0f, 0.0f, 0.0f);
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
   glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
   mv_stack.PopMatrix();

   glUseProgram(0);
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("cwiczenie 5");
   glutReshapeFunc(change_size);
   glutDisplayFunc(render_scene);
   GLenum err = glewInit();
   if(GLEW_OK != err) {
  	return 1;
   }
   SetupRC();
   glutMainLoop();
   return 0;
}