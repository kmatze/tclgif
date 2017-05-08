@echo off

REM *********************************
set GCC=32
set TCL=83
set TCLDIR=\MINGW\TCL\TCL%TCL
set FILE=tclgif
set EXT=dll

gcc -shared -std=c99 -o3 -o %FILE.%EXT %FILE.c AnimGifC\gifsave.c -DUSE_TCL_STUBS -DUSE_TK_STUBS -I%TCLDIR\include -L%TCLDIR\lib -ltclstub%TCL -ltkStub%TCL
strip %FILE.%EXT
rem \win-apps\tools\upx393.exe --ultra-brute %FILE.%EXT