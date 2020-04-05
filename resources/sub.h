//******************************************************************************
//  Program: SpAce
//
//  Author: Jon Baker
//  Email: jb239812@ohio.edu
//
//  Description: This is the continuation of the Vertexture experiment to use
//    heightmaps to displace geometry. Here, instead of one large quad being used
//    there's 6 separate panels, which are defined similarly (subdivided quads).
//
//  Date: 13 October 2019
//******************************************************************************


#include <vector>
#include <iostream>
using std::cout;
using std::endl;

#include <random>
#include "includes.h"



#define POINT_SPRITE_PATH "resources/textures/height/sphere_small.png"

#define WATER_HEIGHT_TEXTURE "resources/textures/height/water_height.png"
#define WATER_NORMAL_TEXTURE "resources/textures/normal/water_normal.png"
#define WATER_COLOR_TEXTURE "resources/textures/water_color.png"

//**********************************************

//************************************************


// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

//SDL includes - windowing, gl context, system info
#include <SDL.h>
//allows you to run OpenGL inside of SDL2
#include <SDL_opengl.h>


// Shader Compilation
#include "shaders/Shader.h"

//**********************************************

#include "../resources/LodePNG/lodepng.h"
// Good, simple png library


#include "../resources/perlin.h"
//perlin noise generation

//**********************************************

// #define GLM_MESSAGES
#define GLM_SWIZZLE
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "glm/glm.hpp" //general vector types
// #include "glm/gtc/swizzle.hpp"    //swizzling
#include "glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "glm/gtc/type_ptr.hpp" //to send matricies gpu-side
#include "glm/gtx/transform.hpp"


//******************************************************************************
//  Function: planetest
//
//  NOTE: This comes up with some frequency. I'll be carrying this along for a while.
//
//  Purpose:
//      Return true if the test point is below the plane. Return false if the
//      test point is below the plane. Above and below are determined with
//      respect to the normal specified by plane_norm. This is used to confirm
//      that computed normals are actually pointing outwards, by testing a
//      point that is known to be inside the shape against the computed normal.
//      If the point is below the plane specified by plane_norm and plane_point,
//      we know that that normal will be valid. Otherwise, it needs to be
//      inverted.
//
//  Parameters:
//      plane_point - the point from which the normal will originate
//      plane_norm - what direction is 'up'?
//      test_point - you want to know if this is above or below the plane
//
//  Preconditions:
//      plane_norm must be given as a nonzero vector
//
//  Postconditions:
//      true or false is returned to tell the user the restult of their query
//      TRUE IF YOUR TEST POINT IS BELOW THE PLANE
//
//******************************************************************************

bool planetest(glm::vec3 plane_point, glm::vec3 plane_norm, glm::vec3 test_point)
{
  double result, a, b, c, x, x1, y, y1, z, z1;

  a  =  plane_norm.x;   b  =  plane_norm.y;  c  =  plane_norm.z;
  x  =  test_point.x;   y  =  test_point.y;  z  =  test_point.z;
  x1 = plane_point.x;   y1 = plane_point.y;  z1 = plane_point.z;

  //equation of a plane is:
    // a (x-x1) + b (y-y1) + c (z-z1) = 0;

  result = a * (x-x1) + b * (y-y1) + c * (z-z1);

  return (result < 0) ? true:false;
}


//function: capsule sdf
float capsdf(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r)
{
  glm::vec3 ab = b-a;
  glm::vec3 ap = p-a;

  float t = glm::dot(ab,ap)/glm::dot(ab,ab);

  t = glm::clamp(t, 0.0f, 1.0f);

  glm::vec3 c = a + (t * ab);

  return glm::length(p-c) - r;
}



