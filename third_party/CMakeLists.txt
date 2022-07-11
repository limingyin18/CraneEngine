project(third_party)

# spdlog
add_subdirectory(spdlog)
target_include_directories(spdlog PUBLIC "Log")

# vma
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE "VulkanMemoryAllocator/include")

# eigen
add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE "eigen")

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

# spirv-reflect
add_library(spirv_reflect "SPIRV-Reflect/spirv_reflect.c")
target_include_directories(spirv_reflect PUBLIC "SPIRV-Reflect")

# VkFFT
add_library(VkFFT INTERFACE)
target_include_directories(VkFFT INTERFACE "VkFFT/vkFFT")
target_include_directories(VkFFT INTERFACE "glslang/glslang/Include/")

#tracy 
add_subdirectory(tracy)

#glslang
add_subdirectory(glslang)