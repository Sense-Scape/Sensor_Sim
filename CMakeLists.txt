# Min CMake version
cmake_minimum_required(VERSION 3.18)
project(ProcessingServerProject CXX)

# Set directory vars
set(INCLUDE_DIR "include")

message(STATUS "Building for Unix")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# For audio interface
find_library(ALSA_LIBRARY asound)

# Add an executable target
add_executable(Sensor_Sim Sensor_Sim/main.cpp)

add_subdirectory(Sensor_Sim/components/Generic_Modules Generic_Modules)
add_subdirectory(Sensor_Sim/components/Linux_Modules Linux_Modules)

# Include directories for main code and tests
target_include_directories(Sensor_Sim
    PRIVATE ${INCLUDE_DIR}
    PRIVATE ${INCLUDE_TESTS_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Linux_Modules/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/components/Chunk_Types/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/components/Chunk_Types/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/components/Chunk_Types/components/Nlohmann_JSON/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/components/Chunk_Types/components/Nlohmann_JSON/include/nlohmann
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/Base_Module/components/plog/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Sensor_Sim/components/Generic_Modules/components/kissfft
)

target_link_libraries(Sensor_Sim PRIVATE GenericModuleLib LinuxModulesLib BaseModuleLib ChunkTypesLib kissfft::kissfft asound)
