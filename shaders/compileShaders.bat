@echo off
setlocal enabledelayedexpansion

set GLSLC=C:/VulkanSDK/1.4.328.1/Bin/glslc.exe
set SHADER_ROOT=%~dp0
set OUTPUT_ROOT=%SHADER_ROOT%compiled\

for /r "%SHADER_ROOT%" %%f in (*.vert *.frag) do (

    set "full=%%f"
    set "rel=!full:%SHADER_ROOT%=!"
    set "out=%OUTPUT_ROOT%!rel!.spv"

    REM Get output directory only
    for %%d in ("!out!") do set "outdir=%%~dpd"

    if not exist "!outdir!" (
        mkdir "!outdir!"
    )

    %GLSLC% "%%f" -I "%SHADER_ROOT%include" -o "!outdir!%%~nf.spv"
)

pause