//******************************************************************************
//  Class: Sub
//
//  Purpose:  To represent the ship on the GPU, and everything that goes along
//        with displaying this ship to the user.
//
//  Functions:
//
//    Constructor:
//        Takes no arguments, calls generate_points() to create geometry. Then
//        buffers all this data to the GPU memory. Textures are handled by
//        the class, and each panel has its own display function. This allows
//        the 6 panels to be drawn in depth-order (back to front)
//
//    Setters:
//        Used to update the values of the uniform variables.
//
//    Generate Points:
//        Creates a square for each panel, subdivides the faces several times,
//        and creates triangles to span the shape. This data is used to populate
//        the vectors containing point data.
//
//    Display functions
//        The top level display funciton orders the panels by depth, then draws
//        them in order, from farthest to nearest. It calls the functions specific
//        to each panel, for each panel.
//
//        For each panel, it makes sure the correct shader is being used, that
//        the correct buffers are bound, that the vertex attributes are set up,
//        and that all the latest values of the uniform variables are sent to the
//        GPU. In addition to this, make sure that all the textures are bound the
//        correct texture units.
//******************************************************************************


class Sub{

public:
  Sub();

  void display();

  // void display_panel(int num);  //display the appropriate side, 1-6 - holdover from SpAce


  void adjust_roll_rate(float adj)  {roll_rate += adj;}
  void adjust_pitch_rate(float adj) {pitch_rate += adj;}
  void adjust_yaw_rate(float adj)   {yaw_rate += adj;}

  // void adjust_roll(int degrees);   //upcoming, allows for manual control over orientation of the ship - when used, zero out the rate value
  // void adjust_pitch(int degrees);
  // void adjust_yaw(int degrees);

  void update_rotation();

  void set_proj(glm::mat4 proj);
  void set_view(glm::mat4 view);
  void set_scale(float scale);
  void set_time(int tin)          {t = tin; glUniform1i(t_loc,t);}

  GLuint get_draw_shader() {return panel_shader;}



private:


  void generate_points();
  void subd_square(glm::vec3 a, glm::vec2 at, glm::vec3 b, glm::vec2 bt, glm::vec3 c, glm::vec2 ct, glm::vec3 d, glm::vec2 dt, glm::vec3 norm, float clow, float chigh);

  // void load_textures();



  //pitch, yaw, roll

  float roll_rate=0, pitch_rate=0, yaw_rate=0;




//BUFFER, VAO
  GLuint vao;
  GLuint buffer;

// //TEXTURES - load them all in the init, then bind the appropriate ones in the associated display functions
//   GLuint panel_height[8];
//   GLuint panel_color[8];
//   GLuint panel_normal[8];



  GLuint point_sprite_tex;



//SHADERS
  GLuint panel_shader;
  GLuint axes_shader;

//UNIFORMS
  GLuint yawpitchroll_loc;
  glm::vec3 yawpitchroll;

  std::deque<glm::vec3> yawpitchroll_history;



  GLuint proj_loc;
  glm::mat4 proj;

  GLuint view_loc;
  glm::mat4 view;


  GLuint eye_position_loc, light_position_loc;
  glm::vec3 eye_position, light_position, original_light_position;


  GLuint scale_loc;
  GLfloat scale;

  GLuint t_loc;
  int t;




//The vertex data
  std::vector<glm::vec3> points;    //add the 1.0 w value in the shader
  std::vector<glm::vec2> texcoords; //texture coordinates
  std::vector<glm::vec3> normals;   //used for displacement along the normals
  std::vector<glm::vec4> colors;    //support alpha

//vertex attribs
  GLuint points_attrib;
  GLuint texcoords_attrib;
  GLuint normals_attrib;
  GLuint colors_attrib;


  int hull_start, hull_num; //start of hull geometry, number of verticies in the hull geometry

  typedef struct triangle_t{
    glm::vec3 points[3];
    glm::vec2 texcoords[3];
    glm::vec4 colors[3];
    glm::vec3 normals[3];

    bool done;
  } triangle;

  std::vector<triangle> triangles;

};


// //******************************************************************************

