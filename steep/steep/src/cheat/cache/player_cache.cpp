#include <impl/includes.h>

#include <cheat/cache/player_cache.h>

namespace cache {
    namespace {
        bool g_chain_logged = false;

        struct debug_flags {
            bool base;
            bool engine;
            bool viewport;
            bool world;
            bool game_instance;
            bool local_players;
            bool local_player;
            bool controller;
        };

        debug_flags g_err_logged { };
        auto g_last_summary = std::chrono::high_resolution_clock::now( );
        std::size_t g_level_count = 0;
        std::size_t g_actor_count = 0;

        // per-pass reject counters so a starved cache tells you WHY it starved
        std::size_t g_reject_filter = 0;
        std::size_t g_reject_dying = 0;
        std::size_t g_reject_mesh = 0;

        void log_stage_once( bool& flag, const char* stage ) {
            if ( flag ) return;
            flag = true;
            logging::warn( oxorany( "chain | %s failed" ), stage );
        }

        // revive-time heuristic is patch-fragile; when it fails but the pawn
        // still carries a valid mesh + playerstate, treat it as a player.
        // keeps ESP alive across patches and includes bots (flagged via is_bot).
        template < typename pawn_t, typename state_t >
        bool looks_like_player( pawn_t* pawn ) {
            if ( !pawn )
                return false;
            if ( pawn->is_player_pawn( ) )
                return true;
            auto* mesh = pawn->mesh( );
            auto* state = static_cast< state_t* >( pawn->player_state( ) );
            return mesh && mesh->is_valid( ) && state && state->is_valid( );
        }
    }

    void c_player_cache::set_image_base( unreal::uptr image_base ) {
        m_image_base = image_base;
    }

    unreal::uptr c_player_cache::image_base( ) const {
        return m_image_base;
    }

    unreal::UWorld* c_player_cache::world( ) const {
        return m_world;
    }

    unreal::UGameInstance* c_player_cache::game_instance( ) const {
        return m_game_instance;
    }

    unreal::APlayerController* c_player_cache::player_controller( ) const {
        return m_player_controller;
    }

    unreal::AFortPlayerPawnAthena* c_player_cache::local_pawn( ) const {
        return m_local_pawn;
    }

    unreal::u8 c_player_cache::local_team_index( ) const {
        return m_local_team_index;
    }

    const std::vector< cached_player >& c_player_cache::players( ) const {
        return m_players;
    }

    bool c_player_cache::update( ) {
        return update( m_image_base );
    }

    void c_player_cache::reset( ) {
        m_world = nullptr;
        m_game_instance = nullptr;
        m_player_controller = nullptr;
        m_local_pawn = nullptr;
        m_local_team_index = 0;
        m_players.clear( );
    }

    bool c_player_cache::update( unreal::uptr image_base ) {
        reset( );
        m_image_base = image_base;
        if ( !image_base )
            return false;

        unreal::UEngine* engine = unreal::UEngine::get( image_base );
        if ( !engine || !engine->is_valid( ) ) {
            log_stage_once( g_err_logged.engine, oxorany( "engine" ) );
            return false;
        }

        unreal::UGameViewportClient* viewport = engine->game_viewport( );
        if ( !viewport || !viewport->is_valid( ) ) {
            log_stage_once( g_err_logged.viewport, oxorany( "viewport" ) );
            return false;
        }

        m_world = viewport->world( );
        if ( !m_world || !m_world->is_valid( ) ) {
            log_stage_once( g_err_logged.world, oxorany( "world" ) );
            return false;
        }

        m_game_instance = m_world->owning_game_instance( );
        if ( !m_game_instance || !m_game_instance->is_valid( ) ) {
            log_stage_once( g_err_logged.game_instance, oxorany( "game instance" ) );
            return false;
        }

        const auto local_players = m_game_instance->local_players( );
        if ( !local_players.is_valid_index( 0 ) ) {
            log_stage_once( g_err_logged.local_players, oxorany( "local players" ) );
            return false;
        }

        unreal::ULocalPlayer* local_player = m_game_instance->read_element( local_players, 0 );
        if ( !local_player || !local_player->is_valid( ) ) {
            log_stage_once( g_err_logged.local_player, oxorany( "local player" ) );
            return false;
        }

        m_player_controller = local_player->player_controller( );
        if ( !m_player_controller || !m_player_controller->is_valid( ) ) {
            log_stage_once( g_err_logged.controller, oxorany( "controller" ) );
            return false;
        }

        if ( !g_chain_logged ) {
            logging::print( oxorany( "chain | engine=0x%llx viewport=0x%llx world=0x%llx controller=0x%llx" ),
                reinterpret_cast< unreal::u64 >( engine ),
                reinterpret_cast< unreal::u64 >( viewport ),
                reinterpret_cast< unreal::u64 >( m_world ),
                reinterpret_cast< unreal::u64 >( m_player_controller ) );
            g_chain_logged = true;
        }

        resolve_local( );
        collect_players( );

        // live summary every ~2s: answers "is the cache good" at a glance.
        // healthy in-match values look like: levels>=1 actors=thousands
        // players=dozens local!=0. players<=1 + local=0 means the filter or
        // the ack-pawn offset is stale, or you are still in lobby/staging.
        {
            const auto now = std::chrono::high_resolution_clock::now( );
            if ( now - g_last_summary >= ch::milliseconds( 2000 ) ) {
                g_last_summary = now;
                logging::print( oxorany( "cache | levels=%d actors=%d players=%d local=0x%llx team=%d [rej filter=%d dying=%d mesh=%d]" ),
                    static_cast< int >( g_level_count ),
                    static_cast< int >( g_actor_count ),
                    static_cast< int >( m_players.size( ) ),
                    reinterpret_cast< unreal::u64 >( m_local_pawn ),
                    m_local_team_index,
                    static_cast< int >( g_reject_filter ),
                    static_cast< int >( g_reject_dying ),
                    static_cast< int >( g_reject_mesh ) );
            }
        }

        return true;
    }

