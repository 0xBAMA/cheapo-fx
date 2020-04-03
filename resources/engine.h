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


// Shader Compilation
#include "shaders/Shader.h"


#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp" //general vector types
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side


//this is based on the project from this summer - here implemented with polygons, and rendered using perspective projection

  //thoughts
    //little propeller, connected out the back by the crankshaft - maybe add a transmission?

class engine
{
public:

  //the engine consists of a number of parts -

    //crankshaft - connects to the driveshaft to take power from the engine
    //conrods    - connects the crankshaft to the pistons - bottom end located by crank, top located by piston
    //pistons    - the bottom of the combustion chamber - doing piston rings is way easier with polygons
    //valves     - this is new, I want to simplify the representation of the cycle
    //cams       - these are also new, and their rotation is half the speed of the crankshaft
    //intake     - a way to represent air and fuel coming into the engine - scoop out the top?
    //exhaust    - a way to represent the exhaust gasses coming out of the engine - may include piping



  // I want to do coloration of the gas, based on this - https://iquilezles.org/www/articles/palettes/palettes.htm
  //  probably going to represent it as disks (disqes, short for disquettes), past the valve,

  //calls all the add functions, so there is geometry to draw all of the engine components
  void init(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);


  //calls glDrawArrays(GL_TRIANGLES, ____start, num_pts_____); for each of the things, for each cylinder
    //so there's a fair few, num_cylinders many for pistons, con rods, and the sets of valves, then one crank, and 4 cams
  void draw();

private:
  int crank_start, num_pts_crank;
  int conrod_start, num_pts_conrod;
  int piston_start, num_pts_piston;
  int valves_start, num_pts_valves;
  int cams_start, num_pts_cams;
  int exhaust_start, num_pts_exhaust;

  //call these during init - set the integer values to keep track of the indices, same way as in ship.h
  void add_crank(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
  void add_conrod(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
  void add_piston(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
  void add_valves(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
  void add_cams(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
  void add_exhaust(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors);
};
