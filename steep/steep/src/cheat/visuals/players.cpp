#include <impl/includes.h>

#include <cheat/visuals/players.h>

#include <cheat/cache/camera_cache.h>
#include <cheat/cache/player_cache.h>
#include <cheat/visuals/skeleton.h>

namespace visuals {
    void draw_players(
        const cache::c_player_cache& players,
        const cache::c_camera_cache& camera,
        ImDrawList* draw_list,
        float width,
        float height ) {
        if ( !draw_list || !camera.valid( ) )
            return;

        for ( const auto& player : players.players( ) ) {
            if ( player.is_dying || !player.mesh )
                continue;
            if ( player.is_local && !self )
                continue;

            ImU32 color;
            if ( player.is_local ) {
                color = IM_COL32( 100, 255, 100, 255 );
            }
            else {
                const bool teammate =
                    player.team_index != 0 &&
                    player.team_index == players.local_team_index( );
                color = teammate
                    ? IM_COL32( 80, 180, 255, 255 )
                    : IM_COL32( 255, 70, 70, 255 );
            }

            draw_player(
                camera,
                player.mesh,
                draw_list,
                width,
                height,
                color,
                skeleton,
                boxes );
        }
    }
}