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

echo Simple
nutella_compiler simple.nutella
echo Test1
nutella_compiler test1.nutella
echo Test2
nutella_compiler test2.nutella
echo Test3
nutella_compiler test3.nutella
echo Test4
nutella_compiler test4.nutella
echo Test5
nutella_compiler test5.nutella
echo Test6
nutella_compiler test6.nutella
echo Test7
nutella_compiler test7.nutella
echo Test8
nutella_compiler test8.nutella
echo Test9
nutella_compiler test9.nutella
echo Test10
nutella_compiler test10.nutella
echo Test11
nutella_compiler test11.nutella
echo Test12
nutella_compiler test12.nutella

popd
