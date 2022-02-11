@echo off
if not defined DevEnvDir (
    call vcvarsall x64
)
REM call vcvarsall x64

set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"
set INC_IMGUI="..\code\imgui"
set SRC_IMGUI=..\code\imgui\imgui*.cpp
set OBJ_IMGUI=imgui.obj imgui_demo.obj imgui_draw.obj imgui_impl_sdl.obj imgui_impl_sdlrenderer.obj imgui_tables.obj imgui_widgets.obj 

set CommonCompileFlags=-MT -nologo -fp:fast -EHa -Od -WX- -W4 -Oi -GR- -Gm- -GS -wd4100 -wd4201 -wd4505 -FC -Z7 /I%INC_VCPKG% /I%INV_IMGUI%
set CommonLinkFlags=-opt:ref -incremental:no /NODEFAULTLIB:msvcrt /SUBSYSTEM:CONSOLE /LIBPATH:%LIB_VCPKG%  

if not exist ..\build mkdir ..\build
pushd ..\build

REM cl %CommonCompileFlags% -O2 %SRC_IMGUI% SDL2main.lib SDL2.lib -LD /link %CommonLinkFlags%
REM cl %CommonCompileFlags% -O2 SDL2main.lib SDL2.lib %OBJ_IMGUI% ..\code\display.cpp -LD /link -opt:ref -incremental:no /LIBPATH:%LIB_VCPKG%
cl %CommonCompileFlags% ..\code\main.cpp %OBJ_IMGUI% SDL2main.lib SDL2.lib display.obj /link %CommonLinkFlags%
popd
