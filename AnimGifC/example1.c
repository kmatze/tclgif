#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "gifsave.h"


#define WIDTH        320
#define HEIGHT       240
#define NUMCOLORS      4  /* must change DumpImage if this is changed */
#define COLORRES       8  /* number of bits for each primary color */
#define FILENAME "example1.gif"

/* the image buffer */
static unsigned char Screen[WIDTH][HEIGHT];



/**************************************************************************
 *                                                                        *
 *                   P R I V A T E    F U N C T I O N S                   *
 *                                                                        *
 **************************************************************************/



/*-------------------------------------------------------------------------
 *
 *  NAME          GetPixel
 *
 *  DESCRIPTION   Callback function fetching a pixel value from the buffer.
 *
 *  INPUT         x,y     the location of the pixel
 *
 *  RETURNS       Pixel value, in the range [0, NUMCOLORS).
 */
static int GetPixel(int x, int y)
{
    return Screen[x][y];
}



/*-------------------------------------------------------------------------
 *
 *  NAME          DrawImage
 *
 *  DESCRIPTION   Produces some output inn the image buffer.
 */
static void
DrawColor(int color)
{
    int i,j;
    for(i=0;i<WIDTH;i++){
        for(j=0;j<HEIGHT;j++){
            Screen[i][j] = color;
        }
    }
}

int
main(void)
{
    /* create and set up the GIF-file */
    GIF_Create(FILENAME, WIDTH, HEIGHT, NUMCOLORS, COLORRES);
    /* define a few colors matching the pixel values used */
    GIF_SetColor(0, 0, 0, 0);        // black
    GIF_SetColor(1, 255, 0, 0);      // red
    GIF_SetColor(2, 0, 255, 0);      // green
    GIF_SetColor(3, 255, 255, 255);  // white

    GIF_WriteHeader(0);

    DrawColor(3);
    GIF_AddFrame(0, 0, -1, -1,100, GetPixel);
    DrawColor(2);
    GIF_AddFrame(0, 0, -1, -1,100, GetPixel);
    DrawColor(1);
    GIF_AddFrame(0, 0, -1, -1,100, GetPixel);
    DrawColor(0);
    GIF_AddFrame(0, 0, -1, -1,100, GetPixel);
    DrawColor(3);
    GIF_AddFrame(0, 0, -1, -1,100, GetPixel);

    GIF_Close();

    return 0;
}
