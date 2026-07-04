# =============================================================================
# FakeClient Android - native (NDK) build.
#
# Reuses Solstice's ImGui ClickGUI/HUD code (cpp/reused, verbatim) and the small
# FakeClient glue (cpp/solstice_src), hosting them on EGL + OpenGL ES 3.0 instead
# of D3D11. Produces libfakeclient.so, loaded by the Kotlin OverlayService.
# =============================================================================
cmake_minimum_required(VERSION 3.22.1)
project(fakeclient CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(PROJ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
set(TP "${PROJ_ROOT}/thirdparty")

# Solstice's vendored ImGui (and some reused code) use the MSVC-only keyword
# __forceinline. Map it to plain inline for the clang/NDK toolchain.
add_compile_definitions(__forceinline=inline)

# ---- third-party (same versions Solstice uses; CPM fetches on first configure)
include(${PROJ_ROOT}/cmake/CPM.cmake)
CPMAddPackage(NAME glm           GITHUB_REPOSITORY g-truc/glm    GIT_TAG 1.0.1)
CPMAddPackage(NAME nlohmann_json GITHUB_REPOSITORY nlohmann/json VERSION 3.11.3)
CPMAddPackage(NAME spdlog        GITHUB_REPOSITORY gabime/spdlog VERSION 1.14.1)

# ---- Dear ImGui (core + OpenGL ES3 backend) ---------------------------------
add_library(fc_imgui STATIC
    ${TP}/ImGui/imgui.cpp
    ${TP}/ImGui/imgui_draw.cpp
    ${TP}/ImGui/imgui_tables.cpp
    ${TP}/ImGui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(fc_imgui PUBLIC
    ${TP}/ImGui
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${TP})                      # glm/spdlog headers are added by their targets
# The vendored imgui.h includes <glm/glm.hpp>; imgui.cpp includes <spdlog/...>.
target_link_libraries(fc_imgui PUBLIC glm spdlog GLESv3 EGL)
# Use the GLES3 path of imgui_impl_opengl3 (skips the desktop gl3w loader).
target_compile_definitions(fc_imgui PUBLIC IMGUI_IMPL_OPENGL_ES3)
# imgui's own TUs self-define the math operators; expose to consumers only.
target_compile_definitions(fc_imgui INTERFACE IMGUI_DEFINE_MATH_OPERATORS)

# ---- FakeClient native library ----------------------------------------------
file(GLOB_RECURSE FC_REUSED CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/reused/*.cpp)

add_library(fakeclient SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/native-lib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AndroidRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs_Game_android.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src/FeatureManagerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src/ModuleManagerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src/ModuleRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src/stubs/Stubs_Utils.cpp
    ${FC_REUSED}
)

target_include_directories(fakeclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shim         # MUST be first: Windows.h / d3d11.h shims
    ${CMAKE_CURRENT_SOURCE_DIR}/reused       # SDK/... shadow stubs + the GUI code
    ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src # pch.hpp, FakeInput.hpp, Wiring.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}              # native headers
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${TP}/ImGui
    ${TP}                                    # entt
    ${TP}/nes/include                        # nes/event_dispatcher.hpp
)

# Force-include the (cross-platform) pch into every FakeClient TU, like the
# desktop build's /FI. The pch's include guard makes explicit re-includes no-ops.
target_compile_options(fakeclient PRIVATE
    -include ${CMAKE_CURRENT_SOURCE_DIR}/solstice_src/pch.hpp)

target_link_libraries(fakeclient PRIVATE
    fc_imgui glm nlohmann_json spdlog
    android log EGL GLESv3)
