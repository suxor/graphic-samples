#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>

void get_version() {
    glClearColor(0.2, 0.4, 0.9, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    fprintf(stderr, 
           "gl_verdor: %s\n"
           "gl_renderer: %s\n"
           "gl_version: %s\n"
           "glsl_version: %s\n", 
           //"gl_extensions: \n%s\n",
           glGetString(GL_VENDOR),
           glGetString(GL_RENDERER),
           glGetString(GL_VERSION),
           //glGetString(GL_EXTENSIONS),
           glGetString(GL_SHADING_LANGUAGE_VERSION));
}
