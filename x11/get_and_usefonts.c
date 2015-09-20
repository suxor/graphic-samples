#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include <X11/Xlib.h>

char *pattern = NULL;
int count = 0;
void parseArgs(int argc, char *argv[])
{
   int oc;
   char *opt_args;

    while((oc = getopt(argc, argv, "p:n:")) != -1) {
        switch(oc) {
        case 'p':
            pattern = optarg;
            break;
        case 'n':
            count = atoi(optarg);
            break;
        default:
            fprintf(stderr, "unknown option character %c", oc);
            break;
        }
    }
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

    default_screen = DefaultScreen(display);

    int count_returned, i;
    XFontStruct *structLists;
    char **fontslist = XListFontsWithInfo(display, pattern, count, &count_returned, &structLists);
    fprintf(stdout, "sizeof Font is %d\n", sizeof(Font));
    for (i = 0; i < count_returned; i ++) {
        fprintf(stdout, 
                "font name: %s, font id: %#x\n",
                fontslist[i],
                (unsigned int)(structLists[i].fid));
        Font font = XLoadFont(display, fontslist[i]);
        fprintf(stdout, "result of XLoadFont is: %#x\n", (unsigned int)(font));
        XUnloadFont(display, font);
    }

    XFreeFontInfo(fontslist, structLists, count_returned);
    


    /* Cleanup */
    XCloseDisplay(display);

    return 0;
}

