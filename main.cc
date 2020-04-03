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


Sub * submodel;
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
  submodel = new Sub();
  cout << " done." << endl;






  // GLfloat left = -1.920f;
  // GLfloat right = 1.920f;
  // GLfloat top =  -1.080f;
  // GLfloat bottom = 1.080f;
  // GLfloat zNear = -1.0f;
  // GLfloat zFar = 1.0f;

  // GLfloat left = -2.0;
  // GLfloat right = 2.0;
  // GLfloat top = 2.0;
  // GLfloat bottom = -2.0;
  // GLfloat zNear = 10.2f;
  // GLfloat zFar = -1.0f;

  // glm::mat4 proj = glm::ortho(left, right, top, bottom, zNear, zFar);


 // detail::tmat4x4< T > 	lookAt (detail::tvec3< T > const &eye, detail::tvec3< T > const &center, detail::tvec3< T > const &up)


 glm::mat4 view = glm::lookAt(
     glm::vec3(-1.3f, 1.0f, -1.7f),
     glm::vec3(0.0f, 0.0f, 0.0f),
     glm::vec3(0.0f, 1.0f, 0.0f)
 );

  submodel->set_view(view);

  glm::mat4 proj = glm::perspective(glm::radians(65.0f), 1366.0f / 768.0f, 1.0f, 100.0f);

  submodel->set_proj(proj);

  submodel->set_scale(scale);

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
  submodel->display();

  // glFlush();
  glutSwapBuffers();
  glutPostRedisplay();

}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {

    case 033:
      exit(EXIT_SUCCESS);
      break;


    case 'q':   //YAW CONTROLS
      submodel->adjust_yaw_rate(0.1);
      break;
    case 'w':
      submodel->adjust_yaw_rate(-0.1);
      break;

    case 'a':   //PITCH CONTROLS
      submodel->adjust_pitch_rate(0.1);
      break;
    case 's':
      submodel->adjust_pitch_rate(-0.1);
      break;

    case 'z':   //ROLL CONTROLS
      submodel->adjust_roll_rate(0.1);
      break;
    case 'x':
      submodel->adjust_roll_rate(-0.1);
      break;


            //SCALE ADJUSTMENT
    case 'e':
      scale -= 0.01;
      submodel->set_scale(scale);
      break;

    case 'r':
      scale += 0.01;
      submodel->set_scale(scale);
      break;


    case 'f':
      // glutFullScreen();
      glutFullScreenToggle(); //specific to freeglut - otherwise keep a bool, return to windowed with window functions
      break;

  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------



void mouse( int button, int state, int x, int y )
{
  if ( state == GLUT_DOWN )
	{
		switch( button )
		{
		    case GLUT_LEFT_BUTTON:    cout << "left" << endl;   break;
		    case GLUT_MIDDLE_BUTTON:  cout << "middle" << endl; break;
		    case GLUT_RIGHT_BUTTON:   cout << "right" << endl;  break;
		}

    if(button == GLUT_LEFT_BUTTON)
    {

      //clear the screen
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      //selection handling code - using input x and y
      y = glutGet( GLUT_WINDOW_HEIGHT ) - y;

      unsigned char pixel[4];
      glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

      cout << "color at click is r:" << (int)pixel[0] << " g:" << (int)pixel[1] << " b:" << (int)pixel[2] << endl;


      //clear the screen
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glutPostRedisplay();

    }
  }
}

//----------------------------------------------------------------------------

void timer(int)
{
  t++;
  submodel->set_time(t);
  submodel->update_rotation();


	glutPostRedisplay();
	glutTimerFunc(1000.0/60.0, timer, 0);
}

//----------------------------------------------------------------------------



void idle( void )
{
	// glutPostRedisplay();
}


//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  glutInitContextVersion( 4, 5 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

  glutInitWindowSize(720,480);
  glutCreateWindow("BOO");

  glewInit();


  //DEBUG
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );

  cout << endl << endl << " GL_DEBUG_OUTPUT ENABLED " << endl;



  init();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc( mouse );
  glutIdleFunc( idle );
  glutTimerFunc(1000.0/60.0, timer, 0);





//ENTER MAIN LOOP
  glutMainLoop();
  return(EXIT_SUCCESS);
}
