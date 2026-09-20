#pragma once

#include <vector>

#include <cheat/unreal/classes.h>

// player cache ported from FortniteESPBase (source/cache/cache.cc).
// walks the engine -> viewport -> world -> levels -> actors chain over the
// steep r/w layer and keeps a flat snapshot for the render loop.
namespace cache {
    struct cached_player {
        unreal::AFortPlayerPawnAthena* pawn = nullptr;
        unreal::USkeletalMeshComponent* mesh = nullptr;
        unreal::AFortPlayerStateAthena* player_state = nullptr;
        unreal::fvector location;
        unreal::u8 team_index = 0;
        bool is_local = false;
        bool is_dying = false;
        bool is_dbno = false;
        bool is_bot = false;
    };

    class c_player_cache {
    public:
        static constexpr unreal::i32 max_levels = 2048;
        static constexpr unreal::i32 max_actors_per_level = 262144;

        void set_image_base( unreal::uptr image_base );

        unreal::uptr image_base( ) const;
        unreal::UWorld* world( ) const;
        unreal::UGameInstance* game_instance( ) const;
        unreal::APlayerController* player_controller( ) const;
        unreal::AFortPlayerPawnAthena* local_pawn( ) const;
        unreal::u8 local_team_index( ) const;
        const std::vector< cached_player >& players( ) const;

        bool update( );
        bool update( unreal::uptr image_base );

    private:
        void reset( );
        void resolve_local( );
        void collect_players( );
        void collect_from_level( unreal::ULevel* level );
        void collect_player( unreal::AActor* actor );

        unreal::uptr m_image_base = 0;
        unreal::UWorld* m_world = nullptr;
        unreal::UGameInstance* m_game_instance = nullptr;
        unreal::APlayerController* m_player_controller = nullptr;
        unreal::AFortPlayerPawnAthena* m_local_pawn = nullptr;
        unreal::u8 m_local_team_index = 0;
        std::vector< cached_player > m_players;
    };

    inline c_player_cache g_players;
}