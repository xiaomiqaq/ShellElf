cmake_minimum_required(VERSION 3.0.0)
project(MyLinker VERSION 0.1.0)

include(CTest)
enable_testing()

set(Tests_src Tests/Test.c)
set(Packer_src 
        packer/main.cpp 
        packer/ELFImage.cpp)

add_executable(MyPacker ${Packer_src})
add_executable(TestProgram ${Tests_src})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
