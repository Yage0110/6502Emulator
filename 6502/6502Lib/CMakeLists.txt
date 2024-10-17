cmake_minimum_required(VERSION 3.7)


project( M6502Lib )

set(CMAKE_CXX_STANDARD 11)  # Or higher
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
	add_compile_options(/MP)				#Use multiple processors when building
	add_compile_options(/W4 /wd4201 /WX)	#Warning level 4, all warnings are errors
else()
	add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif()

set  (M6502_SOURCES
    "src/public/main_6502.hpp"
    "src/private/main_6502.cpp")
		
source_group("src" FILES ${M6502_SOURCES})
		
add_library( M6502Lib ${M6502_SOURCES} )

target_include_directories ( M6502Lib PUBLIC "${PROJECT_SOURCE_DIR}/src/public")
target_include_directories ( M6502Lib PRIVATE "${PROJECT_SOURCE_DIR}/src/private")

#set_target_properties(M6502Lib PROPERTIES FOLDER "M6502Lib")