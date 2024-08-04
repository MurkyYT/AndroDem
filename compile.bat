@call "VsDevCmd.bat"
cmake src\AndroDem -A Win32 -B "src\AndroDem\build32"
cd  src\AndroDem\build32
msbuild AndroDem.sln  /p:Configuration="Release"
cd..
cmake -A x64 -B "build64"
cd build64
msbuild AndroDem.sln  /p:Configuration="Release"
cd ..\..\Server
cmd /c build_without_gradle.bat
cd ..\..\
ISCC.exe installerx86.iss
ISCC.exe installerx64.iss
tar -caf Output\AndroDem_x86.zip data -C src\AndroDem\build32\Release\ *.exe
tar -caf Output\AndroDem_x64.zip data -C src\AndroDem\build64\Release\ *.exe
echo =====================================================
echo Done
echo =====================================================
pause > nul