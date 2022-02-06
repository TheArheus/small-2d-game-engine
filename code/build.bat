@echo off
if not defined DevEnvDir (
    call vcvarsall x64
)
REM call vcvarsall x64

set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"

set CommonCompileFlags=-MT -nologo -fp:fast -EHa -O2 -WX- -W4 -Oi -GR- -Gm- -GS -wd4100 -wd4201 -wd4505 -FC -Z7 -I %INC_VCPKG%
set CommonLinkFlags=-opt:ref -incremental:no /NODEFAULTLIB:msvcrt /SUBSYSTEM:CONSOLE /LIBPATH:%LIB_VCPKG%  

if not exist ..\build mkdir ..\build
pushd ..\build

cl %CommonCompileFlags% SDL2main.lib SDL2.lib ..\code\display.cpp -LD /link -opt:ref -incremental:no /LIBPATH:%LIB_VCPKG%
cl %CommonCompileFlags% ..\code\main.cpp SDL2main.lib SDL2.lib display.obj /link %CommonLinkFlags%
popd
