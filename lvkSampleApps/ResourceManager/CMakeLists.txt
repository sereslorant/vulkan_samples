
add_executable(resource_pool_app ../Main.cpp lvkResourcePool.cpp ../../lFileUtils.cpp)
add_dependencies(resource_pool_app libsdl2)
target_link_libraries(resource_pool_app SDL2 vulkan lvkUtils)

add_executable(resource_manager_app ../Main.cpp lvkResourceManager.cpp ../../lFileUtils.cpp)
add_dependencies(resource_manager_app libsdl2)
target_link_libraries(resource_manager_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(file_loader_app ../Main.cpp lvkFileLoader.cpp ../../lFileUtils.cpp)
add_dependencies(file_loader_app libsdl2)
target_link_libraries(file_loader_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})
