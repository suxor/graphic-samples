#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
//#include <GL/gl.h>

typedef void (*DRAW_FUNC)();

static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
static int dblBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};
Display *display;
Window window;

char *testcase = NULL;
int window_width = 800;
int window_height = 600;
Font font_for_text = 0;
char *font_name_pattern = NULL;

void glfInit() 
{
/*    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();*/
}

void glfProcessKeyboard(KeySym keysym)
{
    if (keysym == (KeySym)XK_Escape)
        exit(0);
}

void glfProcessMousePress(int x, int y)
{
}

void glfUpdate()
{
}
void glfReshape(int width, int height)
{
/*    glf_WinWidth = width;
    glf_WinHeight = height;
    glViewPort(0, 0, glf_WinWidth, glf_WinHeight);*/
}

void glfDraw()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush;

    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(0.0f, 0.0f);
    glPushAttrib(GL_LIST_BIT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    static int isFirstCall = 1;
    static GLuint lists;
    if (isFirstCall) {
        lists = glGenLists(128);
    }
#if 0 
    //for (i = 0; i < sizeof(letters)
    glNewList(lists + 'A', GL_COMPILE);
    glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[0]);
    glEndList();
    GLubyte *str = (GLubyte*)"A";
#else
    glXUseXFont(font_for_text, 0, 128, lists);
    GLubyte *str = (GLubyte*)"Hello, world.";
#endif
    glListBase(lists);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
    glFlush();

}


void parseArgs(int argc, char *argv[])
{
   int oc;

    while((oc = getopt(argc, argv, "w:h:t:p:")) != -1) {
        switch(oc) {
        case 'w':
            window_width = atoi(optarg);
            break;
        case 'h':
            window_height = atoi(optarg);
            break;
        case 't':
            testcase = optarg;
            break;
        case 'p':
            font_name_pattern = optarg;
            break;
        default:
            fprintf(stderr, "unknown option character %c", oc);
            break;
        }
    }
}



int main(int argc, char *argv[])
{
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    GLXContext cx;
    XEvent event;
    GLboolean needDraw = GL_TRUE, needUpdate = GL_TRUE;
    int dummy;

    parseArgs(argc, argv);

    /* Open Xlib Display */
    display = XOpenDisplay(0);
    if (!display)
    {
        fprintf(stderr, "Can't open display\n");
        return -1;
    }

    /*if(glXQueryExtension(display, &dummy, &dummy)) {
        fprintf(stderr, "X server has no OpenGL GLX extentions\n");
        return -1;
    }*/

    vi = glXChooseVisual(display, DefaultScreen(display), dblBuf);
    if (NULL == vi) {
        vi == glXChooseVisual(display, DefaultScreen(display), snglBuf);
        if (NULL == vi) {
            fprintf(stderr, "no RGB visual with depth buffer\n");
            return -1;
        }
        //glf_DoubleBuffer = GL_FALSE;
    }

    if (TrueColor != vi->class) {
        fprintf(stderr, "TrueColor visual required for this program\n");
        return -1;
    }

    cx = glXCreateContext(display, vi, None, GL_TRUE);
    if (NULL == cx) {
        fprintf(stderr, "could not create rendering context\n");
        return -1;
    }

    cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask;
    window = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0,
                        window_width, window_height, 0, vi->depth, InputOutput, vi->visual,
                        CWBorderPixel | CWColormap | CWEventMask, &swa);
    XSetStandardProperties(display, window, "main", "main", None, argv, argc, NULL);
    glXMakeCurrent(display, window, cx);
    XMapWindow(display, window);
    //glfInit();

    font_for_text = XLoadFont(display, font_name_pattern);

    glfDraw();

    glXSwapBuffers(display, window);

    pause();

    XUnloadFont(display, font_for_text);

    XCloseDisplay(display);
#if 0
    while (1) {
        do {
            XNextEvent(display, &event);
            switch(event.type) {
                case KeyPress:
                {
                    KeySym keysym;
                    XKeyEvent *kevent;
                    char buffer[1];
                    kevent = (XKeyEvent*)&event;
                    if (XLookupString((XKeyEvent *)&event, buffer, 1, &keysym, NULL) == 1) {
                        glfProcessKeyboard(keysym);
                        needUpdate = GL_TRUE;
                        break;
                    }
                }
                case ButtonPress:
                {
                    glfProcessMousePress(event.xbutton.x, event.xbutton.y);
                    needUpdate = GL_TRUE;
                    break;
                }
                case ConfigureNotify:
                    glfReshape(event.xconfigure.width, event.xconfigure.height);
                    break;
                case Expose:
                    needDraw = GL_TRUE;
                    break;
            }
        }while(XPending(display));

        if (needUpdate) {
            glfUpdate();
            needUpdate = GL_FALSE;
            needDraw = GL_TRUE;
        }
        if (needDraw) {
            glfDraw();
            glXSwapBuffers(display, window); 
            needDraw = GL_FALSE;
        }
    }
#endif
    return 0;
}

