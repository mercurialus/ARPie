cmake_minimum_required(VERSION 3.0)
project(ARPie)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for PcapPlusPlus
include_directories(/usr/local/include/pcapplusplus)

# Link directories for PcapPlusPlus libraries
link_directories(/usr/local/lib)

# Define compile definitions (only if required)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

# Specify source files for the ARPieHelper executable
add_executable(ARPieHelper
        CapturePacket.h   # Add corresponding .cpp files here
        ARPieFlooder.h
        main.cpp
        floodie.h
        ListInterfaces.h
        ParsePacket.h
)

# Specify source files for the main ARPie executable
add_executable(${PROJECT_NAME}
        ARPie.cpp
)

# Link the necessary libraries for ARPie
target_link_libraries(${PROJECT_NAME}
        Pcap++
        Packet++
        Common++
        pcap
)

# Link the necessary libraries for ARPieHelper (if required)
target_link_libraries(ARPieHelper
        Pcap++
        Packet++
        Common++
        pcap
)
