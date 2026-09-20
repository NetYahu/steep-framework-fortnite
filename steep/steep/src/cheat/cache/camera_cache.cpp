#include <impl/includes.h>

#include <cheat/cache/camera_cache.h>

#include <cmath>

namespace cache {
    const cached_camera& c_camera_cache::camera( ) const {
        return m_camera;
    }

    unreal::fvector c_camera_cache::location( ) const {
        return m_camera.location;
    }

    unreal::frotator c_camera_cache::rotation( ) const {
        return m_camera.rotation;
    }

    float c_camera_cache::fov( ) const {
        return m_camera.fov;
    }

    bool c_camera_cache::valid( ) const {
        return m_camera.valid;
    }

    bool c_camera_cache::update( unreal::UWorld* world ) {
        if ( !world || !world->is_valid( ) ) {
            // keep the last good frame a few ticks so the overlay does not
            // flicker while the game chains in/out
            if ( m_camera.valid && m_unstable_frames < 3 ) {
                ++m_unstable_frames;
                return true;
            }
            m_camera = { };
            return false;
        }

        cached_camera next { };
        bool ok = false;

        const auto views = world->cached_view_info( );
        if ( views.is_valid( ) && views.num > 0 ) {
            const auto view = world->read_element( views, 0 );
            const auto& view_to_world = view.view_to_world;
            const auto& projection = view.projection_matrix;

            next.view_info = view;
            next.axis_right = { view_to_world.M[ 0 ][ 0 ], view_to_world.M[ 0 ][ 1 ], view_to_world.M[ 0 ][ 2 ] };
            next.axis_up = { view_to_world.M[ 1 ][ 0 ], view_to_world.M[ 1 ][ 1 ], view_to_world.M[ 1 ][ 2 ] };
            next.axis_forward = { view_to_world.M[ 2 ][ 0 ], view_to_world.M[ 2 ][ 1 ], view_to_world.M[ 2 ][ 2 ] };
            next.location = { view_to_world.M[ 3 ][ 0 ], view_to_world.M[ 3 ][ 1 ], view_to_world.M[ 3 ][ 2 ] };

            double pitch_value = view_to_world.M[ 2 ][ 2 ];
            if ( pitch_value > 1.0 )
                pitch_value = 1.0;
            if ( pitch_value < -1.0 )
                pitch_value = -1.0;

            next.rotation.Pitch = std::asin( pitch_value ) * 180.0 / unreal::pi;
            next.rotation.Yaw = std::atan2( view_to_world.M[ 2 ][ 1 ], view_to_world.M[ 2 ][ 0 ] ) * 180.0 / unreal::pi;
            next.rotation.Roll = 0.0;

            const auto scale = projection.M[ 0 ][ 0 ];
            next.fov = static_cast< float >( 2.0 * std::atan( 1.0 / scale ) * 180.0 / unreal::pi );

            const double location_squared = next.location.size_squared( );
            if ( std::isfinite( scale ) && scale > 0.0 &&
                std::isfinite( location_squared ) && location_squared >= 1.0 &&
                std::isfinite( next.fov ) ) {
                next.valid = true;
                ok = true;
            }
        }

        if ( ok ) {
            m_camera = next;
            m_unstable_frames = 0;
            return true;
        }

        if ( m_camera.valid && m_unstable_frames < 3 ) {
            ++m_unstable_frames;
            return true;
        }

        m_camera = { };
        return false;
    }

    bool c_camera_cache::world_to_screen( const unreal::fvector& world, double width, double height, unreal::fvector2d& screen ) const {
        if ( !m_camera.valid )
            return false;
        return m_camera.view_info.world_to_screen( world, width, height, screen );
    }
}