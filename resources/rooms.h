#include <vector>
#include <iostream>
using std::cout;
using std::endl;

#include <random>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


// GLUT
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>


// Shader Compilation, if neccesary
#include "shaders/Shader.h"


#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp" //general vector types
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side


class roommodel
{
public:

  void init(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);

  void draw();

private:

};


class accoutrement
{
public:

  void init(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);

  void draw();

private:

};
