cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

project(Engine VERSION 0.1 LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/sources/*.cpp)

file(GLOB SHADERS ${PROJECT_SOURCE_DIR}/shaders/compiled/*.spv)
foreach(SHADER ${SHADERS})
    file(COPY ${SHADER} DESTINATION ${CMAKE_BINARY_DIR}/shaders)
endforeach(SHADER ${SHADERS})

file(GLOB TEXTURES ${PROJECT_SOURCE_DIR}/textures/*)
foreach(TEXTURE ${TEXTURES})
    file(COPY ${TEXTURE} DESTINATION ${CMAKE_BINARY_DIR}/textures)
endforeach(TEXTURE ${TEXTURES})

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)
