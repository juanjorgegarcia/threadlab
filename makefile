all:
		gcc main.c `pkg-config --libs --cflags raylib` -pthread -o uicp
