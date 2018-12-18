@echo off
set PATH=%PATH%;%DEVDIR%\Tools\clang\bin

set CONFIG=%1
IF "%1"=="" (
  set CONFIG=Debug
)

if not exist .bin\%CONFIG% (
  mkdir .bin\%CONFIG%
)

rem Warnings
set CXX_FLAGS=-Wall
rem Architecture
set CXX_FLAGS=%CXX_FLAGS% -march=native
rem C++ settings
set CXX_FLAGS=%CXX_FLAGS% -std=c++17
rem Assembly listing
set CXX_FLAGS=%CXX_FLAGS% -masm=intel
rem Windows specific
set CXX_FLAGS=%CXX_FLAGS% -Xclang -flto-visibility-public-std

if "%CONFIG%"=="Debug" (
  set CXX_FLAGS=%CXX_FLAGS% -g -gcodeview -gno-column-info
)
if "%CONFIG%"=="Release" (
  set CXX_FLAGS=%CXX_FLAGS% -O3
)

rem Main sources
set SOURCES=%SOURCES% main.cpp solve.cpp

clang++.exe %CXX_FLAGS% %SOURCES% -o .bin/%CONFIG%/app.exe
