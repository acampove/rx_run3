include("utilities.cmake")

cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_project_vars()
set_flags()

project(${PROJECT} VERSION 1.0)

# Set the runtime path of the libraries by default but allow to switch it off:
option(SET_RPATH "Set runtime path of the ${project} libraries?" ON)

set_run_path()
set_targets()
set_library_names()
set_header_only()

find_gsl()
find_root()
find_yamlcpp()
find_boost()

message(STATUS "Libraries:")
foreach(LIB ${LIBRARIES})
    message(STATUS "   ${LIB}")
    add_subdirectory(${LIB} ${CMAKE_BINARY_DIR}/${LIB})
    include_directories(${LIB})
    include_directories(${LIB}/src ${LIB}/inc)
    list(APPEND REPO_LIBRARIES ${LIB})
endforeach()

print_target_libraries()
make_targets()

print_vars()
print_array(${CMAKE_CXX_FLAGS}  "C++ flags")
print_array(${CMAKE_C_FLAGS}    "C   flags")

