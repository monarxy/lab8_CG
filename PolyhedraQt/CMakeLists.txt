cmake_minimum_required(VERSION 3.16)

project(PolyhedraQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)

add_executable(PolyhedraQt
    main.cpp

    geometry/Point3D.h
    geometry/Polygon.h
    geometry/Polyhedron.h
    geometry/Polyhedron.cpp
    geometry/ObjIO.h
    geometry/ObjIO.cpp
    geometry/ModelGenerators.h
    geometry/ModelGenerators.cpp
    geometry/PolyhedronFactory.h
    geometry/PolyhedronFactory.cpp

    math/Matrix4.h
    math/Matrix4.cpp
    math/Transformations.h
    math/Transformations.cpp
    math/Projection.h
    math/Projection.cpp

    render/Camera.h
    render/Camera.cpp
    render/PolyhedronWidget.h
    render/PolyhedronWidget.cpp

    ui/MainWindow.h
    ui/MainWindow.cpp
)

target_link_libraries(PolyhedraQt PRIVATE Qt5::Widgets)

target_include_directories(PolyhedraQt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)
