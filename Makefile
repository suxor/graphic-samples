
CC = gcc
INCLUDE_PATH = -Iopengl
CFLAGS = -g $(INCLUDE_PATH)
LDFLAGS = -Wl,-E -lGL -lxcb -lX11-xcb -lX11 -lm  -ldl


globjects = opengl/draw_circle.o opengl/get_version.o opengl/draw_bezier_surface.o opengl/draw_sphere.c
xcbobjects = xcb/xcb-for-opengl.o 
x11objects = x11/x11-for-opengl.o 
objects = $(globjects) $(x11objects) $(xcbobjects)

x11prog = x11test 
xcbprog = xcbtest 

all:$(x11prog) $(xcbprog)

$(x11prog):$(x11objects) $(globjects)
	$(CC) -o $(x11prog) $(x11objects) $(globjects) $(LDFLAGS)
	#rm $(x11objects)

$(xcbprog):$(xcbobjects) $(globjects)
	$(CC) -o $(xcbprog) $(xcbobjects) $(globjects) $(LDFLAGS)
	#rm $(xcbobjects)

$(objects):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
