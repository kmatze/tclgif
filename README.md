tclgif
=========

- tclgif version 0.8 by ma.ke. 2017-05-08 is a tcl package for windows
- tclgif is a hobby project with no warrenty
- tclgif can create animated GIFs from tcl/tk photo images
- based on AnimGifC (animation feature) by Hubert Dryja <hubertdryja@gmail.com>
- based on gifsave.c by Sverre H. Huseby <sverrehu@online.no> (see github)

**features:**

- standard functions of AnimGifC
- add simple standard vga palette of 16 colors
- add function to define default color
- add function to get color palette of an image

**compile:**

- to compile tclgif you need gcc and the header files of tcl/tk 8.3.x or higher.
- i use gcc 4.8.3 32bit and tclkit 8.40 under Windows 10.
- you can use the simple batch file compile_tclgif.bat and change it for you.

**general working steps in wish (tk shell):**

	1. create gif stream
	2. define colors and default color
	3. add images (first add starts animation mode)
	4. close gif stream

**commands for working steps:**

	tclgif::new name width height res(bits) pal(bits)
	tclgif::colput index r g b  --> set color, get r,g,b values
	tclgif::coldef index        --> set default color, get r,g,b values
	tclgif::add image delay     --> add image, get frame and udf rgb color list
	tclgif::close               --> close gif stream, get last frame number

**commands for informations:**

	tclgif::help                --> get these infos
	tclgif::info                --> get info about gif stream
	tclgif::colpal              --> get gif color palette as rgb triple bytes
	tclgif::coludf              --> get udf color palette as rgb triple bytes
	tclgif::colget index        --> get r,g,b values of palette entry
	tclgif::colimg image        --> get image colors as list of rgb triple bytes

**example tclgif_tcl.tcl**

	start wish
	source tclgif.tcl          --> create a simple gif animation as file matze.gif
   
![tclgif](/matze.gif)

**todo:**

- implement a canvas to image feature, so you can use a canvas as animation tool
- implement a widget to image feature (like as img::windows), so you can make a screenshot animation 

TRY IT ;-)

I hope you have fun.

Greetings - kmatze (aka ma.ke.) - 08.05.2017






