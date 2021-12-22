# assimp
add_subdirectory(assimp)

# json
add_subdirectory(json)

# lz4
set(LZ4_SOURCES
  "lz4/lib/lz4.c"
  "lz4/lib/lz4hc.c"
  "lz4/lib/lz4.h"
  "lz4/lib/lz4hc.h"
  "lz4/lib/lz4frame.c"
  "lz4/lib/lz4frame.h"
  "lz4/lib/xxhash.c")
add_library(lz4 STATIC ${LZ4_SOURCES})
target_include_directories(lz4 PUBLIC "lz4/lib")

# tinyobj
add_subdirectory(tinyobjloader)

# glm
add_subdirectory(glm)

# tinyglft
set(TINYGLTF_HEADER_ONLY ON)
add_subdirectory(tinygltf)