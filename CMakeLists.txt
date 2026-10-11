cmake_minimum_required(VERSION 3.24)

project(VulkanGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

# Visual Studio's IntelliSense (Ninja / Open Folder) ignores MSVC's
# -external:I flag, which CMake uses for imported targets (Vulkan, GLFW).
# That leaves Vulkan types and macros unresolved in the editor even though the
# build works. Passing them as plain -I fixes the editor.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

# Everything the game needs at runtime (exe, DLLs, shaders) goes in <build>/bin.
# Per-config variables keep the same layout with the Visual Studio generator
# (otherwise it would create bin/Debug, bin/Release).
set(VULKANGE_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${VULKANGE_BIN_DIR}")
foreach(CFG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} "${VULKANGE_BIN_DIR}")
endforeach()

set(VULKANGE_GLFW_ROOT
    "C:/Users/RedLionBordel le BG/Documents/Visual Studio 2022/Libraries/glfw-3.5.1.bin.WIN64/glfw-3.5.1.bin.WIN64"
    CACHE PATH "Root of the prebuilt GLFW distribution")
set(VULKANGE_GLM_ROOT
    "C:/Users/RedLionBordel le BG/Documents/Visual Studio 2022/Libraries/glm"
    CACHE PATH "Root of the GLM source tree")

if(NOT EXISTS "${VULKANGE_GLFW_ROOT}/include/GLFW/glfw3.h")
    message(FATAL_ERROR "GLFW not found at ${VULKANGE_GLFW_ROOT}. Set VULKANGE_GLFW_ROOT.")
endif()
if(NOT EXISTS "${VULKANGE_GLM_ROOT}/glm/glm.hpp")
    message(FATAL_ERROR "GLM not found at ${VULKANGE_GLM_ROOT}. Set VULKANGE_GLM_ROOT.")
endif()

set(VULKANGE_GLFW_IMPLIB "${VULKANGE_GLFW_ROOT}/lib-vc2022/glfw3dll.lib")
set(VULKANGE_GLFW_DLL "${VULKANGE_GLFW_ROOT}/lib-vc2022/glfw3.dll")
if(NOT EXISTS "${VULKANGE_GLFW_IMPLIB}" OR NOT EXISTS "${VULKANGE_GLFW_DLL}")
    message(FATAL_ERROR "GLFW DLL import lib not found under ${VULKANGE_GLFW_ROOT}/lib-vc2022")
endif()

add_library(glfw SHARED IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_IMPLIB "${VULKANGE_GLFW_IMPLIB}"
    IMPORTED_LOCATION "${VULKANGE_GLFW_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${VULKANGE_GLFW_ROOT}/include"
)

set(VULKANGE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/VulkanGE")

file(GLOB_RECURSE VULKANGE_SOURCES CONFIGURE_DEPENDS
    "${VULKANGE_SRC_DIR}/*.cpp"
    "${VULKANGE_SRC_DIR}/*.h"
    "${VULKANGE_SRC_DIR}/*.hpp"
)
file(GLOB_RECURSE VULKANGE_SHADERS CONFIGURE_DEPENDS
    "${VULKANGE_SRC_DIR}/*.vert"
    "${VULKANGE_SRC_DIR}/*.frag"
    "${VULKANGE_SRC_DIR}/*.comp"
    "${VULKANGE_SRC_DIR}/*.geom"
    "${VULKANGE_SRC_DIR}/*.tesc"
    "${VULKANGE_SRC_DIR}/*.tese"
)

list(FILTER VULKANGE_SOURCES EXCLUDE REGEX "[\\/]x64[\\/]")
list(FILTER VULKANGE_SHADERS EXCLUDE REGEX "[\\/]x64[\\/]")

add_executable(VulkanGE ${VULKANGE_SOURCES} ${VULKANGE_SHADERS})
set_source_files_properties(${VULKANGE_SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group(TREE "${VULKANGE_SRC_DIR}" FILES ${VULKANGE_SOURCES} ${VULKANGE_SHADERS})

target_include_directories(VulkanGE PRIVATE
    "${VULKANGE_SRC_DIR}"
    "${VULKANGE_GLM_ROOT}"
)

target_link_libraries(VulkanGE PRIVATE
    Vulkan::Vulkan
    glfw
)

if(WIN32)
    target_link_libraries(VulkanGE PRIVATE user32 gdi32 shell32)
endif()

if(NOT Vulkan_GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Install the Vulkan SDK or add it to PATH.")
endif()

# Compiled straight into bin/ with the same relative layout as the sources
# (e.g. bin/Render/Shader/BaseVertexShader.vert.spv), so no copy step is needed.
set(VULKANGE_SHADER_OUTPUT_DIR "${VULKANGE_BIN_DIR}")
set(VULKANGE_SPV_OUTPUTS)

foreach(SHADER IN LISTS VULKANGE_SHADERS)
    file(RELATIVE_PATH SHADER_REL "${VULKANGE_SRC_DIR}" "${SHADER}")
    set(SPV_FILE "${VULKANGE_SHADER_OUTPUT_DIR}/${SHADER_REL}.spv")
    get_filename_component(SPV_DIR "${SPV_FILE}" DIRECTORY)
    add_custom_command(
        OUTPUT "${SPV_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SPV_DIR}"
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" "${SHADER}" -o "${SPV_FILE}"
        DEPENDS "${SHADER}"
        COMMENT "Compiling shader ${SHADER_REL}"
        VERBATIM
    )
    list(APPEND VULKANGE_SPV_OUTPUTS "${SPV_FILE}")
endforeach()

if(VULKANGE_SPV_OUTPUTS)
    add_custom_target(VulkanGE_Shaders ALL DEPENDS ${VULKANGE_SPV_OUTPUTS})
    add_dependencies(VulkanGE VulkanGE_Shaders)
endif()

add_custom_command(TARGET VulkanGE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VULKANGE_GLFW_DLL}"
        "$<TARGET_FILE_DIR:VulkanGE>"
    COMMENT "Copying glfw3.dll next to VulkanGE.exe"
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT VulkanGE)
set_target_properties(VulkanGE PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${VULKANGE_BIN_DIR}"
)
