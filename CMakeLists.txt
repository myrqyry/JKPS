cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use cmake -B build.")
endif()

project(JKPS CXX)

file(GLOB PROJECT_SOURCE_DIR "Source")
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS "Source/*.cpp")
file(GLOB_RECURSE SRC_HEADERS CONFIGURE_DEPENDS "Headers/*.hpp")

add_executable(${PROJECT_NAME} ${SRC_SOURCES} ${SRC_HEADERS})

option(ENABLE_WERROR "Treat compiler warnings as errors" OFF)

target_compile_options(
	${PROJECT_NAME} PRIVATE
    $<$<BOOL:${ENABLE_WERROR}>:-Werror>
    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Stripping debug symbols from ${PROJECT_NAME}"
)



target_compile_definitions(${PROJECT_NAME} PRIVATE COMPILE_FLAGS)
target_compile_definitions(${PROJECT_NAME} PUBLIC PROGRAM_VERSION="0.3")

find_package(Threads)
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics ${CMAKE_THREAD_LIBS_INIT})
