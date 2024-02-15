# CMakeList.txt : CMake project for AndroDem, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_AUTORCC ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("AndroDem")

# Add source to this project's executable.
add_executable (AndroDem WIN32 "config.h" "AndroDem.cpp" "AndroDem.h" "AndroDem.rc" "utils/ADB.cpp" "utils/ADB.h" "utils/StringUtils.cpp" "utils/StringUtils.h" "utils/Logger.cpp" "utils/Logger.h" "utils/RegistrySettings.cpp" "utils/WindowsUtils.h" "utils/WindowsUtils.cpp")
file(COPY "..\\..\\data\\" DESTINATION data)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AndroDem PROPERTY CXX_STANDARD 20)
endif()