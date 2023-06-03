call .\clean.bat

@REM    ROM+MBC5+RAM+BATT : -Wl-yt0x1B
@REM    4 ROM banks  : -Wl-yo4
@REM    4 RAM banks  : -Wl-ya4

@REM mkdir obj
@REM mkdir build
@REM mkdir lib

@REM @echo off
@REM @REM ENTER YOUR GBDK DIRECTORY IN @set GBDK = ...
@REM @REM -Wl-j .noi bgb debugging -Wm-yS
@REM :: Assemble the hUGEDriver source into an RGBDS object file
@REM rgbasm -DGBDK -o./sound/hUGEDriver.obj -i.. hUGEDriver.asm

@REM :: Convert the RGBDS object file into a GBDK object file
@REM python utilities\rgb2sdas.py -o sound/hUGEDriver.o sound/hUGEDriver.obj

@REM :: Make the library
@REM sdar -ru lib/hUGEDriver.lib obj/hUGEDriver.o

@set GBDK=C:\c_code\gbdk
%GBDK%\bin\lcc.exe -autobank -Iinclude -Wl-llib/hUGEDriver.lib -Wf--debug -Wl-y -Wb-ext=.rel -Wb-v -Wl-yt0x1B -Wl-yoA -Wl-ya4 -o frogger-clone.gb^
 src\*.c res\tiles\*.c sound\*.c

 


