/**************************************************************************
 *
 *  FILE            tclgif.c version 0.6 by ma.ke. 2017-04-19
 *
 *  ORIGINAL FILE   gifsave.c
 *
 *  DESCRIPTION     Routines to create a GIF-file. See README for
 *                  a description.
 *
 *                  The functions were originally written using Borland's
 *                  C-compiler on an IBM PC -compatible computer, but they
 *                  are compiled and tested on Linux and SunOS as well.
 *
 *  WRITTEN BY      Sverre H. Huseby <sverrehu@online.no>
 *  GIF ANIMATION ADDED BY Hubert Dryja <hubertdryja@gmail.com>
 *
 *  GIFSAVE         is hereby dedicated to the Public Domain.
 *                  Feel free to change the source code as you wish.
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tcl.h>
#include <tk.h>
#include <math.h>
#include "AnimGifC\gifsave.h"

#define NS        "tclgif"
#define VERSION   "0.8"
#define VERSDATUM "08-05-2017"

#define FALSE 0
#define TRUE  1

static unsigned char vga[0x30] = {
    0x00, 0x00, 0x00,	// black
    0xFF, 0xFF, 0xFF,	// white
    0xFF, 0x00, 0x00,	// red
    0xFF, 0xFF, 0x00,   // yellow
    0x00, 0xFF, 0x00,   // lime
    0x00, 0xFF, 0xFF,   // aqua
    0x00, 0x00, 0xFF,   // blue
    0xFF, 0x00, 0xFF,   // fuchsia
    0x80, 0x80, 0x80,   // gray
    0xC0, 0xC0, 0xC0,   // silver
    0x80, 0x00, 0x00,   // maroon
    0x80, 0x80, 0x00,   // olive
    0x00, 0x80, 0x00,   // green
    0x00, 0x80, 0x80,   // teal
    0x00, 0x00, 0x80,   // navy
    0x80, 0x00, 0x80,   // purple
};

enum { NEW, ANIMATION, CLOSE};

typedef struct mkRGB {
		unsigned char 	   b;				// b value
		unsigned char 	   g;				// g value
		unsigned char 	   r;				// r value
} mkRGB;

union convert {								// color convert integer <-> 3 bytes
	unsigned int 	dec:24;					// 24 bit integer for 3 bytes r,g,b
	mkRGB 			rgb;					// 3 bytes r, g, b
};

typedef struct mkGIF {
        char               *filename;   	// pointer to filename
    	int                width;       	// width  of screen in pixels
   		int                height;      	// height of screen in pixels
    	int                colres;      	// number of bits for each primary color
    	int 			   coltab;			// number of colors in color table
    	mkRGB	     	   *palette;    	// pointer to gif color table for r,g,b values
    	unsigned char      coldefault;  	// default gif color index for palette
    	unsigned char 	   **screen;    	// pointer/pointer to 2D screen buffer [width * height] of gif image
    	int                status;  		// NEW | ANIMATION | CLOSE
    	int                numcolors; 		// number of colors
    	int                numframes;   	// number of frames
	   	int				   LastPixelRgb;	// rgb value of last read pixel
    	unsigned char      LastPixelGif;	// gif value of last read pixel
    	int                up;              // counter for undefined pixels
		int                *upc;			// pointer to undifined colors in decimal values
		int				   upccnt;			// counter of undifined colors
} mkGIF;
mkGIF gif;
/*
 * helper makros for
 * ... tcl/tk programming
 */
#define TCL_MSG(m)      	Tcl_AppendResult(interp, fstring("%s", m), NULL);
#define TCL_ERR(m)      	{ Tcl_AppendResult(interp, fstring("%s", m), NULL); return TCL_ERROR; }
#define CHECK_ARGS(n,m) 	if (argc != (n) + 1) TCL_ERR(fstring("wrong arguments: %s", m));
#define CHECK_BYTE_VALUE(n) if (n <0 || n > 255) TCL_ERR("gif error: byte value must be 0 ... 255");
/*
 * helper makros for
 * ... graphic programming
 */
#define R(i)	i[PhotoBlock.offset[0]];
#define G(i)    i[PhotoBlock.offset[1]];
#define B(i)    i[PhotoBlock.offset[2]];
#define A(i)    i[PhotoBlock.offset[3]];

