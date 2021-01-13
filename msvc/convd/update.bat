@ECHO OFF

if "%1" == "x64_debug" (
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x64\pthreadVC2.dll" "%~dp0target\x64\Debug\"
	copy "%~dp0..\..\libiconv\bin\win64\libcharset-1.dll" "%~dp0target\x64\Debug\"
	copy "%~dp0..\..\libiconv\bin\win64\libiconv-2.dll" "%~dp0target\x64\Debug\"

	copy "%~dp0..\..\src\convd\convd_api.h" "%~dp0..\..\libconvd\include\convd\"

	copy "%~dp0target\x64\Debug\libconvd.lib" "%~dp0..\..\libconvd\lib\win64\Debug\"
	copy "%~dp0target\x64\Debug\libconvd.pdb" "%~dp0..\..\libconvd\lib\win64\Debug\"

    copy "%~dp0target\x64\Debug\*.dll" "%~dp0..\..\libconvd\bin\win64\"
	copy "%~dp0..\..\libiconv\bin\win64\iconv.exe" "%~dp0..\..\libconvd\bin\win64\"
)


if "%1" == "x64_release" (
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x64\pthreadVC2.dll" "%~dp0target\x64\Release\"
	copy "%~dp0..\..\libiconv\bin\win64\libcharset-1.dll" "%~dp0target\x64\Release\"
	copy "%~dp0..\..\libiconv\bin\win64\libiconv-2.dll" "%~dp0target\x64\Release\"

    copy "%~dp0..\..\src\convd\convd_api.h" "%~dp0..\..\libconvd\include\convd\"

	copy "%~dp0target\x64\Release\libconvd.lib" "%~dp0..\..\libconvd\lib\win64\Release\"

	copy "%~dp0target\x64\Release\*.dll" "%~dp0..\..\libconvd\bin\win64\"
	copy "%~dp0..\..\libiconv\bin\win64\iconv.exe" "%~dp0..\..\libconvd\bin\win64\"
)


if "%1" == "x86_debug" (
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x86\pthreadVC2.dll" "%~dp0target\Win32\Debug\"
	copy "%~dp0..\..\libiconv\bin\win86\libintl-8.dll" "%~dp0target\Win32\Debug\"
	copy "%~dp0..\..\libiconv\bin\win86\libcharset-1.dll" "%~dp0target\Win32\Debug\"
	copy "%~dp0..\..\libiconv\bin\win86\libiconv-2.dll" "%~dp0target\Win32\Debug\"

	copy "%~dp0..\..\src\convd\convd_api.h" "%~dp0..\..\libconvd\include\convd\"

	copy "%~dp0target\Win32\Debug\libconvd.lib" "%~dp0..\..\libconvd\lib\win86\Debug\"
	copy "%~dp0target\Win32\Debug\libconvd.pdb" "%~dp0..\..\libconvd\lib\win86\Debug\"

	copy "%~dp0target\Win32\Debug\*.dll" "%~dp0..\..\libconvd\bin\win86\"
	copy "%~dp0..\..\libiconv\bin\win86\iconv.exe" "%~dp0..\..\libconvd\bin\win86\"
)


if "%1" == "x86_release" (
	copy "%~dp0..\..\pthreads-w32\Pre-built.2\dll\x86\pthreadVC2.dll" "%~dp0target\Win32\Release\"
	copy "%~dp0..\..\libiconv\bin\win86\libintl-8.dll" "%~dp0target\Win32\Release\"
	copy "%~dp0..\..\libiconv\bin\win86\libcharset-1.dll" "%~dp0target\Win32\Release\"
	copy "%~dp0..\..\libiconv\bin\win86\libiconv-2.dll" "%~dp0target\Win32\Release\"

	copy "%~dp0..\..\src\convd\convd_api.h" "%~dp0..\..\libconvd\include\convd\"

	copy "%~dp0target\Win32\Release\libconvd.lib" "%~dp0..\..\libconvd\lib\win86\Release\"

	copy "%~dp0target\Win32\Release\*.dll" "%~dp0..\..\libconvd\bin\win86\"
	copy "%~dp0..\..\libiconv\bin\win86\iconv.exe" "%~dp0..\..\libconvd\bin\win86\"
)