Sub::Sub()
{

    //initialize all the vectors
    points.clear();
    texcoords.clear();
    normals.clear();
    colors.clear();

    //fill those vectors with geometry
    generate_points();


  //SETTING UP GPU STUFF


    //SHADERS (COMPILE, USE)

    cout << " compiling ship shaders" << endl;
    Shader s("resources/shaders/hull_vert.glsl", "resources/shaders/hull_frag.glsl");

    panel_shader = s.Program;


    //VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //BUFFER, SEND DATA
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glUseProgram(panel_shader);


  //POPULATE THE ARRAYS

    int num_bytes_points = sizeof(glm::vec3) * points.size();
    int num_bytes_texcoords = sizeof(glm::vec2) * texcoords.size();
    int num_bytes_normals = sizeof(glm::vec3) * normals.size();
    int num_bytes_colors = sizeof(glm::vec4) * colors.size();

    int num_bytes = num_bytes_points + num_bytes_texcoords + num_bytes_normals + num_bytes_colors;

    glBufferData(GL_ARRAY_BUFFER, num_bytes, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_texcoords, &texcoords[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_texcoords, num_bytes_normals, &normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_texcoords + num_bytes_normals, num_bytes_colors, &colors[0]);

    //VERTEX ATTRIBS
      //todo - https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
          //  - http://docs.gl/gl4/glVertexAttribPointer

      //these are the per-vertex attributes (point location, texture coordinate, normal vector, and, at least initially, a color value)

    points_attrib = glGetAttribLocation(panel_shader, "vPosition");
    texcoords_attrib = glGetAttribLocation(panel_shader, "vTexCoord");
    normals_attrib = glGetAttribLocation(panel_shader, "vNormal");
    colors_attrib = glGetAttribLocation(panel_shader, "vColor");

    glEnableVertexAttribArray(points_attrib);
    glEnableVertexAttribArray(texcoords_attrib);
    glEnableVertexAttribArray(normals_attrib);
    glEnableVertexAttribArray(colors_attrib);

    cout << "setting up points attrib" << endl;
    glVertexAttribPointer(points_attrib, 3, GL_FLOAT, false, 0, ((GLvoid*) (0)));
    cout << "setting up texcoords attrib" << endl;
    glVertexAttribPointer(texcoords_attrib, 2, GL_FLOAT, false, 0, ((GLvoid*) (num_bytes_points)));
    cout << "setting up normals attrib" << endl;
    glVertexAttribPointer(normals_attrib, 3, GL_FLOAT, false, 0, ((GLvoid*) (num_bytes_points + num_bytes_texcoords)));
    cout << "setting up colors attrib" << endl;
    glVertexAttribPointer(colors_attrib, 4, GL_FLOAT, false, 0, ((GLvoid*) (num_bytes_points + num_bytes_texcoords + num_bytes_normals)));




    //UNIFORMS
    yawpitchroll_loc = glGetUniformLocation(panel_shader, "yawpitchroll");
    yawpitchroll = glm::vec3(0,0,0);

    yawpitchroll_history.resize(BOX_NUM_REPEATS*30);

    glUniform3fv(yawpitchroll_loc, 1, glm::value_ptr(yawpitchroll));

    proj_loc = glGetUniformLocation(panel_shader, "proj");
    glUniformMatrix4fv(proj_loc, 1, GL_TRUE, glm::value_ptr(proj));
    // glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));

    view_loc = glGetUniformLocation(panel_shader, "view");
    glUniformMatrix4fv(view_loc, 1, GL_TRUE, glm::value_ptr(view));
    // glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

    scale = 1.0;
    scale_loc = glGetUniformLocation(panel_shader, "scale");
    glUniform1fv(scale_loc, 1, &scale);

    t = 0;
    t_loc = glGetUniformLocation(panel_shader, "t");
    glUniform1i(t_loc,t);



    eye_position_loc = glGetUniformLocation(panel_shader, "eye_position");
    eye_position = glm::vec3(-1.3f, 1.0f, -1.7f);
    glUniform3fv(eye_position_loc, 1, glm::value_ptr(eye_position));


    light_position_loc = glGetUniformLocation(panel_shader, "light_position");
    light_position = original_light_position = glm::vec3(0,0,0);
    glUniform3fv(light_position_loc, 1, glm::value_ptr(light_position));




    //And now, the generation and loading of a great many textures
    // glEnable(GL_TEXTURE_2D);

    // glGenTextures(8, &panel_height[0]);
    // glGenTextures(8, &panel_color[0]);
    // glGenTextures(8, &panel_normal[0]);

    // load_textures();















    glPointSize(6.0f);
}

// //******************************************************************************


  //****************************************************************************
  //  Function: Sub::generate_points()
  //
  //  Purpose:
  //    This function produces all the data for representing this object.
  //****************************************************************************

void Sub::generate_points()
{
//GENERATING GEOMETRY

  hull_start = points.size();

  triangles.clear();

  glm::vec3 a,b,c,d;
  glm::vec2 at,bt,ct,dt;

  glm::vec3 norm = glm::vec3(0,0,1);

  float scale = 0.5f;
  float xfactor = 1.0f;

  a = glm::vec3(xfactor*scale,scale,scale);
  b = glm::vec3(-xfactor*scale,scale,scale);
  c = glm::vec3(xfactor*scale,-scale,scale);
  d = glm::vec3(-xfactor*scale,-scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(-xfactor*scale,-scale,-scale);
  b = glm::vec3(-xfactor*scale,scale,-scale);
  c = glm::vec3(xfactor*scale,-scale,-scale);
  d = glm::vec3(xfactor*scale,scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,0,-1);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);








  a = glm::vec3(-xfactor*scale,scale,-scale);
  b = glm::vec3(-xfactor*scale,scale,scale);
  c = glm::vec3(xfactor*scale,scale,-scale);
  d = glm::vec3(xfactor*scale,scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,1,0);


  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(xfactor*scale,-scale,scale);
  b = glm::vec3(-xfactor*scale,-scale,scale);
  c = glm::vec3(xfactor*scale,-scale,-scale);
  d = glm::vec3(-xfactor*scale,-scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(0,-1,0);


  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);










  a = glm::vec3(xfactor*scale,scale,scale);
  b = glm::vec3(xfactor*scale,-scale,scale);
  c = glm::vec3(xfactor*scale,scale,-scale);
  d = glm::vec3(xfactor*scale,-scale,-scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(1,0,0);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);

  a = glm::vec3(-xfactor*scale,-scale,-scale);
  b = glm::vec3(-xfactor*scale,-scale,scale);
  c = glm::vec3(-xfactor*scale,scale,-scale);
  d = glm::vec3(-xfactor*scale,scale,scale);

  at = glm::vec2(0,0);
  bt = glm::vec2(1,0);
  ct = glm::vec2(0,1);
  dt = glm::vec2(1,1);

  norm = glm::vec3(-1,0,0);

  subd_square(a,at,b,bt,c,ct,d,dt,norm,0.1,0.1);



  // glm::vec3 acap,bcap;
  // float r;
  //
  // // acap = glm::vec3(0.5, 0, 0);
  // // bcap = glm::vec3(-0.5, 0,0);
  // // r = 0.1618;
  //
  // acap = glm::vec3( 0.3,0,0);
  // bcap = glm::vec3(-0.3,0,0);
  // r = 0.5;
  //
  //
  // int passes = 0;
  // int count = 0;
  //
  //
  // bool some_not_done = true;
  //
  // while(some_not_done)
  // {
  //   some_not_done = false;
  //   count = 0;
  //   for(auto &x : triangles)
  //   {
  //     if(!x.done)
  //     {
  //       //shrink each point in, if it's still outside the shape
  //       float d0,d1,d2;
  //
  //       d0 = capsdf(x.points[0],acap,bcap,r);
  //       d1 = capsdf(x.points[1],acap,bcap,r);
  //       d2 = capsdf(x.points[2],acap,bcap,r);
  //
  //
  //       if(abs(d0)>0.0001)
  //       {
  //         if(d0 > 0)
  //           x.points[0] *= 0.99999;
  //
  //         if(d0 < 0)
  //           x.points[0] /= 0.5;
  //       }
  //
  //
  //
  //       if(abs(d1)>0.0001)
  //       {
  //         if(d1 > 0)
  //           x.points[1] *= 0.99999;
  //
  //         if(d1 < 0)
  //           x.points[1] /= 0.5;
  //       }
  //
  //
  //
  //       if(abs(d2)>0.0001)
  //       {
  //         if(d2 > 0)
  //           x.points[2] *= 0.99999;
  //
  //         if(d2 < 0)
  //           x.points[2] /= 0.5;
  //       }
  //
  //       glm::vec3 mid = (x.points[0] + x.points[1] + x.points[2]) / 3.0f;
  //
  //       if((abs(capsdf(x.points[0], acap, bcap, r)) < 0.0001) && (abs(capsdf(x.points[1], acap, bcap, r)) < 0.0001) && (abs(capsdf(x.points[2], acap, bcap, r)) < 0.0001))
  //       {
  //         x.done = true;
  //
  //         // glm::vec3 norm = glm::normalize(glm::cross(x.points[0]-x.points[1], x.points[0]-x.points[2]));
  //         // if(!planetest(mid, norm, glm::vec3(0,0,0)))
  //         //    norm = norm * -1.0f;
  //
  //         float d = capsdf(mid, acap, bcap, r);
  //         glm::vec2 e = glm::vec2(0.01, 0);
  //
  //         // glm::vec3 norm = glm::normalize(glm::vec3(d,d,d) -
  //         //   glm::vec3(capsdf(glm::vec3(mid.x-e.x,mid.y-e.y,mid.z-e.y), acap, bcap, r),
  //         //             capsdf(glm::vec3(mid.x-e.y,mid.y-e.x,mid.z-e.y), acap, bcap, r),
  //         //             capsdf(glm::vec3(mid.x-e.y,mid.y-e.y,mid.z-e.x), acap, bcap, r)));
  //         // x.normals[0] = norm;
  //         // x.normals[1] = norm;
  //         // x.normals[2] = norm;
  //
  //         x.normals[0] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[0].x-e.x,x.points[0].y-e.y,x.points[0].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[0].x-e.y,x.points[0].y-e.x,x.points[0].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[0].x-e.y,x.points[0].y-e.y,x.points[0].z-e.x), acap, bcap, r)));
  //
  //         x.normals[1] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[1].x-e.x,x.points[1].y-e.y,x.points[1].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[1].x-e.y,x.points[1].y-e.x,x.points[1].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[1].x-e.y,x.points[1].y-e.y,x.points[1].z-e.x), acap, bcap, r)));
  //
  //         x.normals[2] = glm::normalize(glm::vec3(d,d,d) -
  //           glm::vec3(capsdf(glm::vec3(x.points[2].x-e.x,x.points[2].y-e.y,x.points[2].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[2].x-e.y,x.points[2].y-e.x,x.points[2].z-e.y), acap, bcap, r),
  //                     capsdf(glm::vec3(x.points[2].x-e.y,x.points[2].y-e.y,x.points[2].z-e.x), acap, bcap, r)));
  //
  //
  //
  //
  //
  //       }
  //     }
  //
  //     if(!x.done)
  //     {
  //       some_not_done = true;
  //       count++;
  //     }
  //   }
  //   passes++;
  //
  //   cout << "\rpass number " << passes  << " touched " << count << "                      ";
  // }



  float xoffset, yoffset, zoffset, radius;
  xoffset = 0.3f;
  yoffset = 0.3f;
  zoffset = 0.3f;
  radius = 0.1f;

  for(auto x : triangles)
  {

    // points.push_back(0.3f*glm::normalize(x.points[0]));
    // points.push_back(0.3f*glm::normalize(x.points[1]));
    // points.push_back(0.3f*glm::normalize(x.points[2]));

    glm::vec3 midp = (radius*glm::normalize(x.points[0]) +
                        radius*glm::normalize(x.points[1]) +
                           radius*glm::normalize(x.points[2]))/3.0f;



    if(midp.x > 0 && midp.y  > 0 )
    {
      if(midp.z > 0)
      {
        colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        colors.push_back(glm::vec4(0.5,0.0,0.5,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,yoffset,zoffset));
      }
      else
      {
        colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        colors.push_back(glm::vec4(0.15,0.0,0.15,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,yoffset,-zoffset));
      }
    }
    else if ( midp.x > 0 && midp.y  < 0 )
    {
      if(midp.z > 0)
      {
        colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        colors.push_back(glm::vec4(0.1,0.6,0.1,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,-yoffset,zoffset));
      }
      else
      {
        colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        colors.push_back(glm::vec4(0.03,0.18,0.03,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(xoffset,-yoffset,-zoffset));
      }
    }
    else if ( midp.x < 0 && midp.y  > 0 )
    {
      if(midp.z > 0)
      {
        colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        colors.push_back(glm::vec4(0.1,0.1,0.3,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,yoffset,zoffset));
      }
      else
      {
        colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        colors.push_back(glm::vec4(0.03,0.03,0.09,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,yoffset,-zoffset));
      }
    }
    else if ( midp.x < 0 && midp.y  < 0 )
    {
      if(midp.z > 0)
      {
        colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        colors.push_back(glm::vec4(0.5,0.1,0.0,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,-yoffset,zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,-yoffset,zoffset));
      }
      else
      {
        colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        colors.push_back(glm::vec4(0.15,0.03,0.0,1.0));
        points.push_back(radius*glm::normalize(x.points[0]) + glm::vec3(-xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[1]) + glm::vec3(-xoffset,-yoffset,-zoffset));
        points.push_back(radius*glm::normalize(x.points[2]) + glm::vec3(-xoffset,-yoffset,-zoffset));
      }
    }
    else
    {
      points.push_back(radius*glm::normalize(x.points[0]));
      points.push_back(radius*glm::normalize(x.points[1]));
      points.push_back(radius*glm::normalize(x.points[2]));
      colors.push_back(x.colors[0]);
      colors.push_back(x.colors[1]);
      colors.push_back(x.colors[2]);
    }



    texcoords.push_back(x.texcoords[0]);
    texcoords.push_back(x.texcoords[1]);
    texcoords.push_back(x.texcoords[2]);



    // normals.push_back(x.normals[0]);
    // normals.push_back(x.normals[1]);
    // normals.push_back(x.normals[2]);

    normals.push_back(glm::normalize(x.points[0]));
    normals.push_back(glm::normalize(x.points[1]));
    normals.push_back(glm::normalize(x.points[2]));


  }

  //cylinders +/- x
  for(float rot = 0; rot < 6.29; rot += 0.01)
  {
    float xcur = radius * cos(rot);
    float xprev = radius * cos(rot-0.01);

    float ycur = radius * sin(rot);
    float yprev = radius * sin(rot-0.01);

    norm = glm::normalize(glm::vec3(xcur, ycur, 0));


    if(xcur > 0)
    {
      if(ycur > 0)
      {
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur+yoffset,zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yprev-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,ycur-yoffset,zoffset));
      }

    }
    else
    {
      if(ycur > 0)
      {
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev+yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur+yoffset,zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yprev-yoffset,-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,ycur-yoffset,zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    // texcoords.push_back();   //not really defined
  }





  //cylinders +/- y
  for(float rot = 0; rot < 6.29; rot += 0.01)
  {
    float xcur = radius * cos(rot);
    float xprev = radius * cos(rot-0.01);

    float zcur = radius * sin(rot);
    float zprev = radius * sin(rot-0.01);

    norm = glm::normalize(glm::vec3(xcur, 0, zcur));


    if(xcur > 0)
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xcur+xoffset,-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev+zoffset));

        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur+zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev-zoffset));

        points.push_back(glm::vec3(xprev+xoffset,-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xprev+xoffset,yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xcur+xoffset,yoffset,zcur-zoffset));
      }

    }
    else
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xcur-xoffset,-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev+zoffset));

        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur+zoffset));
      }
      else
      {
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev-zoffset));

        points.push_back(glm::vec3(xprev-xoffset,-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xprev-xoffset,yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xcur-xoffset,yoffset,zcur-zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    // texcoords.push_back();   //not really defined
  }






  //cylinders +/- z
  for(float rot = 0; rot < 6.29; rot += 0.01)
  {
    float ycur = radius * cos(rot);
    float yprev = radius * cos(rot-0.01);

    float zcur = radius * sin(rot);
    float zprev = radius * sin(rot-0.01);

    norm = glm::normalize(glm::vec3(0, ycur, zcur));


    if(ycur > 0)
    {
      if(zcur > 0)
      {

        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,ycur+yoffset,zcur+zoffset));

        points.push_back(glm::vec3(xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur+zoffset));

      }
      else
      {

        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,ycur+yoffset,zcur-zoffset));

        points.push_back(glm::vec3(xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev+yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xoffset,ycur+yoffset,zcur-zoffset));

      }

    }
    else
    {
      if(zcur > 0)
      {
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,ycur-yoffset,zcur+zoffset));

        points.push_back(glm::vec3(xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev+zoffset));
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur+zoffset));

      }
      else
      {

        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,ycur-yoffset,zcur-zoffset));

        points.push_back(glm::vec3(xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(-xoffset,yprev-yoffset,zprev-zoffset));
        points.push_back(glm::vec3(xoffset,ycur-yoffset,zcur-zoffset));
      }
    }

    for(int i = 0; i < 6; i++)
      normals.push_back(norm);

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));
    colors.push_back(glm::vec4(0,0,0,0));

    // texcoords.push_back();   //not really defined
  }





  hull_num = points.size() - hull_start;
  cout << "hull starts at " << hull_start << " and is " << hull_num << " verticies" << endl;

}

