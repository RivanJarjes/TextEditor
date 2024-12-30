cmake_minimum_required(VERSION 3.28)
project(TextEditor)
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

# Add the source files
add_executable(TextEditor main.cpp TextBuffer.cpp)

# Add debug flags
target_compile_features(TextEditor PUBLIC cxx_std_23)
target_compile_options(TextEditor PUBLIC -fcolor-diagnostics -fansi-escape-codes -g)

target_link_libraries(TextEditor PRIVATE SFML::System SFML::Window SFML::Graphics)