/****************************************************************************************
 *
 * c gif utilities
 *
 ****************************************************************************************/
 
/*
 *  NAME          GetPixel
 *  DESCRIPTION   Callback function fetching a pixel value from the buffer.
 *  INPUT         x,y      the location of the pixel
 *  RETURNS       Pixel value, in the range [0, NUMCOLORS).
 *
 */
static int mkGetPixel(int x, int y) { return gif.screen[x][y]; }

void mkSetColor(int i, int r, int g, int b)						// set palette[index] with color values r, g, b and decimal rgb
{
	gif.palette[i].r   = r;
	gif.palette[i].g   = g;
	gif.palette[i].b   = b;
}

void mkSetVgaColors()											// init palette with standard 16 VGA colors
{
	int i;
	for ( i = 0; i < 16 ; i++ ) {
		if (i >= gif.numcolors ) break;							// less then 16 colors
		mkSetColor(i, vga[i*3+0], vga[i*3+1], vga[i*3+2]);
	}
}

void mkClearColor()												// clear undefined color table
{
	int i;
	for ( i = 0; i < gif.upccnt; i++ ) gif.upc[i] = 0;	
    gif.upccnt = 0;						 					    // table set to first entry at index 0

}

void mkCheckColor(int color)									// check new color and set this
{
	int i; int colorfound;
	union convert c;
	
	for ( i = 0; i < gif.upccnt; i++ ) {						// find gif color index for decimal value rgb
		c.dec = gif.upc[i];					     				// read decimal rgb value of palette index
		if (color == c.dec) return;								// pixelcolor found in table
	}

	gif.upc[gif.upccnt++] = color;	  			        		// create new color entry for r,g,b and set next color index
	
	return;
}

/****************************************************************************************
 *
 * c utilities
 *
 ****************************************************************************************/

const char* fstring( char* fmt, ...)
{
    // eg.: fstring("%s", "zf> error: ");
    static char buf[100];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf( buf, sizeof( buf), fmt, vl);
    va_end( vl);
    return buf;
}

/****************************************************************************************
 *
 * tcl procedures 
 *
 ****************************************************************************************/

int Help_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	TCL_MSG("-----------------------------------------------------------------------------\n");
	TCL_MSG("tcl package to create animated gif " VERSION ", " VERSDATUM " (c) ma.ke.\n");
	TCL_MSG("based on gifsave.c by Sverre H. Huseby <sverrehu@online.no>\n");
	TCL_MSG("     and AnimGifC  by Hubert Dryja     <hubertdryja@gmail.com>\n");
	TCL_MSG("-----------------------------------------------------------------------------\n");
	TCL_MSG("working steps:\n");
	TCL_MSG("   1. create gif stream\n");
	TCL_MSG("   2. define colors and default color\n");
	TCL_MSG("   3. add images (first add start animation mode)\n");
	TCL_MSG("   4. close gif stream\n");
	TCL_MSG("commands for working steps:\n");
	TCL_MSG("   " NS "::new name width height res(bits) pal(bits)\n");
	TCL_MSG("   " NS "::colput index r g b  --> set color, get r,g,b values\n");
	TCL_MSG("   " NS "::coldef index        --> set default color, get r,g,b values\n");
	TCL_MSG("   " NS "::add image delay     --> add image, get frame and udf rgb color list\n");
	TCL_MSG("   " NS "::close               --> close gif stream, get last frame number\n");
	TCL_MSG("commands for informations:\n");
	TCL_MSG("   " NS "::help                --> get these infos\n");
	TCL_MSG("   " NS "::info                --> get info about gif stream\n");
	TCL_MSG("   " NS "::colpal              --> get gif color palette as rgb triple bytes\n");
	TCL_MSG("   " NS "::coludf              --> get udf color palette as rgb triple bytes\n");	
	TCL_MSG("   " NS "::colget index        --> get r,g,b values of palette entry\n");
	TCL_MSG("   " NS "::colimg image        --> get image colors as list of rgb triple bytes");
	return TCL_OK;
}

