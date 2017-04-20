REM @echo off

REM *********************************

set MINGW=\win-apps\Tclscripts\MINGW
set VER=83
set TCL=%MINGW\TCL%VER
set FILE=tclgif
set EXT=dll

%MINGW\bin\gcc -shared -std=c99 -o3 -o %FILE.%EXT %FILE.c AnimGifC\gifsave.c -DUSE_TCL_STUBS -DUSE_TK_STUBS -I%TCL\include -L%TCL\lib -ltclstub%VER -ltkStub%VER
strip %FILE.%EXT
\win-apps\tools\upx391.exe --ultra-brute %FILE.%EXT