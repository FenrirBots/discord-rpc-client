echo off
setlocal
set root=%cd%
set srcd=%root%\src
set intd=%root%\build\int
set outd=%root%\build\bin
set libd=%root%\build\bin

set in=-I inc -I vendor/discord-rpc/inc -I vendor/parson/inc
set cc=-c -Wmain -Wimplicit -Wparentheses -Wmissing-braces -Wformat -Wcomment ^
          -Wchar-subscripts -Wsequence-point -Wreturn-type -Wunused -Wuninitialized --std=c99
set ld=

if ["%1"]==["debug"] (
  goto :debug
)

if ["%1"]==["release"] (
  goto :release
)

goto :ship

:debug
  set intd=%intd%\debug
  set cc=%cc% -pg -gdwarf -fno-pie -O0
  set ld=%ld% -pg
  goto :build
:release
  set intd=%intd%\release
  set cc=%cc% -pg -gdwarf -fno-pie -O2
  set ld=%ld% -pg
  goto :build
:ship
  set intd=%intd%\ship
  set cc=%cc% -fno-pie -O2
  set ld=%ld%

:build
if not exist %outd% (
  mkdir %outd%
)
if not exist %outd%\licenses (
  mkdir %outd%\licenses
)

copy "vendor\discord-rpc\license" "%outd%\licenses\discord-rpc"
copy "vendor\parson\license" "%outd%\licenses\parson"

if not exist %intd% (
  mkdir %intd%
)

cd "vendor\discord-rpc"
call "build.cmd" %*
cd "..\..\"
copy "vendor\discord-rpc\build\bin\*" "build\bin"
cd "vendor\parson"
call "build.cmd" %*
cd "..\..\"
copy "vendor\parson\build\bin\*" "build\bin"

call gcc.exe %cc% %in% %srcd%\entrypoint.c ^
                    -o %intd%\entrypoint.o

call gcc.exe %cc% %in% %srcd%\config.c ^
                    -o %intd%\config.o

call g++.exe %ld% -o %outd%\discord-rpc-client.exe %intd%\* -L %libd% -l discord-rpc -l json