#include <impl/includes.h>

#include <cheat/updater.h>

#include <cheat/cache/camera_cache.h>
#include <cheat/cache/player_cache.h>
#include <cheat/offsets.h>
#include <cheat/sdk.h>

namespace updater {
    namespace {
        bool g_chain_logged = false;

        auto g_last_log = std::chrono::high_resolution_clock::now( );

        void log_throttle( const char* message, std::size_t players ) {
            const auto now = std::chrono::high_resolution_clock::now( );
            if ( now - g_last_log < ch::milliseconds( 1000 ) )
                return;
            g_last_log = now;
            logging::print( oxorany( "tick | %s players=%d local=0x%llx camera=%d" ),
                message,
                static_cast< int >( players ),
                reinterpret_cast< unsigned long long >( cache::g_players.local_pawn( ) ),
                cache::g_camera.valid( ) ? 1 : 0 );
        }

        void dump_offsets( ) {
            logging::print( oxorany( "offsets | g_engine=0x%llx game_viewport=0x%llx viewport_world=0x%llx" ),
                offsets::g_engine, offsets::engine_game_viewport, offsets::viewport_world );
            logging::print( oxorany( "offsets | world_game_instance=0x%llx local_players=0x%llx player_controller=0x%llx" ),
                offsets::world_game_instance, offsets::game_instance_local_players, offsets::player_player_controller );
            logging::print( oxorany( "offsets | acknowledged_pawn=0x%llx mesh=0x%llx bone_array=0x%llx" ),
                offsets::controller_acknowledged_pawn, offsets::character_mesh, offsets::mesh_bone_array );
            logging::print( oxorany( "offsets | cached_view_info=0x%llx revive_time=0x%llx team_index=0x%llx" ),
                offsets::world_cached_view_info, offsets::fort_pawn_revive_time, offsets::fort_state_team_index );
            logging::print( oxorany( "offsets | new chain | image_base + g_engine -> game_viewport -> world" ) );
        }
    }

    void run( ) {
        dump_offsets( );

        while ( true ) {
            if ( !cheat::g_game || !cheat::g_base ) {
                // fortnite not open yet, poll slowly
                utility::sleep_short( 250.0f );
                continue;
            }

            if ( !cache::g_players.update( cheat::g_base ) || !cache::g_players.world( ) ) {
                // still on the loading/staging screen, poll a bit faster
                log_throttle( oxorany( "waiting for world" ), cache::g_players.players( ).size( ) );
                utility::sleep_short( 100.0f );
                continue;
            }

            // in a lobby or match, refresh the player list at ~20hz, count every ~1s
            log_throttle( oxorany( "ok" ), cache::g_players.players( ).size( ) );
            utility::sleep_short( 50.0f );
        }
    }
}