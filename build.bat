:: DEBUG
 cl /wd4505 /wd4201 /wd4189 /wd4101 /wd4702 /wd4100 /W4 /WX /Zi /D_DEBUG /DEBUG:FULL src/main.c /Fe:"x64/debug-x64.exe" /Fd:"x64/" /Fo:"x64/" opengl32.lib Gdi32.lib  User32.lib
 


:: RELEASE
:: cl /wd4505 /wd4201 /wd4189 /wd4101 /wd4702 /wd4100 /W4 /WX /O2 /GS- /D_RELEASE src/main.c /Fe:"x64/debug-x64.exe" /Fd:"x64/" /Fo:"x64/" /link kernel32.lib Gdi32.lib User32.lib opengl32.lib /subsystem:windows



:: DISABLED WARNINGS
:: /wd4101 warning C4101: unreferenced local variable
:: /wd4702 warning C4702: unreachable code
:: /wd4100 warning C4100: unreferenced formal parameter
:: /wd4189 warning C4189: local variable is initialized but not referenced
:: /wd4201 warning C4201: nonstandard extension used: nameless struct/union
:: RUN
if %ERRORLEVEL% EQU 0 "x64/debug-x64.exe"