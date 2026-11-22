cmake_minimum_required(VERSION 3.10)
project(ARPie CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project headers.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include/link directories for PcapPlusPlus.
# Covers Intel-mac / Linux Homebrew (/usr/local) and Apple-silicon Homebrew (/opt/homebrew).
include_directories(
        /usr/local/include/pcapplusplus
        /opt/homebrew/include/pcapplusplus
)
link_directories(
        /usr/local/lib
        /opt/homebrew/lib
)

# Libraries both executables link against.
set(ARPIE_LIBS Pcap++ Packet++ Common++ pcap)

# On macOS, PcapPlusPlus pulls in CoreFoundation / SystemConfiguration
# (e.g. for DNS-server discovery), so those frameworks must be linked too.
if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(SYSTEM_CONFIGURATION SystemConfiguration)
    list(APPEND ARPIE_LIBS ${CORE_FOUNDATION} ${SYSTEM_CONFIGURATION})
endif()

# Specify source files for the ARPieHelper executable
add_executable(ARPieHelper
        src/main.cpp
)

# Specify source files for the main ARPie executable
add_executable(${PROJECT_NAME}
        src/ARPie.cpp
)

target_link_libraries(${PROJECT_NAME} ${ARPIE_LIBS})
target_link_libraries(ARPieHelper ${ARPIE_LIBS})
