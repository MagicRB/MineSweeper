cmake_minimum_required(VERSION 2.8.9)
project(MineSweeper)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(external_include)
include_directories(include_IL)

link_directories(link)

include_directories(F:/Projects/C++/C++_LIBS/glfw-3.2.1.bin.WIN64/include)
include_directories(F:/Projects/C++/C++_LIBS/glm/include)
include_directories(F:/Projects/C++/C++_LIBS/glew-2.1.0/include)
if(WIN32)
	include_directories(include_win32)
	link_directories(link_win32)
endif(WIN32)

file(GLOB SOURCES "src/*.cpp" "src/*.c")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_executable(MineSweeper ${SOURCES})

if(WIN32)
	target_link_libraries(MineSweeper glfw3 glew32 opengl32)
	#add_custom_command(TARGET MineSweeper POST_BUILD COMMAND powershell cp bin/SFML_RPG.exe ../../run)
elseif(UNIX)
	target_link_libraries(MineSweeper glfw3 glew32 opengl32)
	#add_custom_command(TARGET MineSweeper POST_BUILD COMMAND cp bin/SFML_RPG.exe ../../run)
endif(WIN32)