int ColorPalette_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	if (gif.status == CLOSE)      TCL_ERR("gif error: gif stream not open, use new command first");		// error
	int i;

	for ( i = 0; i < gif.numcolors ; i++ ) {
		TCL_MSG(fstring("%3i %3i %3i\n", gif.palette[i].r, gif.palette[i].g, gif.palette[i].b));  		// return r,g,b values
	}

	return TCL_OK;
}

int UdfColorPalette_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	int i;
	union convert c;
	
	for (i = 0; i < gif.upccnt; i++) {																	// return found colors
		c.dec = gif.upc[i];																				// convert to r,g,b values
		TCL_MSG(fstring("%3i %3i %3i\n", c.rgb.r, c.rgb.g, c.rgb.b));									// return r,g,b values
	}
	return TCL_OK;
}

int ColorGet_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	CHECK_ARGS(1, "index");

	if (gif.status == CLOSE)      TCL_ERR("gif error: gif stream not open, use new command first");

	int i;

	if (Tcl_GetInt(interp, argv[1], &i) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read index
    if (i  <0 || i > gif.numcolors - 1) TCL_ERR(fstring("gif error: index must be 0 ... %i", gif.numcolors - 1));	// error index value

	TCL_MSG(fstring("%3i %3i %3i", gif.palette[i].r, gif.palette[i].g, gif.palette[i].b));      					// return r,g,b values

	return TCL_OK;
}

