cmake_minimum_required(VERSION 3.5)
project(APCpp-C_rpl C CXX)

add_executable(APCpp-C_rpl
    rpl-stubs.cpp
    wiiu-stderr.cpp
    wiiu-stdout.cpp
)

target_link_libraries(APCpp-C_rpl APCpp-C stdc++)

wut_add_exports(APCpp-C_rpl ${CMAKE_CURRENT_SOURCE_DIR}/exports.def)
wut_create_rpl(APCpp-C_rpl)
