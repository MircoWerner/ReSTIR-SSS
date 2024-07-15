### GLFW ###
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")
add_subdirectory(glfw)

### glm ###
add_subdirectory(glm)

### imgui ###
if (WIN32)
    add_library(imgui STATIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
            main.cpp)
else ()
    add_library(imgui SHARED ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
            main.cpp)
endif ()
set_target_properties(imgui PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(imgui
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/glfw/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)
target_link_libraries(imgui Vulkan::Vulkan glfw)
install(TARGETS imgui LIBRARY DESTINATION lib)

### stb ###
add_library(stb SHARED ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.h ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image_write.h main.cpp)
set_target_properties(stb PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS stb LIBRARY DESTINATION lib)

### tinyobjloader ###
add_library(tinyobjloader SHARED ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/tiny_obj_loader.h main.cpp)
set_target_properties(tinyobjloader PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS tinyobjloader LIBRARY DESTINATION lib)

### tinygltf ###
add_library(tinygltf SHARED ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf/tiny_gltf.h ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf/tiny_gltf.cc)
set_target_properties(tinygltf PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS tinygltf LIBRARY DESTINATION lib)

### SPIRV-Reflect ###
add_library(spirv-reflect SHARED ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Reflect/spirv_reflect.h ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Reflect/spirv_reflect.cpp)
set_target_properties(spirv-reflect PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS spirv-reflect LIBRARY DESTINATION lib)

### pugixml ###
add_library(pugixml SHARED ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src/pugiconfig.hpp ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src/pugixml.hpp ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src/pugixml.cpp)
set_target_properties(pugixml PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS pugixml LIBRARY DESTINATION lib)

### argparse ###
add_library(argparse SHARED ${CMAKE_CURRENT_SOURCE_DIR}/argparse/include/argparse/argparse.hpp main.cpp)
set_target_properties(argparse PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS argparse LIBRARY DESTINATION lib)