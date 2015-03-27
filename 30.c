// A simple introductory program; its main window contains a static picture
// of a triangle, whose three vertices are red, green and blue.  The program
// illustrates viewing with default viewing parameters only.

#include <stdio.h>
#include <GL/glut.h>

// Clears the current window and draws a triangle.
void display() {

  // Set every pixel in the frame buffer to the current clear color.
  glClear(GL_COLOR_BUFFER_BIT);

  // Drawing is done by specifying a sequence of vertices.  The way these
  // vertices are connected (or not connected) depends on the argument to
  // glBegin.  GL_POLYGON constructs a filled polygon.
  glBegin(GL_POLYGON);
    glColor3f(0, 0, 0); glVertex3f(-1, -1, 0);
    glColor3f(1, 1, 1); glVertex3f(1, -1, 0);
    glColor3f(1, 1, 1); glVertex3f(1, 1, 0);
    glColor3f(0, 0, 0); glVertex3f(-1, 1, 0);
  glEnd();

  // Flush drawing command buffer to make drawing happen as soon as possible.
  glFlush();
}

// Initializes GLUT, the display mode, and main window; registers callbacks;
// enters the main event loop.
int main(int argc, char** argv) {

  // Use a single buffered window in RGB mode (as opposed to a double-buffered
  // window or color-index mode).
  glutInit(&argc, argv);
  glutInitDisplayString("red=10 green=10 blue=10 alpha=0");
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

  // Position window at (80,80)-(480,380) and give it a title.
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(4096, 2160);
  glutCreateWindow("mpv - foo");
  //glutFullScreen();

  int redSize, greenSize, blueSize;
  glGetIntegerv(GL_RED_BITS,   &redSize);
  glGetIntegerv(GL_GREEN_BITS, &greenSize);
  glGetIntegerv(GL_BLUE_BITS,  &blueSize);
  printf("RGB size %d:%d:%d\n", redSize, greenSize, blueSize);

  // Tell GLUT that whenever the main window needs to be repainted that it
  // should call the function display().
  glutDisplayFunc(display);

  // Tell GLUT to start reading and processing events.  This function
  // never returns; the program only exits when the user closes the main
  // window or kills the process.
  glutMainLoop();
}

