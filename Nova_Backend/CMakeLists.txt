cmake_minimum_required(VERSION 3.16)

project(NovaBackend VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    "C:/msys64/mingw64/include" 
)

# Link directories (for sqlite3.lib/.a)
link_directories(
    "C:/msys64/mingw64/lib"
)

# All source files
set(SOURCES
    src/Core/NeuralNet.cpp
    src/Humanizer/ResponseVariator.cpp
    src/Humanizer/ResponseSelector.cpp
    src/Humanizer/ContextTracker.cpp
    src/Humanizer/TopicExtractor.cpp
    src/utils.cpp
    src/Controller.cpp
)

add_library(NovaBackend STATIC ${SOURCES})

# Link sqlite3
target_link_libraries(NovaBackend PRIVATE sqlite3)

# Optional: export include path
target_include_directories(NovaBackend PUBLIC ${CMAKE_SOURCE_DIR}/include) 

# Optional: add compile definitions if needed
# target_compile_definitions(NovaBackend PRIVATE SOME_DEFINE=1)