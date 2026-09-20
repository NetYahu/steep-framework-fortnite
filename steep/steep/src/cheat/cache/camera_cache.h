#pragma once

#include <cheat/unreal/classes.h>

// camera cache ported from FortniteESPBase (source/cache/camera.cc).
// reads UWorld::CachedViewInfo and decomposes view-to-world + projection
// every frame; tolerates a few unstable frames instead of dropping to zero.
namespace cache {
    struct cached_camera {
        unreal::fvector location;
        unreal::frotator rotation;
        unreal::fvector axis_right;
        unreal::fvector axis_up;
        unreal::fvector axis_forward;
        float fov = 0.0f;
        unreal::fworld_cached_view_info view_info;
        bool valid = false;
    };

    class c_camera_cache {
    public:
        const cached_camera& camera( ) const;
        unreal::fvector location( ) const;
        unreal::frotator rotation( ) const;
        float fov( ) const;
        bool valid( ) const;

        bool update( unreal::UWorld* world );
        bool world_to_screen( const unreal::fvector& world, double width, double height, unreal::fvector2d& screen ) const;

    private:
        cached_camera m_camera { };
        unreal::i32 m_unstable_frames = 0;
    };

    inline c_camera_cache g_camera;
}