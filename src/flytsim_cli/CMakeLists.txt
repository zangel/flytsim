cmake_minimum_required(VERSION 2.8)

project(flytsim_cli)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS system filesystem thread program_options REQUIRED)

add_definitions(-D_WIN32_WINNT=0x0600)

include_directories(
	include
	${Boost_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

#define flytsim_cli library
add_library(flytsim_cli
	STATIC
	Config.hpp
	Base32.hpp				Base32.cpp
	Service.hpp				Service.cpp
	Commands.hpp			Commands.cpp
	Connection.hpp			Connection.cpp
	Response.hpp			Response.cpp
	ResponseParser.hpp		ResponseParser.cpp
	Image.hpp				Image.cpp
)
target_link_libraries(flytsim_cli ${BoostLIBRARIES})


set(BUILD_EXAMPLE ON CACHE BOOL "Build flytsim_cli Qt Example")

if(BUILD_EXAMPLE)

	add_subdirectory(example)

endif()
