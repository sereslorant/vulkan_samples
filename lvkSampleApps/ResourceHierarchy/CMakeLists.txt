
add_executable(mesh_group_app ../Main.cpp lvkMeshGroup.cpp ../../lFileUtils.cpp)
add_dependencies(mesh_group_app libsdl2)
target_link_libraries(mesh_group_app SDL2 vulkan lvkUtils)

add_executable(descriptor_app ../Main.cpp lvkDescriptor.cpp ../../lFileUtils.cpp)
add_dependencies(descriptor_app libsdl2)
target_link_libraries(descriptor_app SDL2 vulkan lvkUtils)

add_executable(resource_hierarchy_app ../Main.cpp lvkResourceHierarchy.cpp ../../lFileUtils.cpp)
add_dependencies(resource_hierarchy_app libsdl2)
target_link_libraries(resource_hierarchy_app SDL2 vulkan lvkUtils)
