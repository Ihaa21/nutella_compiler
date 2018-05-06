@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set _NO_DEBUG_HEAP=1

set CodeDir=..\code
set OutputDir=..\build

set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -Z7 -FC
set CommonLinkerFlags=-incremental:no -opt:ref

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS %CodeDir%\nutella_compiler.cpp /link %CommonLinkerFlags%

set path=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;%path%

REM echo test_factorial
nutella_compiler %OutputDir%\test_factorial.nutella
ml /Zi /Cx /coff %OutputDir%\test_factorial.asm /link /SUBSYSTEM:console
REM echo test_float_math
nutella_compiler %OutputDir%\test_float_math.nutella
ml /Zi /Cx /coff %OutputDir%\test_float_math.asm /link /SUBSYSTEM:console
REM echo test_if
nutella_compiler %OutputDir%\test_if.nutella
ml /Zi /Cx /coff %OutputDir%\test_if.asm /link /SUBSYSTEM:console
REM echo test_int_math
nutella_compiler %OutputDir%\test_int_math.nutella
ml /Zi /Cx /coff %OutputDir%\test_int_math.asm /link /SUBSYSTEM:console
REM echo test_more_math
nutella_compiler %OutputDir%\test_more_math.nutella
ml /Zi /Cx /coff %OutputDir%\test_more_math.asm /link /SUBSYSTEM:console
REM echo test_pointers
nutella_compiler %OutputDir%\test_pointers.nutella
ml /Zi /Cx /coff %OutputDir%\test_pointers.asm /link /SUBSYSTEM:console

