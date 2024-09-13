# CMakeList.txt : CMake project for raylib-flecs, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("raylib-flecs")
set (CMAKE_BUILD_TYPE RELEASE)

add_subdirectory("src")
file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})




include(FetchContent)

set(RAYLIB_VERSION 5.0)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${RAYLIB_VERSION} EXACT
)

set(FLECS_VERSION 4.0.1)
FetchContent_Declare(
    flecs
    URL https://github.com/SanderMertens/flecs/archive/refs/tags/v${FLECS_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${FLECS_VERSION} EXACT
    )
    
FetchContent_MakeAvailable(raylib flecs)

target_link_libraries(${PROJECT_NAME} PUBLIC raylib flecs::flecs_static)




if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET raylib-flecs PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

