cmake_minimum_required(VERSION 2.8.3)
project(ott)

find_package(Eigen3 REQUIRED)
find_package(Boost 1.58 COMPONENTS system serialization REQUIRED)
find_package(pybind11 REQUIRED)

set(Eigen3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
set(PYLIBDIR ${PROJECT_SOURCE_DIR})
message("All Python Library will be in ${PYLIBDIR}")

# detect if on mac or unix
if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(THIRD_DIR ${PROJECT_SOURCE_DIR}/third_party)
    # set up for mosek
    set(MOSEK_DIR ${THIRD_DIR}/mosek/8/tools/platform/linux64x86)
    link_directories(${MOSEK_DIR}/bin)
    include_directories(${MOSEK_DIR}/h)
else (UNIX)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/mosek/8/tools/platform/osx64x86/h)
    link_directories(${PROJECT_SOURCE_DIR}/third_party/mosek/8/tools/platform/osx64x86/bin)    
endif (UNIX)


set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXXFLAGS} -O3 -Wall")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

# libbezier module defines the matrices used in the optimization
pybind11_add_module(bezier MODULE src/bezier_base.cpp src/bezier_wrapper.cpp include/ott/bezier_base.h)
set_target_properties(bezier PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}"
       PREFIX "lib")


include_directories(${EIGEN3_INCLUDE_DIR})
pybind11_add_module(ott MODULE src/pybind_wrapper.cpp src/problem_constructor.cpp
        include/ott/pybind_box_type.h include/ott/data_types.h include/ott/TGProblem.h )
target_link_libraries(ott ${Boost_LIBRARIES})

set_target_properties(ott PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}"
       PREFIX "lib")

#add_subdirectory(src/snopt7)

