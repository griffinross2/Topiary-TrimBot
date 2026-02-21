cmake_minimum_required(VERSION 3.22)

set(VCPKG_TARGET_TRIPLET "x64-windows")
project(USB_Packet_Testing)

find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME})

# LibUSB
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::libusb)

add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PRIVATE inc)

# Add the cobs-c library
add_subdirectory(lib/cobs-c)