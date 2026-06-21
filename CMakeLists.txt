cmake_minimum_required(VERSION 3.24)

project(APCpp-C LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if (WIIU)
    set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

include(FetchContent)

if(WIIU)
    message(STATUS "Fetching Wii U APCpp fork")

    FetchContent_Declare(
        APCpp
        GIT_REPOSITORY https://github.com/techmuse8/APCpp-wiiu.git
        GIT_TAG de7ad8c2e907bd66e20984902f4439262c9729ed
    )
else()
    message(STATUS "Using upstream APCpp")

    FetchContent_Declare(
        APCpp
        GIT_REPOSITORY https://github.com/N00byKing/APCpp.git
        GIT_TAG 8970fdcf7f04c5ce72d060e119f18c59574616e9
    )
endif()

FetchContent_MakeAvailable(APCpp)

if(WIIU)
    add_library(APCpp-C STATIC
        APCpp_c.cpp
    )
else()
    add_library(APCpp-C SHARED
        APCpp_c.cpp
    )
endif()

#target_compile_definitions(APCpp-C PUBLIC APC_DYNAMIC_MODE=0)

target_include_directories(APCpp-C PRIVATE
    ${APCpp_SOURCE_DIR}
)

target_link_libraries(APCpp-C PRIVATE APCpp)

if(WIIU)
    add_subdirectory(platform/wiiu)
endif()