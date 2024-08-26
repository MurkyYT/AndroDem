@ECHO OFF
setlocal
set PLATFORM=33
set BUILD_TOOLS=29.0.1


set SERVER_DIR=%CD%
set ANDROID_HOME=%SERVER_DIR%\..\..\Android
set BUILD_DIR=%SERVER_DIR%\build_manual
set CLASSES_DIR=%BUILD_DIR%\classes
set ANDROID_JAR=%ANDROID_HOME%\platforms\android-%PLATFORM%\android.jar

echo Platform: android-%PLATFORM%
echo Build-tools: %BUILD_TOOLS%
echo Build dir: %BUILD_DIR%
echo Android-Home: %ANDROID_HOME%

cd %BUILD_DIR%
rmdir /s /q "%CLASSES_DIR%"
mkdir "%CLASSES_DIR%\com\murky\androdem"

echo Generating java from aidl...
cd "%SERVER_DIR%\src\main\aidl"
"%ANDROID_HOME%\build-tools\%BUILD_TOOLS%\aidl" -o"%CLASSES_DIR%" android\view\IRotationWatcher.aidl
"%ANDROID_HOME%\build-tools\%BUILD_TOOLS%\aidl" -o"%CLASSES_DIR%" android\content\IOnPrimaryClipChangedListener.aidl

echo Compiling java sources...
cd ..\java
"javac.exe" -encoding utf8 -bootclasspath "%ANDROID_JAR%" -cp "%CLASSES_DIR%" -d "%CLASSES_DIR%"  -source 1.8 -target 1.8 com\murky\androdem\\*.java

echo Dexing...
cd "%CLASSES_DIR%"
"%ANDROID_HOME%\build-tools\%BUILD_TOOLS%\d8" --classpath "%ANDROID_JAR%" "%BUILD_DIR%\classes\com\murky\androdem\\*.class" --output "%SERVER_DIR%\..\..\data"