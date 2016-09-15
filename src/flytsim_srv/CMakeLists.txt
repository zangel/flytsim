cmake_minimum_required(VERSION 2.8.3)
project(flytsim_srv)

find_package(catkin REQUIRED COMPONENTS roslib roscpp rosconsole image_transport)
find_package(Boost COMPONENTS system filesystem thread coroutine context REQUIRED)

set(FLYTSIM_CORE_INCLUDE_DIR "/flyt/flytos/flytcore/include" CACHE PATH "Path to FlytSim core include dirs")

include_directories(
    include
    ${catkin_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${FLYTSIM_CORE_INCLUDE_DIR}
)

add_definitions(-std=gnu++0x)

add_executable(flytsim_srv
	main.cpp
	Server.hpp			Server.cpp
	Connection.hpp		Connection.cpp
	Commands.hpp		Commands.cpp
	CommandsParser.hpp	CommandsParser.cpp
	Base32.hpp			Base32.cpp
)

target_link_libraries(flytsim_srv ${catkin_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS flytsim_srv DESTINATION flytsim_srv)