int ColorDefault_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{

	CHECK_ARGS(1, "index");

	if (gif.status == CLOSE)      TCL_ERR("gif error: gif stream not open, use new command first");
	if (gif.status == ANIMATION)  TCL_ERR("gif error: gif stream is already in animation mode");

	int i;

	if (Tcl_GetInt(interp, argv[1], &i) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read index
    if (i  <0 || i > gif.numcolors - 1) TCL_ERR(fstring("gif error: index must be 0 ... %i", gif.numcolors - 1));	// error index value

    gif.coldefault = i;

	TCL_MSG(fstring("%3i %3i %3i", gif.palette[i].r, gif.palette[i].g, gif.palette[i].b));      					// return r,g,b values

	return TCL_OK;
}

int ColorPut_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	CHECK_ARGS(4, "index r g b");

	if (gif.status == CLOSE)      TCL_ERR("gif error: gif stream not open, use new command first");
	if (gif.status == ANIMATION)  TCL_ERR("gif error: gif stream is already in animation mode");

	int i, r, g, b;

	if (Tcl_GetInt(interp, argv[1], &i) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read index
	if (Tcl_GetInt(interp, argv[2], &r) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read r
	if (Tcl_GetInt(interp, argv[3], &g) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read g
	if (Tcl_GetInt(interp, argv[4], &b) != TCL_OK) 	TCL_ERR("tcl error: failure parameter index");					// error read b
    if (i <0 || i > gif.numcolors - 1) TCL_ERR(fstring("gif error: index must be 0 ... %i", gif.numcolors - 1));	// error index value

    CHECK_BYTE_VALUE(r); CHECK_BYTE_VALUE(g); CHECK_BYTE_VALUE(b);													// error byte value error

	mkSetColor(i, r, g, b);																							// set color
	TCL_MSG(fstring("%3i %3i %3i", gif.palette[i].r, gif.palette[i].g, gif.palette[i].b));      					// return r,g,b values

	return TCL_OK;
}

int New_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	CHECK_ARGS(5, "filename width height colres colpal")

	if (gif.status == NEW)        TCL_ERR("gif error: only one gif stream can be open");
	if (gif.status == ANIMATION)  TCL_ERR("gif error: gif stream is already in animation mode");

	if (Tcl_GetInt(interp, argv[2], &gif.width) != TCL_OK) 	    TCL_ERR("tcl error: failure parameter width");		// error read width
	if (Tcl_GetInt(interp, argv[3], &gif.height)!= TCL_OK)	    TCL_ERR("tcl error: failure parameter height");		// error read height
	if (Tcl_GetInt(interp, argv[4], &gif.colres ) != TCL_OK)	TCL_ERR("tcl error: failure parameter colres");		// error read colres
	if (Tcl_GetInt(interp, argv[5], &gif.coltab ) != TCL_OK)	TCL_ERR("tcl error: failure parameter coltab");		// error read coltab
	if (gif.colres < 1 || gif.colres > 8) TCL_ERR("gif error: colres must be 1 ... 8");						        // error colres
	if (gif.coltab < 1 || gif.coltab > 8) TCL_ERR("gif error: coltab must be 1 ... 8");						        // error coltab

	gif.filename  = (char *)calloc(strlen(argv[1])+1, sizeof(char));												// memory allocation for filename
	strcpy(gif.filename, argv[1]);																					// get filename

    int i;
	gif.screen  = (unsigned char **)calloc(gif.width, sizeof(int));													// memory allocation for screen buffer
	for(i = 0; i < gif.width; i++) gif.screen[i] = (unsigned char *) calloc(gif.height, sizeof(unsigned char));

	gif.numcolors  = pow(2, gif.coltab);																			// calculate number of colors
	gif.palette    = (mkRGB *)calloc(gif.numcolors * sizeof(mkRGB), sizeof(mkRGB));				      				// memory allocation for palette
    gif.upc    	   = (int *)calloc(gif.width * gif.height * sizeof(int), sizeof(int));					    		// memory allocation for upc - max w*h

    GIF_Create(gif.filename, gif.width, gif.height, gif.numcolors, gif.colres); 								    // create gif file
	mkSetVgaColors();																								// initialize with vga colors

    gif.status = NEW;
    return TCL_OK;
}

int Info_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	if (gif.status == CLOSE)  TCL_ERR("gif error: gif stream not open, use new command first");
	int i, r, g, b;

    TCL_MSG(fstring("status          : %i (0=NEW 1=ANIMATION)\n", gif.status));
    TCL_MSG(fstring("filename        : %s\n", gif.filename));
    TCL_MSG(fstring("width           : %i\n", gif.width));
   	TCL_MSG(fstring("height          : %i\n", gif.height));
    TCL_MSG(fstring("color resolution: %i bit\n", gif.colres));
    TCL_MSG(fstring("color palette   : 2^%i = %i colors\n", gif.coltab, gif.numcolors));

	i = gif.coldefault;
	r = gif.palette[i].r; g = gif.palette[i].g; b = gif.palette[i].b;
    TCL_MSG(fstring("color default   : %i - r(%i) g(%i) b(%i)\n", i, r, g, b));

    TCL_MSG(fstring("last frame      : %i\n", gif.numframes));
    TCL_MSG(fstring("undefined pixels: %i\n", gif.up));
	TCL_MSG(fstring("undefined colors: %i\n", gif.upccnt));

	i = gif.LastPixelGif;
	r = gif.palette[i].r; g = gif.palette[i].g; b = gif.palette[i].b;
    TCL_MSG(fstring("last pixel color: %i - r(%i) g(%i) b(%i)\n", i, r, g, b))

	return TCL_OK;
}

int Close_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	if (gif.status == CLOSE)   TCL_ERR("gif error: no open stream");									// error
	int i;

    GIF_Close();																						// close gif stream
	gif.status = CLOSE;
	gif.LastPixelRgb = gif.LastPixelGif = 0;

	for (i = 0; i < gif.width; i++) free(gif.screen[i]);												// free memory allocation
	free(gif.screen);
	free(gif.palette);
	free(gif.filename);
	free(gif.upc);

	TCL_MSG(fstring("%i", gif.numframes));																// return number of frames

    return TCL_OK;
}

int Add_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	CHECK_ARGS( 2, "image delay" );

	int delay, i;
	if (Tcl_GetInt(interp, argv[2], &delay) != TCL_OK) 	TCL_ERR("tcl error: failure parameter delay");	// error read delay
	if (delay < 1 || delay > 100)  TCL_ERR("gif error: delay must be 1 ... 100");						// error delay value

	if (gif.status == NEW) {																			// prepare for animation
		for ( i = 0; i < gif.numcolors ; i++ ) {														// fill gif palette
			GIF_SetColor(i, gif.palette[i].r, gif.palette[i].g, gif.palette[i].b);
		}
		GIF_WriteHeader(0);																				// open gif stream
		gif.status    = ANIMATION;																		// set animation staus
		gif.numframes = 0;																				// set start frame
	}
	
	if (gif.status != ANIMATION)  TCL_ERR("gif error: gif stream not open or not in animation mode");	// error not in animation mode
