::output libclogger


::include

set includedir="%~dp0..\..\libclogger\include\clogger"


::x64

set x64bindir="%~dp0..\..\libclogger\bin\win64"

set x64dbgdir="%~dp0..\..\libclogger\lib\win64\Debug"

set x64relsdir="%~dp0..\..\libclogger\lib\win64\Release"


::x86

set x86bindir="%~dp0..\..\libclogger\bin\win86"

set x86dbgdir="%~dp0..\..\libclogger\lib\win86\Debug"

set x86relsdir="%~dp0..\..\libclogger\lib\win86\Release"


::create libclogger dirs

IF not exist %includedir% (mkdir %includedir%)

IF not exist %x64bindir% (mkdir %x64bindir%)

IF not exist %x64dbgdir% (mkdir %x64dbgdir%)

IF not exist %x64relsdir% (mkdir %x64relsdir%)

IF not exist %x86bindir% (mkdir %x86bindir%)

IF not exist %x86dbgdir% (mkdir %x86dbgdir%)

IF not exist %x86relsdir% (mkdir %x86relsdir%)

