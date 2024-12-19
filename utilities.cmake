# ------------------------------------------------
function(make_targets)
    message(STATUS "Targets:")
    foreach(SRC ${CPPSRCS})
        message(STATUS "   ${SRC}")

        get_filename_component(BARENAME ${SRC} NAME)
        string(REPLACE ".cpp" ".out" OUTNAME ${BARENAME})

        add_executable(${OUTNAME} ${SRC})
        target_link_libraries(${OUTNAME} PUBLIC ${PKG_LIBRARIES} ${REPO_LIBRARIES})
    endforeach()
endfunction()
# ------------------------------------------------
function(set_targets)
    message(DEBUG  "")
    message(DEBUG  "Detecting ${TARGETS}")

    file(GLOB LOCAL_CPPSRCS ${LOCAL_CPPSRCS} "targets/*.cpp")
    set(CPPSRCS ${LOCAL_CPPSRCS} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(set_library_names)
    set(LOCAL_LIBRARIES "roofit velo kernel tuples efficiencies fitter toys")
    separate_arguments(LOCAL_LIBRARIES)

    set(LIBRARIES ${LOCAL_LIBRARIES} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(set_header_only)
    set(HOPKG "pyloops")
    separate_arguments(HOPKG)

    message(STATUS "Adding header only dependencies")
    foreach(PKG ${HOPKG})
        message(STATUS "   ${PKG}")
        include_directories(${PKG})
    endforeach()
endfunction()
# ------------------------------------------------
function(set_project_vars)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS              1 PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE                  RelWithDebInfo PARENT_SCOPE) 
    set(PROJECT                        ewp-rkst-analysis PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(print_vars)
    message(STATUS "Project                "               ${PROJECT})
    message(STATUS "CMAKE_CXX_COMPILER_ID  " ${CMAKE_CXX_COMPILER_ID})
    message(STATUS "")
endfunction()
# ------------------------------------------------
function(set_flags)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DDROP_CGAL -Wno-return-type")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wno-deprecated-declarations")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wimplicit-fallthrough")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wimplicit-fallthrough")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow=global")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wshadow=global")
endfunction()
# ------------------------------------------------
function(print_array VALUES KIND)
    message(STATUS "${KIND}:")
    separate_arguments(VALUES)
    foreach(VALUE ${VALUES})
        message(STATUS "    ${VALUE}")
    endforeach()
endfunction()
# ------------------------------------------------
function(set_run_path)
    if( NOT SET_RPATH)
        return()
    endif()

    # Set up the RPATH so executables find the libraries even when installed in non-default location
    set(CMAKE_MACOSX_RPATH                                         1 PARENT_SCOPE)
    set(CMAKE_SKIP_BUILD_RPATH                                 FALSE PARENT_SCOPE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH                         FALSE PARENT_SCOPE) 
    set(CMAKE_INSTALL_RPATH            "${CMAKE_INSTALL_PREFIX}/lib" PARENT_SCOPE)

    # Add the automatically determined parts of the RPATH which point to directories outside
    # the build tree to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH                       TRUE PARENT_SCOPE)

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    if (${isSystemDir} EQUAL -1)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" PARENT_SCOPE)
    endif()
endfunction()
# ------------------------------------------------
# ------------------------------------------------
function(find_gsl)
    message(DEBUG  "")
    message(DEBUG  "Detecting GSL")

    find_package(GSL REQUIRED)
    if(NOT GSL_FOUND)
        message(FATAL_ERROR "GSL found ${GSL_FOUND}")
    endif()

    message(STATUS "GSL_VERSION   = ${GSL_VERSION}")
    message(DEBUG  "GSL_INCLUDES  = ${GSL_INCLUDE_DIRS}")
    message(DEBUG  "GSL_LIBRARIES = ${GSL_LIBRARIES}")
    include_directories(${GSL_INCLUDE_DIRS})
    list(APPEND PKG_LIBRARIES ${GSL_LIBRARIES})
    set(PKG_LIBRARIES ${PKG_LIBRARIES} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(find_root)
    message(DEBUG  "")
    message(DEBUG  "Detecting ROOT")

    find_package(ROOT REQUIRED COMPONENTS Cling TreePlayer Tree Rint MathMore MathCore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA XMLIO MLP ROOTDataFrame vdt fftw3)
    if (NOT ROOT_FOUND)
        message(FATAL_ERROR "ROOT found ${ROOT_FOUND}")
    endif()

    message(STATUS "ROOT_VERSION   = ${ROOT_VERSION}")
    message(DEBUG  "ROOT_INCLUDES  = ${ROOT_INCLUDE_DIRS}")
    message(DEBUG  "ROOT_LIBRARIES = ${ROOT_LIBRARIES}")
    include_directories(${ROOT_INCLUDE_DIRS})
    link_directories(${ROOT_LIBRARY_DIR})
    include(${ROOT_USE_FILE})
    list(APPEND ROOT_LIBRARIES ROOT::ROOTDataFrame)
    list(APPEND ROOT_LIBRARIES vdt)
    list(APPEND PKG_LIBRARIES ${ROOT_LIBRARIES})
    set(PKG_LIBRARIES ${PKG_LIBRARIES} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(find_yamlcpp)
    message(DEBUG  "")
    message(DEBUG  "Detecting YAML_CPP")

    find_package(YAML-CPP REQUIRED yaml-cpp)
    if (NOT YAML-CPP_FOUND)
        message(FATAL_ERROR "YAML-CPP found ${YAML-CPP_FOUND}")
    endif()

    message(STATUS "YAML-CPP_VERSION   = ${YAML_CPP_VERSION}")
    message(DEBUG  "YAML-CPP_INCLUDES  = ${YAML_CPP_INCLUDE_DIR}")
    message(DEBUG  "YAML-CPP_LIBRARIES = ${YAML_CPP_LIBRARIES}")
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(YAMLCPP_LIBRARIES "${YAMLCPP_INCLUDE_DIR}/libyaml-cpp.dylib" PARENT_SCOPE)
    else()
        set(YAMLCPP_LIBRARIES "${YAMLCPP_INCLUDE_DIR}/libyaml-cpp.so"    PARENT_SCOPE)
    endif()

    string(REGEX REPLACE "include" "lib" YAMLCPP_LIBRARIES ${YAML_CPP_LIBRARIES})
    include_directories(${YAML_CPP_INCLUDE_DIR})
    list(APPEND PKG_LIBRARIES ${YAML_CPP_LIBRARIES})
    set(PKG_LIBRARIES ${PKG_LIBRARIES} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
function(find_boost)
    message(DEBUG  "")
    message(DEBUG  "Detecting BOOST")

    find_package(Boost COMPONENTS system filesystem) 
    if (NOT Boost_FOUND)
        message(FATAL_ERROR "BOOST found ${Boost_FOUND}")
    endif()

    message(STATUS "BOOST_VERSION   = ${Boost_VERSION}")
    message(DEBUG  "BOOST_INCLUDES  = ${Boost_INCLUDE_DIRS}")
    message(DEBUG  "BOOST_LIBRARIES = ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS}) 
    list(APPEND PKG_LIBRARIES ${Boost_LIBRARIES})
    set(PKG_LIBRARIES ${PKG_LIBRARIES} PARENT_SCOPE)
endfunction()
# ------------------------------------------------
# ------------------------------------------------
