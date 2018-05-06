@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set path=W:\nutella_compiler\misc;%path%

set _NO_DEBUG_HEAP=1