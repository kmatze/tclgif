tclgif
=========

- tclgif version 0.6 by ma.ke. 2017-04-20 is a tcl package for windows,
- based on AnimGifC (animation feature) by Hubert Dryja <hubertdryja@gmail.com>,
- based on gifsave.c by Sverre H. Huseby <sverrehu@online.no> (see github)
- tclgif can create animated Gifs from tcl/tk photo images.

**features:**

- standard functions of AnimGifC
- add a simple standard vga palette of 16 colors
- add default color

To compile tclgif you need gcc and the header files of tcl/tk 8.3.x or higher.

You can use the simple batch file compile_tclgif.bat.

**general working steps in wish (tk shell):**

	0. load tclgif package
	1. create gif stream
	2. define colors and default color
	3. prepare animation
	4. add images
	5. close gif stream

**commands for working steps:**

	tclgif::new name width height resolution(bits) palette(bits) --> initialize standard vga palette
   	tclgif::colput index r g b  --> return r,g,b and decimal color value for setted color
   	tclgif::coldef index        --> return r,g,b and decimal color value
   	tclgif::animation           --> return number of start frame
   	tclgif::add image delay     --> return number of written frame
   	tclgif::close               --> return number of all written frames

**commands for informations:**

   	tclgif::info                --> return info about gif stream
   	tclgif::colpal              --> return r,g,b and decimal color values of palette
   	tclgif::colget index        --> return r,g,b and decimal color value of palette entry

**example tclgif_tcl.tcl**

	start wish
	source tclgif.tcl          --> create a simple gif animation as file matze.gif
   
![tclgif](/matze.gif)

**todo:**

- implement a canvas to image feature, so you can use a canvas as animation tool
- implement a widget to image feature (like as img::windows), so you can make a screenshot animation 

TRY IT ;-)

I hope you have fun.

Greetings - kmatze (aka ma.ke.) - 20.04.2017