/*
	typedef struct Tk_PhotoImageBlock {
    	unsigned char 	   *pixelPtr;   // Pointer to the first pixel.
    	int                width;       // Width of block, in pixels.
   		int                height;      // Height of block, in pixels.
    	int                pitch;       // Address difference between corresponding
                                    	// pixels in successive lines.
    	int                pixelSize;   // Address difference between successive
                                    	// pixels in the same line.
    	int                offset[3];   // Address differences between the red, green
                                    	// and blue components of the pixel and the
                                    	// pixel as a whole.
    	int                reserved;    // Reserved for extensions (dash patch)
	} Tk_PhotoImageBlock;
*/
    Tk_PhotoHandle PhotoHandle;
	Tk_PhotoImageBlock PhotoBlock;
    int PhotoGet, PhotoPixels;

	PhotoHandle = Tk_FindPhoto(interp, argv[1]);														// register image in handle
    if (!PhotoHandle) TCL_ERR (fstring ("tcl error: %s is not a photo image", argv[1]));				// error image failure
	if (Tcl_GetInt(interp, argv[2], &delay) != TCL_OK) 	TCL_ERR("tcl error: failure parameter delay");	// error looptime failure

	PhotoGet    = Tk_PhotoGetImage( PhotoHandle, &PhotoBlock);											// read the complete photoblock
	PhotoPixels = PhotoBlock.width * PhotoBlock.height;													// determine the number of pixels
    if (PhotoPixels == 0 ) TCL_ERR("gif error: empty image");											// error empty image

	if (gif.width != PhotoBlock.width || gif.height != PhotoBlock.height)
		TCL_ERR("gif error: wrong image dimensions");													// error dimension gif != image
	//
	//  convert rgb to gif color index in gif table
	//
   	int gifNew, colorfound;
   	int w, h;
    register unsigned char *srcPtr = PhotoBlock.pixelPtr;
    union convert colNew, c;																    		// use to color convert rgb <-> integer

    gif.up = 0;																							// reset counter of undefined pixel
	mkClearColor();			 																		    // clear undefined color palette
	
	for (h = 0; h < gif.height; h++) {																	// read color values from each pixel
		for (w = 0; w < gif.width; w++) {
			colorfound = FALSE;
			gifNew = gif.LastPixelGif;																	// get gif index from last pixel
			colNew.rgb.r = R(srcPtr);																	// read r,g,b for actual pixel
			colNew.rgb.g = G(srcPtr);
			colNew.rgb.b = B(srcPtr);
			if (colNew.dec != gif.LastPixelRgb) {												        // not same color as last pixel
				for ( i = 0; i < gif.numcolors; i++ ) {													// find gif color index for decimal value rgb
				    c.rgb.r = gif.palette[i].r;															// read r,g,b values of palette index
				    c.rgb.g = gif.palette[i].g;
				    c.rgb.b = gif.palette[i].b;
					if (colNew.dec == c.dec) {															// pixelcolor found in gif tabelle
						gifNew = i; 																	// set gif color index
						colorfound = TRUE;																// set flag colorfound
						break;
					}
				}

				if (colorfound == FALSE) { 																// color not found in color table
					gifNew = gif.coldefault;															// set default gif color index
					gif.up++;																			// counter for undefined pixel
					mkCheckColor(colNew.dec);															// check and add undefined color
				}

			}
            gif.screen[w][h] = gifNew;  																// put gif color for pixel
			srcPtr += PhotoBlock.pixelSize;																// prepare for next pixel
			gif.LastPixelRgb = colNew.dec;
			gif.LastPixelGif = gifNew;
		}
	}

    GIF_AddFrame(0, 0, -1, -1, delay, mkGetPixel);														// add image to animated gif
    gif.numframes++;

	TCL_MSG(fstring("%i %i\n", gif.numframes, gif.up)) 													// return frame number, undefined pixels
	UdfColorPalette_cmd(cdata, interp, argc, argv);                                                     // return found undefined colors
 
	return TCL_OK;
}

