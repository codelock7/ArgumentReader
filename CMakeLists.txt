cmake_minimum_required(VERSION 2.8)

project(ArgumentReader)
set(SOURCES ArgumentReader.cpp main.cpp)
set(HEADERS ArgumentReader.hh)
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
