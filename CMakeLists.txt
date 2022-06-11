cmake_minimum_required(VERSION 3.5)
project(worden)

include_directories(inc)
file(GLOB SOURCES "src/*.cpp")
add_executable(worden ${SOURCES})

set_target_properties(worden
    PROPERTIES
    CXX_STANDARD 17
    # msys mingw64 libcurl cannot be statistically linked - bummer!
    # LINK_SEARCH_START_STATIC 1
    # LINK_SEARCH_END_STATIC 1
)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

target_link_libraries(worden ${CURL_LIBRARIES})