int ColorImage_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	CHECK_ARGS( 1, "image" );
	if (gif.status == CLOSE)   TCL_ERR("gif error: no open stream");									// error
	
    Tk_PhotoHandle PhotoHandle;
	Tk_PhotoImageBlock PhotoBlock;
    int PhotoGet, PhotoPixels;

	PhotoHandle = Tk_FindPhoto(interp, argv[1]);														// register image in handle
    if (!PhotoHandle) TCL_ERR (fstring ("tcl error: %s is not a photo image", argv[1]));				// error image failure

	PhotoGet    = Tk_PhotoGetImage( PhotoHandle, &PhotoBlock);											// read the complete photoblock
	PhotoPixels = PhotoBlock.width * PhotoBlock.height;													// determine the number of pixels
    if (PhotoPixels == 0 ) TCL_ERR("gif error: empty image");											// error empty image

	//
	//  convert rgb  colors to gif color
	//
	
   	int i, w, h, colorfound, coldefaultLast;
    register unsigned char *srcPtr = PhotoBlock.pixelPtr;												// image pointer
    union convert c;																		    		// use to color convert rgb <-> integer

	mkClearColor();																					    // clear undefined color palette

	for (h = 0; h < gif.height; h++) {																	// read color values from each pixel
		for (w = 0; w < gif.width; w++) {
			colorfound = FALSE;
			c.rgb.r = R(srcPtr); c.rgb.g = G(srcPtr); c.rgb.b = B(srcPtr);								// read r,g,b for actual pixel
			if (c.dec != gif.LastPixelRgb) {													        // not same color as last pixel
				mkCheckColor(c.dec);																	// check and add new color
			}
			srcPtr += PhotoBlock.pixelSize;																// prepare for next pixel
			gif.LastPixelRgb = c.dec;
		}
	}

	UdfColorPalette_cmd(cdata, interp, argc, argv);                                                     // return found colors
  	return TCL_OK;
}

/****************************************************************************************
 *
 * tcl main procedure
 *
 ****************************************************************************************/

int Tclgif_Init(Tcl_Interp *interp)
{
	if (Tcl_InitStubs (interp, TCL_VERSION, 0) == NULL) 		return TCL_ERROR;
	if (Tcl_PkgProvide(interp, NS, VERSION)    == TCL_ERROR)	return TCL_ERROR;
  	if (Tk_InitStubs(interp, TK_VERSION, 0)    == NULL) 		return TCL_ERROR;

  	gif.status=CLOSE;

  	Tcl_CreateCommand(interp, NS "::help",       (Tcl_CmdProc*)Help_cmd, NULL, NULL);
  	Tcl_CreateCommand(interp, NS "::info",       (Tcl_CmdProc*)Info_cmd, NULL, NULL);
	Tcl_CreateCommand(interp, NS "::new",        (Tcl_CmdProc*)New_cmd, NULL, NULL);
  	Tcl_CreateCommand(interp, NS "::colpal",     (Tcl_CmdProc*)ColorPalette_cmd, NULL, NULL);
	Tcl_CreateCommand(interp, NS "::coludf",     (Tcl_CmdProc*)UdfColorPalette_cmd, NULL, NULL);
  	Tcl_CreateCommand(interp, NS "::colget",     (Tcl_CmdProc*)ColorGet_cmd, NULL, NULL);
  	Tcl_CreateCommand(interp, NS "::colput",     (Tcl_CmdProc*)ColorPut_cmd, NULL, NULL);
	Tcl_CreateCommand(interp, NS "::colimg",     (Tcl_CmdProc*)ColorImage_cmd, NULL, NULL);
  	Tcl_CreateCommand(interp, NS "::coldef",     (Tcl_CmdProc*)ColorDefault_cmd, NULL, NULL);
	Tcl_CreateCommand(interp, NS "::add",        (Tcl_CmdProc*)Add_cmd, NULL, NULL);
	Tcl_CreateCommand(interp, NS "::close",      (Tcl_CmdProc*)Close_cmd, NULL, NULL);

  	return TCL_OK;

}

