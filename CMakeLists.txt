cmake_minimum_required(VERSION 3.24)
project(TextEditor)

set(CMAKE_CXX_STANDARD 23)

# Add debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics -fansi-escape-codes -g")

# Include the SFML headers
include_directories(/opt/homebrew/Cellar/sfml/2.6.2/include/)

# Link the SFML libraries
link_directories(/opt/homebrew/Cellar/sfml/2.6.2/lib/)

# Add the source files
add_executable(TextEditor main.cpp TextBuffer.cpp)

# Link the SFML libraries
target_link_libraries(TextEditor sfml-graphics sfml-window sfml-system)

