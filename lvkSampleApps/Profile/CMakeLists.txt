
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_executable(profile_0_0_naive_app ../Main.cpp lvkProfileNaive.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_0_naive_app libsdl2)
target_link_libraries(profile_0_0_naive_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_0_1_typical_app ../Main.cpp lvkProfileTypical.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_1_typical_app libsdl2)
target_link_libraries(profile_0_1_typical_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_0_2_typ_hierarchy_app ../Main.cpp lvkProfileTypHierarchy.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_2_typ_hierarchy_app libsdl2)
target_link_libraries(profile_0_2_typ_hierarchy_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_0_3_grouped_app ../Main.cpp lvkProfileGrouped.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_3_grouped_app libsdl2)
target_link_libraries(profile_0_3_grouped_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_0_4_grp_hierarchy_app ../Main.cpp lvkProfileGrpHierarchy.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_4_grp_hierarchy_app libsdl2)
target_link_libraries(profile_0_4_grp_hierarchy_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_0_5_multithreaded_app ../Main.cpp lvkProfileMultithreaded.cpp ../../lFileUtils.cpp)
add_dependencies(profile_0_5_multithreaded_app libsdl2)
target_link_libraries(profile_0_5_multithreaded_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_0_naive_app ../Main.cpp lvkProfileNaive.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_0_naive_app libsdl2)
target_compile_definitions(profile_1_0_naive_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_0_naive_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_1_typical_app ../Main.cpp lvkProfileTypical.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_1_typical_app libsdl2)
target_compile_definitions(profile_1_1_typical_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_1_typical_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_2_typ_hierarchy_app ../Main.cpp lvkProfileTypHierarchy.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_2_typ_hierarchy_app libsdl2)
target_compile_definitions(profile_1_2_typ_hierarchy_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_2_typ_hierarchy_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_3_grouped_app ../Main.cpp lvkProfileGrouped.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_3_grouped_app libsdl2)
target_compile_definitions(profile_1_3_grouped_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_3_grouped_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_4_grp_hierarchy_app ../Main.cpp lvkProfileGrpHierarchy.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_4_grp_hierarchy_app libsdl2)
target_compile_definitions(profile_1_4_grp_hierarchy_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_4_grp_hierarchy_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})

add_executable(profile_1_5_multithreaded_app ../Main.cpp lvkProfileMultithreaded.cpp ../../lFileUtils.cpp)
add_dependencies(profile_1_5_multithreaded_app libsdl2)
target_compile_definitions(profile_1_5_multithreaded_app PRIVATE LVK_PROF_BIG_INPUT=1)
target_link_libraries(profile_1_5_multithreaded_app SDL2 vulkan lvkUtils ${CMAKE_THREAD_LIBS_INIT})