// //******************************************************************************

void Sub::subd_square(glm::vec3 a, glm::vec2 at, glm::vec3 b, glm::vec2 bt, glm::vec3 c, glm::vec2 ct, glm::vec3 d, glm::vec2 dt, glm::vec3 norm, float clow, float chigh)
{

  float thresh = 0.01;
  if(glm::distance(a, b) < thresh || glm::distance(a,c) < thresh || glm::distance(a,d) < thresh)
  {//add points


    triangle temp1, temp2;

    //initially not done, want to still move these around since they're just at the initial positions.
    temp1.done = false;
    temp2.done = false;

    temp1.points[0] = a;
    temp1.points[1] = b;
    temp1.points[2] = c;

    temp2.points[0] = c;
    temp2.points[1] = b;
    temp2.points[2] = d;




    //random number generation
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(clow, chigh);

    //colors
    temp1.colors[0] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp1.colors[1] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp1.colors[2] = glm::vec4(dist(mt), dist(mt), 0, 1);

    temp2.colors[0] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp2.colors[1] = glm::vec4(dist(mt), dist(mt), 0, 1);
    temp2.colors[2] = glm::vec4(dist(mt), dist(mt), 0, 1);


    //texcoords

    temp1.texcoords[0] = at;
    temp1.texcoords[1] = bt;
    temp1.texcoords[2] = ct;

    temp2.texcoords[0] = bt;
    temp2.texcoords[1] = ct;
    temp2.texcoords[2] = dt;


    //this scheme uses one point at the center - it was for the spaceship but might be useful.
    // glm::vec2 tc = (at + bt + ct + dt)/4.0f;
    //
    // temp1.texcoords[0] = tc;
    // temp1.texcoords[1] = tc;
    // temp1.texcoords[2] = tc;
    //
    // temp2.texcoords[0] = tc;
    // temp2.texcoords[1] = tc;
    // temp2.texcoords[2] = tc;


    //normals
    temp1.normals[0] = norm;
    temp1.normals[1] = norm;
    temp1.normals[2] = norm;

    temp2.normals[0] = norm;
    temp2.normals[1] = norm;
    temp2.normals[2] = norm;

    triangles.push_back(temp1);
    triangles.push_back(temp2);




  }
  else
  { //recurse
    glm::vec3 center = (a + b + c + d) / 4.0f;    //center of the square
    glm::vec2 centert = (at + bt + ct + dt) / 4.0f;

    glm::vec3 bdmidp = (b + d) / 2.0f;            //midpoint between b and d
    glm::vec2 bdmidpt = (bt + dt) / 2.0f;

    glm::vec3 abmidp = (a + b) / 2.0f;            //midpoint between a and b
    glm::vec2 abmidpt = (at + bt) / 2.0f;

    glm::vec3 cdmidp = (c + d) / 2.0f;            //midpoint between c and d
    glm::vec2 cdmidpt = (ct + dt) / 2.0f;

    glm::vec3 acmidp = (a + c) / 2.0f;            //midpoint between a and c
    glm::vec2 acmidpt = (at + ct) / 2.0f;

    subd_square(abmidp, abmidpt, b, bt, center, centert, bdmidp, bdmidpt, norm, clow, chigh);
    subd_square(a, at, abmidp, abmidpt, acmidp, acmidpt, center, centert, norm, clow, chigh);
    subd_square(center, centert, bdmidp, bdmidpt, cdmidp, cdmidpt, d, dt, norm, clow, chigh);
    subd_square(acmidp, acmidpt, center, centert, c, ct, cdmidp, cdmidpt, norm, clow, chigh);
  }
}

