//******************************************************************************
//  Program: HAUNTED HOUSE
//
//  Author: Jon Baker
//  Email: jb239812@ohio.edu
//
//  Description: This file contains GLUT specific code to open a window and
//       allow interaction. It instantiates model classes declared in model.h
//
//      The goal of this program is to experiment with a number of textures
//      being used in the representation of a surface as part of a model.
//
//  Date: 10 October 2019
//******************************************************************************

#include "resources/sub.h"
#include <stdio.h>


Sub * scene;
float scale = 1;
int t = 0;





//DEBUG STUFF

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "    GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}



void init()
{
  cout << "initializing models ...";
  scene = new Sub();
  cout << " done." << endl;


 glm::mat4 view = glm::lookAt(
     glm::vec3(-1.3f, 1.0f, -1.7f),
     glm::vec3(0.0f, 0.0f, 0.0f),
     glm::vec3(0.0f, 1.0f, 0.0f)
 );

  scene->set_view(view);

  glm::mat4 proj = glm::perspective(glm::radians(65.0f), 1366.0f / 768.0f, 1.0f, 100.0f);

  scene->set_proj(proj);

  scene->set_scale(scale);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);

  glClearColor(0.168f, 0.168f, 0.168f, 1.0f);

}

//----------------------------------------------------------------------------

void display()
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // display functions go here
  scene->display();

  // glFlush();
  // glutSwapBuffers();
  // glutPostRedisplay();

}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {

    case 033:
      exit(EXIT_SUCCESS);
      break;


    case 'q':   //YAW CONTROLS
      scene->adjust_yaw_rate(0.1);
      break;
    case 'w':
      scene->adjust_yaw_rate(-0.1);
      break;

    case 'a':   //PITCH CONTROLS
      scene->adjust_pitch_rate(0.1);
      break;
    case 's':
      scene->adjust_pitch_rate(-0.1);
      break;

    case 'z':   //ROLL CONTROLS
      scene->adjust_roll_rate(0.1);
      break;
    case 'x':
      scene->adjust_roll_rate(-0.1);
      break;


            //SCALE ADJUSTMENT
    case 'e':
      scale -= 0.01;
      scene->set_scale(scale);
      break;

    case 'r':
      scale += 0.01;
      scene->set_scale(scale);
      break;


    case 'f':
      // glutFullScreen();
      // glutFullScreenToggle(); //specific to freeglut - otherwise keep a bool, return to windowed with window functions
      break;

  }
  // glutPostRedisplay();
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void timer(int)
{
  t++;
  scene->set_time(t);
  scene->update_rotation();


	// glutPostRedisplay();
	// glutTimerFunc(1000.0/60.0, timer, 0);
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  cout << endl << "Creating OpenGL window ...";

  if(SDL_Init( SDL_INIT_EVERYTHING ) != 0)
  {
      printf("Error: %s\n", SDL_GetError());
  }

  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8);

  // GL 4.5 + GLSL 450
  // const char* glsl_version = "#version 450";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );

  //this is how you query the screen resolution
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode(0, &dm);

  //pulling these out because I'm going to try to span the whole screen with
  //the window, in a way that's flexible on different resolution screens
  int total_screen_width = dm.w;
  int total_screen_height = dm.h;

  SDL_Window * window;
  SDL_GLContext GLcontext;

  window = SDL_CreateWindow( "OpenGL Window", 0, 0, total_screen_width, total_screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS );
  GLcontext = SDL_GL_CreateContext( window );

  SDL_GL_MakeCurrent(window, GLcontext);
  SDL_GL_SetSwapInterval(1); // Enable vsync -- questionable utility



  if (glewInit() != GLEW_OK)
  {
      fprintf(stderr, "Failed to initialize OpenGL loader!\n");
  }

  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  cout << endl << endl << " GL_DEBUG_OUTPUT ENABLED " << endl;


  SDL_GL_SwapWindow( window );

  cout << " done." << endl;


  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  cout << endl << endl << " GL_DEBUG_OUTPUT ENABLED " << endl;



  init();

  //ENTER MAIN LOOP


  bool exit = false;
  while(!exit)
  {

  //take input
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        exit = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
        exit = true;
      if (event.type == SDL_KEYUP  && event.key.keysym.sym == SDLK_ESCAPE)
        exit = true;

      if(event.type == SDL_KEYDOWN)
      {
        //switch based on key press
        switch( event.key.keysym.sym )
        {
          case '=':   //+
            scale *= 0.9;
            glUniform1fv(glGetUniformLocation(scene->get_draw_shader(), "scale"), 1, &scale);
            cout << scale << endl;
            break;

          case '-':   //-
            scale /= 0.9;
            glUniform1fv(glGetUniformLocation(scene->get_draw_shader(), "scale"), 1, &scale);
            cout << scale << endl;
            break;


          case 'g':
            // scene.compute();
            // THIS SHOULD ENSURE COHERENCY - 3/12/20 not sure if this is true? maybe only neccesary to prevent the display function from using the wrong data?
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            break;


            case 'q':   //YAW CONTROLS
              scene->adjust_yaw_rate(0.1);
              break;
            case 'w':
              scene->adjust_yaw_rate(-0.1);
              break;

            case 'a':   //PITCH CONTROLS
              scene->adjust_pitch_rate(0.1);
              break;
            case 's':
              scene->adjust_pitch_rate(-0.1);
              break;

            case 'z':   //ROLL CONTROLS
              scene->adjust_roll_rate(0.1);
              break;
            case 'x':
              scene->adjust_roll_rate(-0.1);
              break;

            case 'f':
              scene->set_yaw_rate(0.0f);
              scene->set_pitch_rate(0.0f);
              scene->set_roll_rate(0.0f);
              break;


                    //SCALE ADJUSTMENT
            case 'e':
              scale -= 0.01;
              scene->set_scale(scale);
              break;

            case 'r':
              scale += 0.01;
              scene->set_scale(scale);
              break;

        }
      }
    }

  //clear and draw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    scene->set_time(SDL_GetTicks()/16);
    scene->update_rotation();


    scene->display();

    SDL_GL_SwapWindow(window);                      //swap the double buffers to display

  }

  return(EXIT_SUCCESS);
}
