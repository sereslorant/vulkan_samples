
add_custom_target(
    HardcodedVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o HardcodedVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/HardcodedVertexShader.vert
    SOURCES HardcodedVertexShader.vert
    )
add_dependencies(HardcodedVertexShader.spv glslang)
#add_custom_command(
#	TARGET HardcodedVertexShader.spv
#	COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -invert-y -V -o HardcodedVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/HardcodedVertexShader.vert
#	)

add_custom_target(
    UboHardcodedVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o UboHardcodedVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/UboHardcodedVertexShader.vert
    SOURCES UboHardcodedVertexShader.vert
    )
add_dependencies(UboHardcodedVertexShader.spv glslang)


add_custom_target(
    VboVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o VboVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/VboVertexShader.vert
    SOURCES VboVertexShader.vert
    )
add_dependencies(VboVertexShader.spv glslang)
#add_custom_command(
#	TARGET VboVertexShader.spv
#	COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -invert-y -V -o VboVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/VboVertexShader.vert
#	)


add_custom_target(
    VboUboVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o VboUboVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/VboUboVertexShader.vert
    SOURCES VboUboVertexShader.vert
    )
add_dependencies(VboUboVertexShader.spv glslang)







add_custom_target(
    FragmentShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o FragmentShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/FragmentShader.frag
    SOURCES FragmentShader.frag
    )
add_dependencies(FragmentShader.spv glslang)
#add_custom_command(
#	TARGET FragmentShader.spv
#	COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o FragmentShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/FragmentShader.frag
#	)



add_custom_target(
    TexHardcodedVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o TexHardcodedVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/TexHardcodedVertexShader.vert
    SOURCES TexHardcodedVertexShader.vert
    )
add_dependencies(TexHardcodedVertexShader.spv glslang)


add_custom_target(
    TexVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o TexVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/TexVertexShader.vert
    SOURCES TexVertexShader.vert
    )
add_dependencies(TexVertexShader.spv glslang)

add_custom_target(
    TexFragmentShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o TexFragmentShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/TexFragmentShader.frag
    SOURCES TexFragmentShader.frag
    )
add_dependencies(TexFragmentShader.spv glslang)



add_custom_target(
    MeshVertexShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o MeshVertexShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/MeshVertexShader.vert
    SOURCES MeshVertexShader.vert
    )
add_dependencies(MeshVertexShader.spv glslang)


add_custom_target(
    MeshNrmFragmentShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o MeshNrmFragmentShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/MeshNrmFragmentShader.frag
    SOURCES MeshNrmFragmentShader.frag
    )
add_dependencies(MeshNrmFragmentShader.spv glslang)

add_custom_target(
    MeshTexCoordFragmentShader.spv
    #ALL
    COMMAND ${CMAKE_BINARY_DIR}/bin/glslangValidator -V -o MeshTexCoordFragmentShader.spv ${CMAKE_CURRENT_SOURCE_DIR}/MeshTexCoordFragmentShader.frag
    SOURCES MeshTexCoordFragmentShader.frag
    )
add_dependencies(MeshTexCoordFragmentShader.spv glslang)
