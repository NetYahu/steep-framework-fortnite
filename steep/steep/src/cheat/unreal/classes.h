#pragma once

#include <cmath>

#include <cheat/sdk.h>
#include <cheat/offsets.h>
#include <cheat/unreal/types.h>

// unreal object facade ported from FortniteESPBase (unreal/classes.hh).
// every class owns its layout and exposes typed reads over the steep
// physical r/w layer — no global offset table, the object is the API.
namespace unreal {
    inline bool valid_address( uptr address ) {
        return utility::is_valid_pointer( address );
    }

    class UObject;
    class UEngine;
    class UGameViewportClient;
    class UWorld;
    class ULevel;
    class UGameInstance;
    class UPlayer;
    class ULocalPlayer;
    class USceneComponent;
    class USkeletalMeshComponent;
    class AActor;
    class AController;
    class APlayerController;
    class APawn;
    class ACharacter;
    class APlayerState;
    class AFortPawn;
    class AFortPlayerPawnAthena;
    class AFortPlayerStateAthena;

    class UObject {
    public:
        uptr address( ) const {
            return reinterpret_cast< uptr >( this );
        }

        bool is_valid( ) const {
            return valid_address( address( ) );
        }

        // single bulk read through the steep r/w layer, defined in classes.cpp
        static bool read_raw( uptr source, void* destination, u64 size );

        template < typename t >
        static t read_at( uptr address ) {
            return cheat::readx< t >( static_cast< std::uint64_t >( address ) );
        }

        template < typename t >
        t read_field( u64 offset ) const {
            return read_at< t >( address( ) + offset );
        }

        template < typename t >
        t* read_ptr( u64 offset ) const {
            return read_at< t* >( address( ) + offset );
        }

        template < typename t >
        tarray< t > read_array( u64 offset ) const {
            return read_at< tarray< t > >( address( ) + offset );
        }

        template < typename t >
        t read_element( const tarray< t >& array, i32 index ) const {
            if ( !array.is_valid_index( index ) )
                return t { };
            return read_at< t >( array.address( ) + static_cast< uptr >( index ) * sizeof( t ) );
        }

        template < typename t >
        static bool read_range( uptr source, t* out, i32 count ) {
            if ( !out || count <= 0 )
                return false;
            return read_raw( source, out, static_cast< u64 >( count ) * sizeof( t ) );
        }

        bool read_bit( u64 offset, u8 bit ) const {
            return ( read_field< u8 >( offset ) & ( 1U << bit ) ) != 0;
        }

        // reads arrays in large batches instead of one remote read per element,
        // which matters when talking to physical-memory r/w
        template < typename t, typename callback >
        bool for_each( const tarray< t >& array, i32 maximum_count, callback&& fn ) const {
            if ( !array.is_valid( ) )
                return false;

            const i32 count = array.num;
            if ( !count )
                return true;
            if ( count > maximum_count )
                return false;

            constexpr i32 batch_capacity = 128;
            t values[ batch_capacity ] { };

            for ( i32 first = 0; first < count; ) {
                const i32 remaining = count - first;
                const i32 number = remaining < batch_capacity ? remaining : batch_capacity;
                const uptr src = array.address( ) + static_cast< uptr >( first ) * sizeof( t );

                if ( read_range( src, values, number ) ) {
                    for ( i32 index = 0; index < number; ++index )
                        fn( values[ index ] );
                }
                else {
                    for ( i32 index = 0; index < number; ++index )
                        fn( read_at< t >( src + static_cast< uptr >( index ) * sizeof( t ) ) );
                }

                first += number;
            }

            return true;
        }
    };

    class UEngine : public UObject {
    public:
        static UEngine* get( uptr image_base ) {
            return read_at< UEngine* >( image_base + offsets::g_engine );
        }

        UGameViewportClient* game_viewport( ) const {
            return read_ptr< UGameViewportClient >( offsets::engine_game_viewport );
        }
    };

    class UGameViewportClient : public UObject {
    public:
        UWorld* world( ) const {
            return read_ptr< UWorld >( offsets::viewport_world );
        }
    };

    class UWorld : public UObject {
    public:
        ULevel* persistent_level( ) const {
            return read_ptr< ULevel >( offsets::world_persistent_level );
        }

        tarray< ULevel* > levels( ) const {
            return read_array< ULevel* >( offsets::world_levels );
        }

        tarray< fworld_cached_view_info > cached_view_info( ) const {
            return read_array< fworld_cached_view_info >( offsets::world_cached_view_info );
        }

        UGameInstance* owning_game_instance( ) const {
            return read_ptr< UGameInstance >( offsets::world_game_instance );
        }
    };

