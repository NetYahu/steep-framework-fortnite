#pragma once

#include <deps/imgui/imgui.h>

// bone + draw primitives ported from FortniteESPBase (source/visuals/skeleton.cc).
namespace unreal {
    class USkeletalMeshComponent;
}

namespace cache {
    class c_camera_cache;
}

namespace visuals {
    void draw_player(
        const cache::c_camera_cache& camera,
        unreal::USkeletalMeshComponent* mesh,
        ImDrawList* draw_list,
        float width,
        float height,
        ImU32 color,
        bool draw_skeleton,
        bool draw_boxes );
}