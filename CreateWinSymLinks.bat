@ECHO OFF
cd %~dp0

mkdir bin
cd bin

mklink base.r ..\base.r

mklink /D textures ..\textures
mklink /D shaders ..\shaders