// //******************************************************************************

void Sub::display()
{

  glBindVertexArray(vao);
  glUseProgram(panel_shader);

  light_position = original_light_position + glm::vec3(2*cos(0.005*t),-2,2*sin(0.01*t));
  glUniform3fv(light_position_loc, 1, glm::value_ptr(light_position));


  //maintaining history of yawptichroll
  //push back - put in the new value
  yawpitchroll_history.push_back(yawpitchroll);
  //pop front - take out the oldest value
  yawpitchroll_history.pop_front();



  float disp_scale = scale;
  for(int i = 0; i < BOX_NUM_REPEATS; i++)
  {

    glUniform1fv(scale_loc, 1, &disp_scale);
    glDrawArrays(GL_TRIANGLES, hull_start, hull_num);
    glUniform3fv(yawpitchroll_loc, 1, glm::value_ptr(yawpitchroll_history[30*i]));

    disp_scale *=0.618;
  }

  cout <<yawpitchroll[0]<<" "<<yawpitchroll[1]<<" "<<yawpitchroll[2]<<endl;




  //push and pop to keep the history

  // yawpitchroll

}


// //******************************************************************************

void Sub::update_rotation()
{
  yawpitchroll[0] += (3.14/180.0) * yaw_rate;
  yawpitchroll[1] += (3.14/180.0) * pitch_rate;
  yawpitchroll[2] += (3.14/180.0) * roll_rate;

  if(abs(yawpitchroll[0]) > 2*3.14)
    if(yawpitchroll[0] > 0) //positive overflow
      yawpitchroll[0] -= 2*3.14;
    else                    //negative "underflow"
      yawpitchroll[0] += 2*3.14;

  if(abs(yawpitchroll[1]) > 2*3.14)
    if(yawpitchroll[1] > 0)
      yawpitchroll[1] -= 2*3.14;
    else
      yawpitchroll[1] += 2*3.14;

  if(abs(yawpitchroll[2]) > 2*3.14)
    if(yawpitchroll[2] > 0)
      yawpitchroll[2] -= 2*3.14;
    else
      yawpitchroll[2] += 2*3.14;



  // view = view * glm::rotate(1.0f/200.0f,glm::vec3(view[0][0], view[0][1], view[0][2]));
  // glUniformMatrix4fv(view_loc, 1, GL_TRUE, glm::value_ptr(view));



}

// //******************************************************************************

void Sub::set_proj(glm::mat4 in)
{
  proj = in;
  glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
  // glUniformMatrix4fv(proj_loc, 1, GL_TRUE, glm::value_ptr(proj));
}

// //******************************************************************************

void Sub::set_view(glm::mat4 in)
{
  view = in;
  glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
}

// //******************************************************************************


void Sub::set_scale(float in)
{
  scale = in;
  glUniform1fv(scale_loc, 1, &scale);
}

// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
// //******************************************************************************
