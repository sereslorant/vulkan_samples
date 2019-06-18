
add_executable(instance_device_app ../Main.cpp lvkInstanceDevice.cpp)
add_dependencies(instance_device_app libsdl2)
target_link_libraries(instance_device_app SDL2 vulkan)

add_executable(triangle_app ../Main.cpp lvkTriangle.cpp ../../lFileUtils.cpp)
add_dependencies(triangle_app libsdl2)
target_link_libraries(triangle_app SDL2 vulkan lvkUtils)

add_executable(dynamic_state_app ../Main.cpp lvkDynamicState.cpp ../../lFileUtils.cpp)
add_dependencies(dynamic_state_app libsdl2)
target_link_libraries(dynamic_state_app SDL2 vulkan lvkUtils)

add_executable(vertex_buffer_app ../Main.cpp lvkVertexBuffer.cpp ../../lFileUtils.cpp)
add_dependencies(vertex_buffer_app libsdl2)
target_link_libraries(vertex_buffer_app SDL2 vulkan lvkUtils)

add_executable(index_buffer_app ../Main.cpp lvkIndexBuffer.cpp ../../lFileUtils.cpp)
add_dependencies(index_buffer_app libsdl2)
target_link_libraries(index_buffer_app SDL2 vulkan lvkUtils)

add_executable(vertex_staging_buffer_app ../Main.cpp lvkVertexStagingBuffer.cpp ../../lFileUtils.cpp)
add_dependencies(vertex_staging_buffer_app libsdl2)
target_link_libraries(vertex_staging_buffer_app SDL2 vulkan lvkUtils)

add_executable(uniform_buffer_app ../Main.cpp lvkUniformBuffer.cpp ../../lFileUtils.cpp)
add_dependencies(uniform_buffer_app libsdl2)
target_link_libraries(uniform_buffer_app SDL2 vulkan lvkUtils)

add_executable(texture_array_app ../Main.cpp lvkTextureArray.cpp ../../lFileUtils.cpp)
add_dependencies(texture_array_app libsdl2)
target_link_libraries(texture_array_app SDL2 vulkan lvkUtils)
