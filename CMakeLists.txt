cmake_minimum_required(VERSION 3.14)
project(RifleRange)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE PROJECT_SOURCES
        CONFIGURE_DEPENDS
        RELATIVE ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/*.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include)

if (EXISTS ${CURRENT_BINARY_DIR}/res)
    file (REMOVE_RECURSE ${CURRENT_BINARY_DIR}/res)
endif()
make_directory(${CMAKE_CURRENT_BINARY_DIR}/res)
file(GLOB_RECURSE resourses
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/res/*)
foreach(resourse ${resourses})
    file(COPY ${resourse} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/res)
endforeach(resourse)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw3/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw3/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/minIni/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/minIni/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/luajit/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/luajit/lib)


include_directories(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/OpenAL32/include)
link_directories(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libs/OpenAL32/bin)
file(COPY ${PROJECT_SOURCE_DIR}/libs/OpenAL32/bin/OpenAL32.dll
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/ogg/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/ogg/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/vorbis/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/vorbis/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/stb_image/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glm/include)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
        OpenAL32
        vorbisenc
        vorbisfile
        vorbis
        ogg
        glad
        glfw3
        opengl32
        gdi32
        minIni
        luajit
        )