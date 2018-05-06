@echo off

set CodeDir=W:\nutella_compiler\code
set OutputDir=W:\nutella_compiler\build

set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -Z7 -FC
set CommonCompilerFlags=-DROSEMARY_INTERNAL=1 -DROSEMARY_WINDOWS=1 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib Winmm.lib opengl32.lib

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS %CodeDir%\nutella_compiler.cpp /link %CommonLinkerFlags%

popd
