#include <GL/gl.h>
#include <math.h>

void draw_circle() {
    int i = 0;
    int n = 20;
    GLfloat R=0.5f;
    GLfloat Pi = 3.1415926536f;

    glClearColor(0.2, 0.4, 0.9, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POLYGON);
    for(i = 0; i < n; i ++)
        glVertex2f(R*cos(2*Pi/n*i), R*sin(2*Pi/n*i));
    glEnd();
    glFlush();
}
