#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
//#include <GL/gl.h>

#include "samples.h"
typedef void (*DRAW_FUNC)();

static int singleBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
static int doubleBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};
Display *display;
Window window;

char *testcase = NULL;
int window_width = 800;
int window_height = 600;
Font font_for_text = 0;
char *font_name_pattern = NULL;


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


void draw() {
    if (NULL != testcase) {
        DRAW_FUNC pf = (DRAW_FUNC)dlsym(0, testcase);
        if (NULL != pf) {
            fprintf(stderr, "call %s \n", testcase);
            pf();
            return;
        }
    }
    get_version();

    /*static int isFirstCall = 1;
    static GLuint lists;
    //load font
    //gen list
    //if (isFirstCall) {
        lists = glGenLists(128);
        glXUseXFont(font_for_text, 0, 128, lists);
    //}

    char *str = "Hello, world";
    for (; *str!='\0';++str) {
        glCallList(lists + *str);
    }

    glDeleteLists(lists, 128);
    //use font
    //draw text
    //unuse font?
    //delete list
    //unload font*/
}

int main_loop(Display *display, xcb_connection_t *connection, xcb_window_t window, GLXDrawable drawable)
{
    int running = 1;
    while (running) {
        /* Wait for event */
        xcb_generic_event_t *event = xcb_wait_for_event(connection);
        if (!event) {
            fprintf(stderr, "i/o error in xcb_wait_for_event\n");
            return -1;
        }

        switch(event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
                /* Quit on key press */
                running = 0;
                break;
            case XCB_EXPOSE:
                /* Handle expose event, draw and swap buffers */
                draw();
                glXSwapBuffers(display, drawable);
                break;
            default:
                break;
        }
        free(event);
    }
    return 0;
}

int setup_and_run(Display *display, int default_screen)
{
    int visualID = 0;

    /*Query framebuffer configurations */
    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    fb_configs = glXGetFBConfigs(display, default_screen, &num_fb_configs);
    if (!fb_configs || num_fb_configs == 0)
    {
        fprintf(stderr, "glXGetFBConfigs failed\n");
        return -1;
    }

    /* Select first framebuffer config and query visualID */
    GLXFBConfig fb_config = fb_configs[0];
    glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID, &visualID);

    GLXContext context;

    /* Create OpenGL context */
    context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
    if (!context)
    {
        fprintf(stderr, "glXCreateNewContext failed\n");
        return -1;
    }

    uint32_t eventmask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
    uint32_t valuelist[] = {eventmask, colormap, 0};
    uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

    xcb_create_window(
        connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0,
        window_width, window_height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        visualID,
        valuemask,
        valuelist);

    /* NOTE: window must be mapped before glXMakeContextCurrent */
    xcb_map_window(connection, window);

    /* Create GLX window */
    GLXDrawable drawable = 0;
    GLXWindow glxwindow = glXCreateWindow(display, fb_config, window, 0);
    if (!glxwindow)
    {
        xcb_destroy_window(connection, window);
        glXDestroyContext(display, context);
        fprintf(stderr, "glXCreateWindow failed\n");
        return -1;
    }

    drawable = glxwindow;

    /* make OpenGL context current */
    if (!glXMakeContextCurrent(display, drawable, drawable, context))
    {
        xcb_destroy_window(connection, window);
        glXDestroyContext(display, context);
        fprintf(stderr, "glXMakeContextCurrent failed\n");
        return -1;
    }

    /* run main loop */
    int retval = main_loop (display, connection, window, drawable);

    /* Cleanup */
    glXDestroyWindow(display, glxwindow);
    xcb_destroy_window(connection, window);
    glXDestroyContext(display, context);
    return retval;
}

int main(int argc, char *argv[])
{
    XVisualInfo *vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    GLXContext cx;
    XEvent event;
    GLboolean needDraw = GL_FALSE, needUpdate = GL_TRUE;
    int dummy;

    parseArgs(argc, argv);

    /* Open Xlib Display */
    display = XOpenDisplay(0);
    if (!display)
    {
        fprintf(stderr, "Can't open display\n");
        return -1;
    }

    if(glXQueryExtension(display, &dummy, &dummy)) {
        fprintf(stderr, "X server has no OpenGL GLX extentions\n");
        return -1;
    }

    vi = glXChooseVisual(dpy, DefaultScreen(display), dblBuf);
    if (NULL == vi) {
        vi == glXChooseVisual(display, DefaultScreen(display), snglBuf);
        if (NULL == vi) {
            fprintf(stderr, "no RGB visual with depth buffer\n");
            return -1;
        }
        glf_DoubleBuffer = GL_FALSE;
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
    swa.colomap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask;
    win = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0,
                        glf_WinWidth, glf_WinHeight, 0, vi->depth, InputOutput, vi->visual,
                        CWBorderPixel | CWColormap | CWEventMask, &swa);
    XSetStandardProperties(display, window, "main", "main", None, argv, argc, NULL);
    glXMakeCurrent(display, window, cx);
    XMapWindow(display, window);
    glfInit();

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
            glfUpdate()
            needUpdate = GL_FALSE;
            needDraw = GL_TRUE;
        }
        if (needDraw) {
            glDraw();
            needDraw = GL_FALSE;
        }
    }
    return 0;
}

void glfInit() 
{
}

void glProcessKeyboard(KeySym keysym)
{
    if (keysym == (KeySym)XK_Escapse)
        exit(0);
}

void glProcessMousePress(int x, int y)
{
}

void glfUpdate()
{
}
void glfReshape(int width, int height)
{
    glf_WinWidth = width;
    glf_WinHeight = height;
    glViewPort(0, 0, glf_WinWidth, glf_WinHeight);
}

void glfDraw()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

