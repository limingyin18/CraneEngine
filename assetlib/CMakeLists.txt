#set(CMAKE_CXX_STANDARD 17)
# Add source to this project's executable.
add_library (assetlib STATIC 
"mesh_asset.h"
"mesh_asset.cpp"
"skeleton_asset.hpp"
"skeleton_asset.cpp"
"texture_asset.h"
"texture_asset.cpp"
"material_asset.h"
"material_asset.cpp"
"prefab_asset.h"
"prefab_asset.cpp"
"asset_loader.h"
"asset_loader.cpp"
)

target_include_directories(assetlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(assetlib PRIVATE 
                        nlohmann_json
                        lz4)