#pragma once

#include <cstdint>


namespace offsets {
    inline constexpr std::uint64_t g_engine                               = 0x1b2c7518;
    inline constexpr std::uint64_t engine_game_viewport                   = 0xb70;
    inline constexpr std::uint64_t viewport_world                         = 0x78;
    inline constexpr std::uint64_t world_persistent_level                 = 0x38;
    inline constexpr std::uint64_t world_cached_view_info                 = 0x178; // DefaultPhysicsVolume (0x158) + 0x20, calculated
    inline constexpr std::uint64_t world_levels                           = 0x1d8;
    inline constexpr std::uint64_t world_game_instance                    = 0x238;
    inline constexpr std::uint64_t level_actors                           = 0x1a0;
    inline constexpr std::uint64_t game_instance_local_players            = 0x38;
    inline constexpr std::uint64_t player_player_controller               = 0x30;
    inline constexpr std::uint64_t scene_component_relative_location      = 0x140;
    inline constexpr std::uint64_t scene_component_component_to_world     = 0x1e0;
    inline constexpr std::uint64_t mesh_bone_array                        = 0x660;
    inline constexpr std::uint64_t mesh_bone_array_cache                  = 0x670; // bone_array + 0x10
    inline constexpr std::uint64_t mesh_component_transforms              = 0x6a8; // bone_array + 0x48
    inline constexpr std::uint64_t actor_root_component                   = 0x1b0;
    inline constexpr std::uint64_t controller_acknowledged_pawn           = 0x318;
    inline constexpr std::uint64_t pawn_player_state                      = 0x290;
    inline constexpr std::uint64_t character_mesh                         = 0x2f0;
    inline constexpr std::uint64_t player_state_b_is_a_bot                = 0x27a; // bit 3
    inline constexpr std::uint64_t fort_pawn_b_is_dying                   = 0x728; // bit 5
    inline constexpr std::uint64_t fort_pawn_b_is_dbno                    = 0x881; // bit 0
    inline constexpr std::uint64_t fort_pawn_revive_time                  = 0x4ad8;
    inline constexpr std::uint64_t fort_state_team_index                  = 0xf69;

    inline constexpr std::uint8_t b_is_a_bot_bit                          = 3;
    inline constexpr std::uint8_t b_is_dying_bit                          = 5;
    inline constexpr std::uint8_t b_is_dbno_bit                           = 0;
}