#ifndef GIFSAVE_H
#define GIFSAVE_H

enum GIF_Code {
    GIF_OK,
    GIF_ERRCREATE,
    GIF_ERRWRITE,
    GIF_OUTMEM
};

int  GIF_Create(const char *filename, int width, int height,
		int numcolors, int colorres);
void GIF_SetColor(int colornum, int red, int green, int blue);
int  GIF_WriteHeader(int repeats);
int  GIF_AddFrame(int left, int top, int width, int height, int delay,
		       int (*getpixel)(int x, int y));
int  GIF_Close(void);
#endif
