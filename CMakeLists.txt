cmake_minimum_required(VERSION 3.0.0)
project(MyLinker VERSION 0.1.0)

include(CTest)
enable_testing()


set(Packer_src 
        packer64/main.cpp 
        packer64/ELFImage.cpp
        packer64/packer.cpp
        packer64/ElfUtil.cpp)

add_executable(Packer ${Packer_src})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
