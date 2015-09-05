#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>

void get_version() {
    fprintf(stderr, "gl_version: %s, glsl_version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
}
