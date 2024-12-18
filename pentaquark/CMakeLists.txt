set(PROJECT pentaquark)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h*)

message(DEBUG "SRCS = ${SOURCES}")
message(DEBUG "INCS = ${HEADERS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(${PROJECT} SHARED ${SOURCES} )
target_link_libraries(${PROJECT} PUBLIC ${ROOT_LIBRARIES} roofit)
