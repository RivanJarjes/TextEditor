cmake_minimum_required(VERSION 3.28)
project(TextEditor)
find_package(SFML 3 COMPONENTS Graphics REQUIRED)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")

add_executable(TextEditor main.cpp textBuffer.cpp editorUI.cpp history.cpp FileDialog.mm)

add_subdirectory(clip)
target_compile_features(TextEditor PUBLIC cxx_std_23)
target_compile_options(TextEditor PUBLIC -fcolor-diagnostics -fansi-escape-codes -g)
target_link_libraries(TextEditor PRIVATE SFML::System SFML::Window SFML::Graphics clip)
target_include_directories(TextEditor PRIVATE ${workspaceFolder}/Resources)

set(FONT_FILE "${CMAKE_SOURCE_DIR}/Resources/FiraCode.ttf")

add_custom_command(
    TARGET TextEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:TextEditor>/Resources"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FONT_FILE}"
        "$<TARGET_FILE_DIR:TextEditor>/Resources"
    COMMENT "Copying FiraCode.ttf to output directory in a Resources folder"
)
