all:
		cc main.c `pkg-config --libs --cflags raylib` -o uicp