#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <GL/glx.h>
//#include <GL/gl.h>

#include "samples.h"
typedef void (*DRAW_FUNC)();

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

GLubyte letters[][13] = {
    {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
};

void draw(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window) {
    if (NULL != testcase) {
        DRAW_FUNC pf = (DRAW_FUNC)dlsym(0, testcase);
        if (NULL != pf) {
            fprintf(stderr, "call %s \n", testcase);
            pf();
            return;
        }
    }

    //get_version();
   
    xcb_font_t font = xcb_generate_id(connection);
    xcb_void_cookie_t cookie_font = xcb_open_font_checked(connection, font, strlen(font_name_pattern), font_name_pattern);
    xcb_generic_error_t *error = xcb_request_check(connection, cookie_font);
    if (error) {
        fprintf(stderr, "ERROR: can't open font :%d\n", error->error_code);
        xcb_disconnect(connection);
        exit(0);
    }
    
    xcb_gcontext_t gc = xcb_generate_id(connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    uint32_t value_list[3];
    value_list[0] = screen->black_pixel;
    value_list[1] = screen->white_pixel;
    value_list[2] = font;
    xcb_void_cookie_t cookie_gc = xcb_create_gc_checked(connection, gc, window, mask, value_list);
    error = xcb_request_check(connection, cookie_gc);
    if (error) {
        fprintf(stderr, "ERROR: can't create gc: %d\n", error->error_code);
        xcb_disconnect(connection);
        exit(0);
    }

    cookie_font = xcb_close_font_checked(connection, font);
    error = xcb_request_check(connection, cookie_font);
    if (error) {
        fprintf(stderr, "ERROR: can't close font: %d\n", error->error_code);
        xcb_disconnect(connection);
        exit(0);
    }
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(0.0f, 0.0f);
    glPushAttrib(GL_LIST_BIT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    static int isFirstCall = 1;
    static GLuint lists;
    if (isFirstCall) {
        lists = glGenLists(128);
    }
#if 1 
    //for (i = 0; i < sizeof(letters)
    glNewList(lists + 'A', GL_COMPILE);
    glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[0]);
    glEndList();
    GLubyte *str = (GLubyte*)"A";
#else
    glXUseXFont(font, 0, 128, lists);
    GLubyte *str = (GLubyte*)"Hello, world.";
#endif
    glListBase(lists);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
    glFlush();


/*    static int isFirstCall = 1;
    static GLuint lists;
    //load font
    //gen list
    if (isFirstCall) {
        lists = glGenLists(128);
    }

    glXUseXFont(font, 0, 128, lists);
    glListBase(lists);
    char *str = "Hello, world";
    for (; *str!='\0';++str) {
        glCallList(lists + *str);
    }
    glFlush();
    //glDeleteLists(lists, 256);
    //use font
    //draw text
    //unuse font?
    //delete list
    //unload font*/

    cookie_gc = xcb_free_gc(connection, gc);
    error = xcb_request_check(connection, cookie_gc);
    if (error) {
        fprintf(stderr, "ERROR: can't free gc: %d\n", error->error_code);
        xcb_disconnect(connection);
        exit(0);
    }
}

int main_loop(Display *display, xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window, GLXDrawable drawable)
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
                draw(connection, screen, window);
                glXSwapBuffers(display, drawable);
                break;
            default:
                break;
        }
        free(event);
    }
    return 0;
}

int setup_and_run(Display *display, xcb_connection_t *connection, int default_screen, xcb_screen_t *screen)
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

    /* Create XID's for colormap and window */
    xcb_colormap_t colormap = xcb_generate_id(connection);
    xcb_window_t window  = xcb_generate_id(connection);

    /* Create colormap */
    xcb_create_colormap(
        connection,
        XCB_COLORMAP_ALLOC_NONE,
        colormap,
        screen->root,
        visualID);

    /* Create window */
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
    int retval = main_loop (display, connection, screen, window, drawable);

    /* Cleanup */
    glXDestroyWindow(display, glxwindow);
    xcb_destroy_window(connection, window);
    glXDestroyContext(display, context);
    return retval;
}

int main(int argc, char *argv[])
{
    Display *display;
    int default_screen, screen_num;

    parseArgs(argc, argv);

    /* Open Xlib Display */
    display = XOpenDisplay(0);
    if (!display)
    {
        fprintf(stderr, "Can't open display\n");
        return -1;
    }
    //font_for_text = XLoadFont(display, font_name_pattern);

    default_screen = DefaultScreen(display);

    /* Get the XCB connection from the display */
    xcb_connection_t *connection = XGetXCBConnection(display);
    if (!connection)
    {
        XCloseDisplay(display);
        fprintf(stderr, "Can't get xcb connection from display\n");
        return -1;
    }

    /* Acquire event ownership */
    XSetEventQueueOwner(display, XCBOwnsEventQueue);
    
    /* Find XCB screen */
    xcb_screen_t *screen = 0;
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
    for (screen_num = default_screen; screen_iter.rem && screen_num > 0; --screen_num, xcb_screen_next(&screen_iter));
    screen = screen_iter.data;

    /* Initialize window and OpenGL context, run main loop and deinitialize */
    int retval = setup_and_run(display, connection, default_screen, screen);

    //XUnloadFont(display, font_for_text);
    /* Cleanup */
    XCloseDisplay(display);

    return retval;
}

