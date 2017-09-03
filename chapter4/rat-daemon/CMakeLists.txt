cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME molerats_rfid)

project(${NAME})

include_directories("include")

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Find boost - used in the mysql bit
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++11)

file(GLOB SOURCE src/*.cc )

ADD_EXECUTABLE(molerats_rfid ${SOURCE})

FIND_LIBRARY(MYSQL_LIBRARY NAMES "libmysqlcppconn-static.a" PATHS "${CMAKE_SOURCE_DIR}/lib" NO_DEFAULT_PATH)
target_link_libraries(molerats_rfid ${MYSQL_LIBRARY})

