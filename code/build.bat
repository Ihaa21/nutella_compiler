@echo off

set CodeDir=..\code
set OutputDir=..\build

set CommonCompilerFlags=-Od -EHsc -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -Z7 -FC
set CommonLinkerFlags=-incremental:no -opt:ref

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS %CodeDir%\nutella_compiler.cpp /link %CommonLinkerFlags%

popd
