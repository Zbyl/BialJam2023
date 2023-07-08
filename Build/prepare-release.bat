@echo off

echo "Win build."
cmake --build VSProject --config Release
IF ERRORLEVEL 1 (
    echo "Win build failed."
    exit /b 1
)

echo "Win zip."
powershell Compress-Archive -Path ..\Runtime\*,VSProject\Release\* -DestinationPath KunekBogusWin.zip -Force
IF ERRORLEVEL 1 (
    echo "Win zip failed."
    exit /b 1
)

echo "Web build."
cmake --build WebProject --config Release
IF ERRORLEVEL 1 (
    echo "Web build failed."
    exit /b 1
)

echo "Web zip."
copy WebProject\RayGame.html WebProject\index.html
IF ERRORLEVEL 1 (
    echo "Copy index failed."
    exit /b 1
)
powershell Compress-Archive -Path WebProject\index.html,WebProject\RayGame.js,WebProject\RayGame.wasm,WebProject\RayGame.data -DestinationPath KunekBogusWeb.zip -Force
IF ERRORLEVEL 1 (
    echo "Web zip failed."
    exit /b 1
)

echo "Done!"
