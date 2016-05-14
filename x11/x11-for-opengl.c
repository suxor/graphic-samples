#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

typedef void (*DRAW_FUNC)(void);

static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
static int dblBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

char *testlib = NULL;
char *testcase = NULL;
int window_width = 800;
int window_height = 600;

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
    glFlush();

    if (NULL != testcase) {
        void *handle = dlopen(testlib, RTLD_NOW | RTLD_GLOBAL);
	if (NULL == handle) {
	    printf("open library[%s] failed: %s", testlib, dlerror());
	    return;
        }

	DRAW_FUNC pf = (DRAW_FUNC)dlsym(handle, testcase);
	if (NULL != pf) pf();
	else printf("testcase[%s] in testlib[%s] dosn't exist.", testcase, testlib);
    }
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
        case 'l':
            testlib = optarg;
            break;
        default:
            fprintf(stderr, "unknown option character %c", oc);
            break;
        }
    }
}

int prepareContext(Display *display, XVisualInfo *vi, Window window)
{
    GLXContext cx;

    cx = glXCreateContext(display, vi, None, GL_TRUE);
    if (NULL == cx) {
        fprintf(stderr, "could not create rendering context\n");
        return -1;
    }

    glXMakeCurrent(display, window, cx);
    
    return 0;
}

/*some problem exist*/
int prepareContextOption2(Display *display, Window window)
{
    GLXContext cx;
    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    GLXFBConfig fb_config;
    GLXWindow glxwindow;

    fb_configs = glXGetFBConfigs(display, DefaultScreen(display), &num_fb_configs);
    if (!fb_configs || num_fb_configs == 0) {
        fprintf(stderr, "glXGetFBConfig failed\n");
	return -1;
    }
    fb_config = fb_configs[0];

    cx = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, GL_TRUE);
    if (NULL == cx) {
        fprintf(stderr, "could not create rendering context\n");
        return -1;
    }

    glxwindow = glXCreateWindow(display, fb_config, window, 0);
    if (!glxwindow)
    {
	glXDestroyContext(display, cx);
	fprintf(stderr, "glXCreateWindow failed\n");
	return -1;
    }

    glXMakeContextCurrent(display, glxwindow, glxwindow, cx);

    return 0;
}

int main(int argc, char *argv[])
{
    Display *display;
    Window window;
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

    if(!glXQueryExtension(display, &dummy, &dummy)) {
        fprintf(stderr, "X server has no OpenGL GLX extentions\n");
        return -1;
    }

    vi = glXChooseVisual(display, DefaultScreen(display), dblBuf);
    if (NULL == vi) {
        vi == glXChooseVisual(display, DefaultScreen(display), snglBuf);
        if (NULL == vi) {
            fprintf(stderr, "no RGB visual with depth buffer\n");
            return -1;
        }
    }

    if (TrueColor != vi->class) {
        fprintf(stderr, "TrueColor visual required for this program\n");
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
    XMapWindow(display, window);

    prepareContext(display, vi, window);
    /*or prepareContextOption2(display, vi, window)*/

    fprintf(stderr, 
           "gl_verdor: %s\n"
           "gl_renderer: %s\n"
           "gl_version: %s\n"
           "glsl_version: %s\n", 
           glGetString(GL_VENDOR),
           glGetString(GL_RENDERER),
           glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION));


    glfDraw();

    glXSwapBuffers(display, window);

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
    
    XCloseDisplay(display);
   
   return 0;
}

