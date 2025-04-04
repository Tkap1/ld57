@echo off

cls

SETLOCAL ENABLEDELAYEDEXPANSION

set comp=
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\SDL_mixer\include"
set comp=!comp! -I"..\..\libs"
set comp=!comp! -lSDL2_mixer
set comp=!comp! -lopenal
set comp=!comp! -lwebsocket.js
set comp=!comp! -lidbfs.js
set comp=!comp! -sSTACK_SIZE=1048576
set comp=!comp! --shell-file ../shell.html
set comp=!comp! -sFETCH
set comp=!comp! -Dm_emscripten

set debug=1
if !debug!==0 (
	set comp=!comp! -O3
) else (
	@REM set comp=!comp! -Dm_debug
	set comp=!comp! -O3
	set comp=!comp! -sSAFE_HEAP=1
	set comp=!comp! -sASSERTIONS=1
	set comp=!comp! --preload-file ../shaders@shaders
	set comp=!comp! --preload-file ../assets@assets
	set comp=!comp! --preload-file ../src/shader_shared.h@src/shader_shared.h
	@REM set comp=!comp! -gsource-map
	@REM set comp=!comp! -fsanitize=address
)

@REM -sFULL_ES3
pushd build
	call emcc ..\src\platform.cpp ..\src\game.cpp -Dm_sdl -sFULL_ES3 !comp! -std=c++20 -Wno-writable-strings -sUSE_SDL=2 -sUSE_WEBGL2=1 -sALLOW_MEMORY_GROWTH -o index.html -I"C:\Users\34687\Desktop\Dev\C\emsdk\upstream\emscripten\cache\sysroot\include"
popd

copy build\index.html index.html > NUL
copy build\index.js index.js > NUL
copy build\index.wasm index.wasm > NUL
copy build\index.data index.data > NUL
copy build\index.wasm.map index.wasm.map > NUL