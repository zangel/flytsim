project(example)

cmake_policy(SET CMP0020 NEW)

find_package(Qt5Widgets)
find_package(Qt5PrintSupport)

set(QT_USE_QMAIN true)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(example_headers
	Config.hpp
	Application.hpp
	MainWindow.hpp
	DroneView.hpp
	CameraView.hpp
)

set(example_sources
	Application.cpp
	MainWindow.cpp
	DroneView.cpp
	CameraView.cpp
	main.cpp
)
	
set(example_ui_files
	MainWindow.ui
	DroneView.ui
)

set(example_ui_files_resource_files
    Resources.qrc
)

qt5_wrap_ui(example_generated_ui_headers ${example_ui_files})
qt5_add_resources(example_generated_resources ${example_ui_files_resource_files})

add_executable(example WIN32

    ${example_headers}
    ${example_sources}
    ${example_ui_files}
    ${example_generated_ui_headers}
    ${example_generated_resources}
)

target_link_libraries(example
	Qt5::Widgets Qt5::PrintSupport
	flytsim_cli
	${BoostLIBRARIES}
	opengl32
)

install(TARGETS example DESTINATION example)
