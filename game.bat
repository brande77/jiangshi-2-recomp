@echo off
setlocal
pushd "%~dp0" || exit /b 1
if not exist "build\Kyonshiizu2Recomp.exe" (
    echo Build the project first. Expected build\Kyonshiizu2Recomp.exe
    pause
    popd
    exit /b 1
)
if not exist "roms\game.nes" (
    echo Supply your supported ROM at roms\game.nes
    pause
    popd
    exit /b 1
)
start "" /D "%CD%" "%CD%\build\Kyonshiizu2Recomp.exe" "%CD%\roms\game.nes" --scale 3
popd
endlocal