    class ULevel : public UObject {
    public:
        tarray< AActor* > actors( ) const {
            return read_array< AActor* >( offsets::level_actors );
        }
    };

    class UGameInstance : public UObject {
    public:
        tarray< ULocalPlayer* > local_players( ) const {
            return read_array< ULocalPlayer* >( offsets::game_instance_local_players );
        }
    };

    class UPlayer : public UObject {
    public:
        APlayerController* player_controller( ) const {
            return read_ptr< APlayerController >( offsets::player_player_controller );
        }
    };

    class ULocalPlayer : public UPlayer {
    };

    class USceneComponent : public UObject {
    public:
        fvector relative_location( ) const {
            return read_field< fvector >( offsets::scene_component_relative_location );
        }

        ftransform component_to_world( ) const {
            return read_field< ftransform >( offsets::scene_component_component_to_world );
        }
    };

    class USkeletalMeshComponent : public USceneComponent {
    public:
        tarray< ftransform > bone_array( ) const {
            return read_array< ftransform >( offsets::mesh_bone_array );
        }

        tarray< ftransform > bone_array_cache( ) const {
            return read_array< ftransform >( offsets::mesh_bone_array_cache );
        }

        u8 current_read_component_transforms( ) const {
            return read_field< u8 >( offsets::mesh_component_transforms );
        }

        bool read_bone_locations( const i32* indices, i32 count, fvector* out ) const {
            if ( !is_valid( ) || !indices || !out || count <= 0 )
                return false;

            const tarray< ftransform > bones =
                ( current_read_component_transforms( ) & 1 ) ? bone_array_cache( ) : bone_array( );
            if ( !bones.is_valid( ) )
                return false;

            i32 max_index = 0;
            for ( i32 index = 0; index < count; ++index ) {
                if ( indices[ index ] > max_index )
                    max_index = indices[ index ];
            }

            const i32 needed = max_index + 1;
            if ( needed > bones.num || needed > 128 )
                return false;

            ftransform transforms[ 128 ];
            if ( !read_range( bones.address( ), transforms, needed ) )
                return false;

            const fmatrix component_to_world = this->component_to_world( ).to_matrix_with_scale( );

            for ( i32 index = 0; index < count; ++index ) {
                const i32 bone_index = indices[ index ];
                if ( bone_index < 0 || bone_index >= needed )
                    return false;
                const fmatrix matrix = transforms[ bone_index ].to_matrix_with_scale( ) * component_to_world;
                out[ index ] = { matrix.WPlane.X, matrix.WPlane.Y, matrix.WPlane.Z };
            }

            return true;
        }
    };

    class AActor : public UObject {
    public:
        USceneComponent* root_component( ) const {
            return read_ptr< USceneComponent >( offsets::actor_root_component );
        }

        fvector location( ) const {
            USceneComponent* root = root_component( );
            if ( !root || !root->is_valid( ) )
                return { };
            return root->relative_location( );
        }
    };

    class AController : public AActor {
    };

    class APlayerController : public AController {
    public:
        APawn* acknowledged_pawn( ) const {
            return read_ptr< APawn >( offsets::controller_acknowledged_pawn );
        }
    };

    class APawn : public AActor {
    public:
        APlayerState* player_state( ) const {
            return read_ptr< APlayerState >( offsets::pawn_player_state );
        }
    };

    class ACharacter : public APawn {
    public:
        USkeletalMeshComponent* mesh( ) const {
            return read_ptr< USkeletalMeshComponent >( offsets::character_mesh );
        }
    };

    class AFortPawn : public ACharacter {
    public:
        bool b_is_dying( ) const {
            return read_bit( offsets::fort_pawn_b_is_dying, offsets::b_is_dying_bit );
        }

        bool b_is_dbno( ) const {
            return read_bit( offsets::fort_pawn_b_is_dbno, offsets::b_is_dbno_bit );
        }
    };

    class AFortPlayerPawnAthena : public AFortPawn {
    public:
        float revive_from_dbno_time( ) const {
            return read_field< float >( offsets::fort_pawn_revive_time );
        }

        // real player pawns carry a 10s revive time; bots do not
        bool is_player_pawn( ) const {
            return std::fabs( revive_from_dbno_time( ) - 10.0f ) < 0.5f;
        }
    };

    class APlayerState : public AActor {
    public:
        bool b_is_a_bot( ) const {
            return read_bit( offsets::player_state_b_is_a_bot, offsets::b_is_a_bot_bit );
        }
    };

    class AFortPlayerStateAthena : public APlayerState {
    public:
        u8 team_index( ) const {
            return read_field< u8 >( offsets::fort_state_team_index );
        }
    };
}