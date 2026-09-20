#include <impl/includes.h>

#include <cheat/visuals/skeleton.h>

#include <cheat/cache/camera_cache.h>
#include <cheat/unreal/classes.h>
#include <cheat/unreal/enums.h>

#include <cmath>

namespace visuals {
    namespace {
        inline constexpr int skeleton_pairs[ ][ 2 ] = {
            { static_cast< int >( unreal::e_fort_skeleton::spine_05 ), static_cast< int >( unreal::e_fort_skeleton::neck_01 ) },
            { static_cast< int >( unreal::e_fort_skeleton::neck_01 ), static_cast< int >( unreal::e_fort_skeleton::head ) },
            { static_cast< int >( unreal::e_fort_skeleton::spine_05 ), static_cast< int >( unreal::e_fort_skeleton::upperarm_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::spine_05 ), static_cast< int >( unreal::e_fort_skeleton::upperarm_l ) },
            { static_cast< int >( unreal::e_fort_skeleton::upperarm_l ), static_cast< int >( unreal::e_fort_skeleton::lowerarm_l ) },
            { static_cast< int >( unreal::e_fort_skeleton::lowerarm_l ), static_cast< int >( unreal::e_fort_skeleton::hand_l ) },
            { static_cast< int >( unreal::e_fort_skeleton::upperarm_r ), static_cast< int >( unreal::e_fort_skeleton::lowerarm_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::lowerarm_r ), static_cast< int >( unreal::e_fort_skeleton::hand_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::spine_05 ), static_cast< int >( unreal::e_fort_skeleton::pelvis ) },
            { static_cast< int >( unreal::e_fort_skeleton::pelvis ), static_cast< int >( unreal::e_fort_skeleton::thigh_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::pelvis ), static_cast< int >( unreal::e_fort_skeleton::thigh_l ) },
            { static_cast< int >( unreal::e_fort_skeleton::thigh_r ), static_cast< int >( unreal::e_fort_skeleton::calf_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::calf_r ), static_cast< int >( unreal::e_fort_skeleton::foot_r ) },
            { static_cast< int >( unreal::e_fort_skeleton::thigh_l ), static_cast< int >( unreal::e_fort_skeleton::calf_l ) },
            { static_cast< int >( unreal::e_fort_skeleton::calf_l ), static_cast< int >( unreal::e_fort_skeleton::foot_l ) },
        };

        constexpr int skeleton_pair_count = static_cast< int >( sizeof( skeleton_pairs ) / sizeof( skeleton_pairs[ 0 ] ) );

        constexpr int skeleton_lookup[ 16 ] = {
            static_cast< int >( unreal::e_fort_skeleton::head ),
            static_cast< int >( unreal::e_fort_skeleton::neck_01 ),
            static_cast< int >( unreal::e_fort_skeleton::spine_05 ),
            static_cast< int >( unreal::e_fort_skeleton::pelvis ),
            static_cast< int >( unreal::e_fort_skeleton::upperarm_l ),
            static_cast< int >( unreal::e_fort_skeleton::lowerarm_l ),
            static_cast< int >( unreal::e_fort_skeleton::hand_l ),
            static_cast< int >( unreal::e_fort_skeleton::upperarm_r ),
            static_cast< int >( unreal::e_fort_skeleton::lowerarm_r ),
            static_cast< int >( unreal::e_fort_skeleton::hand_r ),
            static_cast< int >( unreal::e_fort_skeleton::thigh_l ),
            static_cast< int >( unreal::e_fort_skeleton::calf_l ),
            static_cast< int >( unreal::e_fort_skeleton::foot_l ),
            static_cast< int >( unreal::e_fort_skeleton::thigh_r ),
            static_cast< int >( unreal::e_fort_skeleton::calf_r ),
            static_cast< int >( unreal::e_fort_skeleton::foot_r ),
        };

        int bone_slot( int bone ) {
            switch ( bone ) {
            case static_cast< int >( unreal::e_fort_skeleton::head ): return 0;
            case static_cast< int >( unreal::e_fort_skeleton::neck_01 ): return 1;
            case static_cast< int >( unreal::e_fort_skeleton::spine_05 ): return 2;
            case static_cast< int >( unreal::e_fort_skeleton::pelvis ): return 3;
            case static_cast< int >( unreal::e_fort_skeleton::upperarm_l ): return 4;
            case static_cast< int >( unreal::e_fort_skeleton::lowerarm_l ): return 5;
            case static_cast< int >( unreal::e_fort_skeleton::hand_l ): return 6;
            case static_cast< int >( unreal::e_fort_skeleton::upperarm_r ): return 7;
            case static_cast< int >( unreal::e_fort_skeleton::lowerarm_r ): return 8;
            case static_cast< int >( unreal::e_fort_skeleton::hand_r ): return 9;
            case static_cast< int >( unreal::e_fort_skeleton::thigh_l ): return 10;
            case static_cast< int >( unreal::e_fort_skeleton::calf_l ): return 11;
            case static_cast< int >( unreal::e_fort_skeleton::foot_l ): return 12;
            case static_cast< int >( unreal::e_fort_skeleton::thigh_r ): return 13;
            case static_cast< int >( unreal::e_fort_skeleton::calf_r ): return 14;
            case static_cast< int >( unreal::e_fort_skeleton::foot_r ): return 15;
            default: return -1;
            }
        }

