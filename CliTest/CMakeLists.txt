
add_subdirectory(TestMesh)
add_subdirectory(TestMeshGroup)

add_subdirectory(TestImage)
add_subdirectory(TestImageGroup)

add_executable(test_buffer_suballocator       TestBufferSuballocator.cpp)
add_executable(test_image_suballocator        TestImageSuballocator.cpp)

add_executable(test_mesh_loader       TestMeshLoader.cpp)
add_executable(test_mesh_group_loader TestMeshGroupLoader.cpp)

add_executable(test_image_loader       TestImageLoader.cpp)