    void c_player_cache::resolve_local( ) {
        m_local_pawn = static_cast< unreal::AFortPlayerPawnAthena* >( m_player_controller->acknowledged_pawn( ) );
        if ( !m_local_pawn || !m_local_pawn->is_valid( ) ) {
            m_local_pawn = nullptr;
            return;
        }

        unreal::AFortPlayerStateAthena* local_state =
            static_cast< unreal::AFortPlayerStateAthena* >( m_local_pawn->player_state( ) );
        if ( local_state && local_state->is_valid( ) )
            m_local_team_index = local_state->team_index( );
    }

    void c_player_cache::collect_players( ) {
        g_level_count = 0;
        g_actor_count = 0;
        g_reject_filter = 0;
        g_reject_dying = 0;
        g_reject_mesh = 0;

        const auto levels = m_world->levels( );
        if ( !levels.is_valid( ) || levels.num > max_levels )
            return;

        m_world->for_each(
            levels,
            max_levels,
            [ & ]( unreal::ULevel* level ) {
                ++g_level_count;
                collect_from_level( level );
            } );
    }

    void c_player_cache::collect_from_level( unreal::ULevel* level ) {
        if ( !level || !level->is_valid( ) )
            return;

        const auto actors = level->actors( );
        if ( !actors.is_valid( ) || actors.num > max_actors_per_level )
            return;

        level->for_each(
            actors,
            max_actors_per_level,
            [ & ]( unreal::AActor* actor ) {
                ++g_actor_count;
                collect_player( actor );
            } );
    }

    void c_player_cache::collect_player( unreal::AActor* actor ) {
        if ( !actor || !actor->is_valid( ) )
            return;

        auto* pawn = static_cast< unreal::AFortPlayerPawnAthena* >( actor );
        if ( !looks_like_player< unreal::AFortPlayerPawnAthena, unreal::AFortPlayerStateAthena >( pawn ) ) {
            ++g_reject_filter;
            return;
        }

        if ( pawn->b_is_dying( ) ) {
            ++g_reject_dying;
            return;
        }

        cached_player player { };
        player.pawn = pawn;
        player.mesh = pawn->mesh( );
        if ( !player.mesh || !player.mesh->is_valid( ) ) {
            ++g_reject_mesh;
            return;
        }
        player.player_state = static_cast< unreal::AFortPlayerStateAthena* >( pawn->player_state( ) );
        player.location = pawn->location( );
        player.is_local = pawn == m_local_pawn;
        player.is_dying = pawn->b_is_dying( );
        player.is_dbno = pawn->b_is_dbno( );

        if ( player.player_state && player.player_state->is_valid( ) ) {
            player.team_index = player.player_state->team_index( );
            player.is_bot = player.player_state->b_is_a_bot( );
        }

        m_players.push_back( player );
    }
}