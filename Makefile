
CC = gcc
INCLUDE_PATH = -Iopengl
CFLAGS = -g $(INCLUDE_PATH)
LDFLAGS = -Wl,-E -lGL -lxcb -lX11-xcb -lX11 -lm  -ldl


objects = opengl/draw-circle.o opengl/get-version.o xcb/xcb-for-opengl.o

prog = test 

all:$(objects)
	$(CC) -o $(prog) $(objects) $(LDFLAGS)
	rm $(objects)

$(objects):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