        void draw_line( ImDrawList* draw_list,
            const unreal::fvector2d& from,
            const unreal::fvector2d& to,
            ImU32 color,
            float thickness ) {
            draw_list->AddLine(
                ImVec2( static_cast< float >( from.X ), static_cast< float >( from.Y ) ),
                ImVec2( static_cast< float >( to.X ), static_cast< float >( to.Y ) ),
                IM_COL32( 0, 0, 0, 180 ),
                thickness + 1.6f );
            draw_list->AddLine(
                ImVec2( static_cast< float >( from.X ), static_cast< float >( from.Y ) ),
                ImVec2( static_cast< float >( to.X ), static_cast< float >( to.Y ) ),
                color,
                thickness );
        }

        void draw_box( ImDrawList* draw_list,
            const unreal::fvector2d& min,
            const unreal::fvector2d& max,
            ImU32 color,
            float thickness ) {
            const ImVec2 top_left( static_cast< float >( min.X ), static_cast< float >( min.Y ) );
            const ImVec2 bottom_right( static_cast< float >( max.X ), static_cast< float >( max.Y ) );

            draw_list->AddRect( top_left, bottom_right, IM_COL32( 0, 0, 0, 180 ), 0.0f, 0, thickness + 1.6f );
            draw_list->AddRect( top_left, bottom_right, color, 0.0f, 0, thickness );
        }
    }

    void draw_player(
        const cache::c_camera_cache& camera,
        unreal::USkeletalMeshComponent* mesh,
        ImDrawList* draw_list,
        float width,
        float height,
        ImU32 color,
        bool draw_skeleton,
        bool draw_boxes ) {
        if ( !camera.valid( ) || !mesh || !mesh->is_valid( ) || !draw_list )
            return;

        unreal::fvector world[ 16 ];
        if ( !mesh->read_bone_locations( skeleton_lookup, 16, world ) )
            return;

        bool sane = true;
        double max_offset = 0.0;
        for ( int index = 0; index < 16; ++index ) {
            if ( !std::isfinite( world[ index ].X ) ||
                !std::isfinite( world[ index ].Y ) ||
                !std::isfinite( world[ index ].Z ) ) {
                sane = false;
                break;
            }
            const double offset = ( world[ index ] - world[ 3 ] ).size( );
            if ( offset > max_offset )
                max_offset = offset;
        }
        if ( !sane || max_offset > 400.0 )
            return;

        unreal::fvector2d screen[ 16 ];
        bool valid[ 16 ] { };
        for ( int index = 0; index < 16; ++index )
            valid[ index ] = camera.world_to_screen( world[ index ], width, height, screen[ index ] );

        if ( draw_skeleton ) {
            for ( int pair = 0; pair < skeleton_pair_count; ++pair ) {
                const auto from_slot = bone_slot( skeleton_pairs[ pair ][ 0 ] );
                const auto to_slot = bone_slot( skeleton_pairs[ pair ][ 1 ] );
                if ( from_slot < 0 || to_slot < 0 )
                    continue;
                if ( !valid[ from_slot ] || !valid[ to_slot ] )
                    continue;

                draw_line( draw_list, screen[ from_slot ], screen[ to_slot ], color, 1.6f );
            }
        }

        if ( draw_boxes ) {
            bool any = false;
            double min_x = 0.0, min_y = 0.0, max_x = 0.0, max_y = 0.0;
            for ( int index = 0; index < 16; ++index ) {
                if ( !valid[ index ] )
                    continue;

                const double x = screen[ index ].X;
                const double y = screen[ index ].Y;
                if ( !any ) {
                    min_x = max_x = x;
                    min_y = max_y = y;
                    any = true;
                    continue;
                }
                if ( x < min_x ) min_x = x;
                if ( x > max_x ) max_x = x;
                if ( y < min_y ) min_y = y;
                if ( y > max_y ) max_y = y;
            }

            if ( any ) {
                const double box_width = max_x - min_x;
                const double box_height = max_y - min_y;

                double pad_x = box_width * 0.15;
                if ( pad_x < 6.0 )
                    pad_x = 6.0;

                double pad_top = box_height * 0.12;
                if ( pad_top < 6.0 )
                    pad_top = 6.0;

                double pad_bottom = box_height * 0.08;
                if ( pad_bottom < 6.0 )
                    pad_bottom = 6.0;

                draw_box(
                    draw_list,
                    { min_x - pad_x, min_y - pad_top },
                    { max_x + pad_x, max_y + pad_bottom },
                    color,
                    1.6f );
            }
        }
    }
}