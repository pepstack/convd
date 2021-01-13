@ECHO OFF

if "%1" == "x64_debug" (
	copy "%~dp0..\..\src\clogger.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\src\logger_api.h" "%~dp0..\..\libclogger\include\clogger\"

	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x64\pthreadVC2.dll" "%~dp0target\x64\Debug\"

	copy "%~dp0target\x64\Debug\libclogger.lib" "%~dp0..\..\libclogger\lib\win64\Debug\"
	copy "%~dp0target\x64\Debug\libclogger.pdb" "%~dp0..\..\libclogger\lib\win64\Debug\"
	copy "%~dp0target\x64\Debug\*.dll" "%~dp0..\..\libclogger\bin\win64\"
)


if "%1" == "x64_release" (
	copy "%~dp0..\..\src\clogger.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\src\logger_api.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x64\pthreadVC2.dll" "%~dp0target\x64\Release\"

	copy "%~dp0target\x64\Release\pthreadVC2.dll" "%~dp0..\..\libclogger\bin\win64\"
	copy "%~dp0target\x64\Release\libclogger.lib" "%~dp0..\..\libclogger\lib\win64\Release\"
)


if "%1" == "x86_debug" (
	copy "%~dp0..\..\src\clogger.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\src\logger_api.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x86\pthreadVC2.dll" "%~dp0target\Win32\Debug\"

	copy "%~dp0target\Win32\Debug\pthreadVC2.dll" "%~dp0..\..\libclogger\bin\win86\"
	copy "%~dp0target\Win32\Debug\libclogger.lib" "%~dp0..\..\libclogger\lib\win86\Debug\"
	copy "%~dp0target\Win32\Debug\libclogger.pdb" "%~dp0..\..\libclogger\lib\win86\Debug\"
)


if "%1" == "x86_release" (
	copy "%~dp0..\..\src\clogger.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\src\logger_api.h" "%~dp0..\..\libclogger\include\clogger\"
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x86\pthreadVC2.dll" "%~dp0target\Win32\Release\"

	copy "%~dp0target\Win32\Release\pthreadVC2.dll" "%~dp0..\..\libclogger\bin\win86\"
	copy "%~dp0target\Win32\Release\libclogger.lib" "%~dp0..\..\libclogger\lib\win86\Release\"
)
