#pragma once

#include <deps/imgui/imgui.h>

// per-frame player render entry + shared toggle state.
namespace cache {
    class c_player_cache;
    class c_camera_cache;
}

namespace visuals {
    inline bool boxes = true;
    inline bool skeleton = true;
    inline bool self = true;
    inline bool vsync = false;

    void draw_players(
        const cache::c_player_cache& players,
        const cache::c_camera_cache& camera,
        ImDrawList* draw_list,
        float width,
        float height );
}