#pragma once

#include <cmath>
#include <cstdint>

// engine value types ported from FortniteESPBase (unreal/structures.hh).
// member names mirror the real UE5 layout; local helpers are snake_case.
namespace unreal {
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using uptr = std::uintptr_t;

    constexpr double pi = 3.14159265358979323846;
    constexpr double small_number = 1e-8;
    constexpr double kinda_small_number = 1e-4;

    template < typename t >
    struct tarray {
        t* data;
        i32 num;
        i32 Max;

        tarray( ) : data( nullptr ), num( 0 ), Max( 0 ) { }
        tarray( t* in_data, i32 in_num, i32 in_max ) : data( in_data ), num( in_num ), Max( in_max ) { }

        bool is_valid( ) const { return data && num >= 0 && Max >= num; }
        bool is_valid_index( i32 index ) const { return index >= 0 && index < num && index < Max; }
        i32 size( ) const { return num; }
        i32 max_size( ) const { return Max; }
        uptr address( ) const { return reinterpret_cast< uptr >( data ); }
    };

    static_assert( sizeof( tarray< u8 > ) == 0x10, "tarray must be 16 bytes" );

    struct fvector2d {
        double X, Y;

        fvector2d( ) : X( 0.0 ), Y( 0.0 ) { }
        fvector2d( double in_x, double in_y ) : X( in_x ), Y( in_y ) { }

        fvector2d operator+( const fvector2d& other ) const { return { X + other.X, Y + other.Y }; }
        fvector2d operator-( const fvector2d& other ) const { return { X - other.X, Y - other.Y }; }
        fvector2d operator*( double scale ) const { return { X * scale, Y * scale }; }
        fvector2d operator/( double scale ) const { return { X / scale, Y / scale }; }

        fvector2d& operator+=( const fvector2d& other ) { X += other.X; Y += other.Y; return *this; }
        fvector2d& operator-=( const fvector2d& other ) { X -= other.X; Y -= other.Y; return *this; }
        fvector2d& operator*=( double scale ) { X *= scale; Y *= scale; return *this; }
        fvector2d& operator/=( double scale ) { X /= scale; Y /= scale; return *this; }

        bool operator==( const fvector2d& other ) const { return X == other.X && Y == other.Y; }
        bool operator!=( const fvector2d& other ) const { return !( *this == other ); }
        operator bool( ) const { return X != 0.0 || Y != 0.0; }

        double dot( const fvector2d& other ) const { return X * other.X + Y * other.Y; }
        double size( ) const { return std::sqrt( X * X + Y * Y ); }
        double size_squared( ) const { return X * X + Y * Y; }
        double distance_to( const fvector2d& other ) const {
            const double dx = other.X - X;
            const double dy = other.Y - Y;
            return std::sqrt( dx * dx + dy * dy );
        }
        bool is_valid( ) const { return X != 0.0 || Y != 0.0; }
    };

    static_assert( sizeof( fvector2d ) == 0x10, "fvector2d must be 16 bytes" );

    struct fvector {
        double X, Y, Z;

        fvector( ) : X( 0.0 ), Y( 0.0 ), Z( 0.0 ) { }
        fvector( double in_x, double in_y, double in_z ) : X( in_x ), Y( in_y ), Z( in_z ) { }

        fvector operator+( const fvector& other ) const { return { X + other.X, Y + other.Y, Z + other.Z }; }
        fvector operator-( const fvector& other ) const { return { X - other.X, Y - other.Y, Z - other.Z }; }
        fvector operator*( const fvector& other ) const { return { X * other.X, Y * other.Y, Z * other.Z }; }
        fvector operator*( double scale ) const { return { X * scale, Y * scale, Z * scale }; }
        fvector operator/( double scale ) const { return { X / scale, Y / scale, Z / scale }; }

        fvector& operator+=( const fvector& other ) { X += other.X; Y += other.Y; Z += other.Z; return *this; }
        fvector& operator-=( const fvector& other ) { X -= other.X; Y -= other.Y; Z -= other.Z; return *this; }
        fvector& operator*=( const fvector& other ) { X *= other.X; Y *= other.Y; Z *= other.Z; return *this; }
        fvector& operator*=( double scale ) { X *= scale; Y *= scale; Z *= scale; return *this; }
        fvector& operator/=( double scale ) { X /= scale; Y /= scale; Z /= scale; return *this; }

        bool operator==( const fvector& other ) const { return X == other.X && Y == other.Y && Z == other.Z; }
        bool operator!=( const fvector& other ) const { return !( *this == other ); }
        operator bool( ) const { return X != 0.0 || Y != 0.0 || Z != 0.0; }

        double dot( const fvector& other ) const { return X * other.X + Y * other.Y + Z * other.Z; }
        fvector cross( const fvector& other ) const {
            return {
                Y * other.Z - Z * other.Y,
                Z * other.X - X * other.Z,
                X * other.Y - Y * other.X
            };
        }
        double size( ) const { return std::sqrt( X * X + Y * Y + Z * Z ); }
        double size_squared( ) const { return X * X + Y * Y + Z * Z; }
        double size2d( ) const { return std::sqrt( X * X + Y * Y ); }

        // unreal units are cm; returns metres
        double distance_to( const fvector& other ) const {
            return ( other - *this ).size( ) / 100.0;
        }
        double distance_squared( const fvector& other ) const {
            return ( other - *this ).size_squared( );
        }

        fvector get_safe_normal( double tolerance = small_number ) const {
            const double square_sum = size_squared( );
            if ( square_sum > tolerance ) {
                const double scale = 1.0 / std::sqrt( square_sum );
                return { X * scale, Y * scale, Z * scale };
            }
            return { };
        }

        fvector get_unsafe_normal( ) const {
            const double scale = 1.0 / size( );
            return { X * scale, Y * scale, Z * scale };
        }

        bool is_nearly_zero( double tolerance = kinda_small_number ) const {
            return std::fabs( X ) <= tolerance && std::fabs( Y ) <= tolerance && std::fabs( Z ) <= tolerance;
        }
        bool is_zero( ) const { return X == 0.0 && Y == 0.0 && Z == 0.0; }
        bool is_valid( ) const { return X != 0.0 || Y != 0.0 || Z != 0.0; }

        fvector lerp( const fvector& other, double alpha ) const {
            return *this * ( 1.0 - alpha ) + other * alpha;
        }
    };

    static_assert( sizeof( fvector ) == 0x18, "fvector must be 24 bytes (LWC)" );

    struct fvector4 {
        double X, Y, Z, W;

        fvector4( ) : X( 0.0 ), Y( 0.0 ), Z( 0.0 ), W( 1.0 ) { }
        fvector4( double in_x, double in_y, double in_z, double in_w ) : X( in_x ), Y( in_y ), Z( in_z ), W( in_w ) { }
    };

    static_assert( sizeof( fvector4 ) == 0x20, "fvector4 must be 32 bytes" );

    struct fplane {
        double X, Y, Z, W;
    };

    static_assert( sizeof( fplane ) == 0x20, "fplane must be 32 bytes" );

    struct frotator {
        double Pitch, Yaw, Roll;

        frotator( ) : Pitch( 0.0 ), Yaw( 0.0 ), Roll( 0.0 ) { }
        frotator( double in_pitch, double in_yaw, double in_roll ) : Pitch( in_pitch ), Yaw( in_yaw ), Roll( in_roll ) { }

        frotator operator+( const frotator& other ) const { return { Pitch + other.Pitch, Yaw + other.Yaw, Roll + other.Roll }; }
        frotator operator-( const frotator& other ) const { return { Pitch - other.Pitch, Yaw - other.Yaw, Roll - other.Roll }; }
        frotator operator*( double scale ) const { return { Pitch * scale, Yaw * scale, Roll * scale }; }

        frotator& operator+=( const frotator& other ) { Pitch += other.Pitch; Yaw += other.Yaw; Roll += other.Roll; return *this; }
        frotator& operator-=( const frotator& other ) { Pitch -= other.Pitch; Yaw -= other.Yaw; Roll -= other.Roll; return *this; }

        bool operator==( const frotator& other ) const {
            return Pitch == other.Pitch && Yaw == other.Yaw && Roll == other.Roll;
        }
        bool operator!=( const frotator& other ) const { return !( *this == other ); }

        static double clamp_axis( double angle ) {
            angle = std::fmod( angle, 360.0 );
            if ( angle < 0.0 )
                angle += 360.0;
            return angle;
        }
        static double normalize_axis( double angle ) {
            angle = clamp_axis( angle );
            if ( angle > 180.0 )
                angle -= 360.0;
            return angle;
        }

        void normalize( ) {
            Pitch = normalize_axis( Pitch );
            Yaw = normalize_axis( Yaw );
            Roll = normalize_axis( Roll );
        }

        frotator get_normalized( ) const {
            frotator result = *this;
            result.normalize( );
            return result;
        }

        fvector vector( ) const {
            const double pitch_rad = Pitch * ( pi / 180.0 );
            const double yaw_rad = Yaw * ( pi / 180.0 );
            const double cp = std::cos( pitch_rad );
            const double sp = std::sin( pitch_rad );
            const double cy = std::cos( yaw_rad );
            const double sy = std::sin( yaw_rad );
            return { cp * cy, cp * sy, sp };
        }

        fvector get_forward_vector( ) const { return vector( ); }
        bool is_valid( ) const { return Pitch != 0.0 || Yaw != 0.0 || Roll != 0.0; }
    };

    static_assert( sizeof( frotator ) == 0x18, "frotator must be 24 bytes (LWC)" );

    struct alignas( 16 ) fquat {
        double X, Y, Z, W;

        fquat( ) : X( 0.0 ), Y( 0.0 ), Z( 0.0 ), W( 1.0 ) { }
        fquat( double in_x, double in_y, double in_z, double in_w ) : X( in_x ), Y( in_y ), Z( in_z ), W( in_w ) { }

        fvector rotate_vector( const fvector& v ) const {
            const fvector q( X, Y, Z );
            const fvector t = q.cross( v ) * 2.0;
            return v + ( t * W ) + q.cross( t );
        }
    };

    static_assert( sizeof( fquat ) == 0x20, "fquat must be 32 bytes" );

    struct fmatrix {
        union {
            struct {
                double M11, M12, M13, M14;
                double M21, M22, M23, M24;
                double M31, M32, M33, M34;
                double M41, M42, M43, M44;
            };
            double M[ 4 ][ 4 ];
            struct {
                fplane XPlane, YPlane, ZPlane, WPlane;
            };
        };

        fmatrix( ) {
            M11 = 0; M12 = 0; M13 = 0; M14 = 0;
            M21 = 0; M22 = 0; M23 = 0; M24 = 0;
            M31 = 0; M32 = 0; M33 = 0; M34 = 0;
            M41 = 0; M42 = 0; M43 = 0; M44 = 0;
        }

        fvector transform_position( const fvector& v ) const {
            return {
                v.X * M11 + v.Y * M21 + v.Z * M31 + M41,
                v.X * M12 + v.Y * M22 + v.Z * M32 + M42,
                v.X * M13 + v.Y * M23 + v.Z * M33 + M43
            };
        }

        fvector4 transform_position4( const fvector& v ) const {
            return {
                v.X * M11 + v.Y * M21 + v.Z * M31 + M41,
                v.X * M12 + v.Y * M22 + v.Z * M32 + M42,
                v.X * M13 + v.Y * M23 + v.Z * M33 + M43,
                v.X * M14 + v.Y * M24 + v.Z * M34 + M44
            };
        }

        fmatrix operator*( const fmatrix& other ) const {
            fmatrix result;
            for ( i32 row = 0; row < 4; ++row ) {
                for ( i32 col = 0; col < 4; ++col ) {
                    result.M[ row ][ col ] =
                        M[ row ][ 0 ] * other.M[ 0 ][ col ] +
                        M[ row ][ 1 ] * other.M[ 1 ][ col ] +
                        M[ row ][ 2 ] * other.M[ 2 ][ col ] +
                        M[ row ][ 3 ] * other.M[ 3 ][ col ];
                }
            }
            return result;
        }

        static fmatrix make_from_rotator( const frotator& rotation ) {
            fmatrix matrix;

            const double pitch = rotation.Pitch * ( pi / 180.0 );
            const double yaw = rotation.Yaw * ( pi / 180.0 );
            const double roll = rotation.Roll * ( pi / 180.0 );

            const double sp = std::sin( pitch ), cp = std::cos( pitch );
            const double sy = std::sin( yaw ), cy = std::cos( yaw );
            const double sr = std::sin( roll ), cr = std::cos( roll );

            matrix.XPlane.X = cp * cy;
            matrix.XPlane.Y = cp * sy;
            matrix.XPlane.Z = sp;

            matrix.YPlane.X = sr * sp * cy - cr * sy;
            matrix.YPlane.Y = sr * sp * sy + cr * cy;
            matrix.YPlane.Z = -sr * cp;

            matrix.ZPlane.X = -( cr * sp * cy + sr * sy );
            matrix.ZPlane.Y = cy * sr - cr * sp * sy;
            matrix.ZPlane.Z = cr * cp;

            matrix.WPlane.W = 1.0;
            return matrix;
        }

        bool world_to_screen( const fvector& world, double screen_width, double screen_height, fvector2d& screen ) const {
            if ( screen_width <= 0.0 || screen_height <= 0.0 )
                return false;

            const fvector4 clip = transform_position4( world );
            if ( !std::isfinite( clip.W ) || clip.W <= 0.001 )
                return false;

            const double recip_w = 1.0 / clip.W;
            screen.X = ( clip.X * recip_w + 1.0 ) * 0.5 * screen_width;
            screen.Y = ( 1.0 - clip.Y * recip_w ) * 0.5 * screen_height;
            return std::isfinite( screen.X ) && std::isfinite( screen.Y );
        }
    };

    static_assert( sizeof( fmatrix ) == 0x80, "fmatrix must be 128 bytes" );

    struct alignas( 16 ) ftransform {
        fquat Rotation;
        fvector Translation;
        u8 Pad_38[ 0x8 ];
        fvector Scale3D;
        u8 Pad_58[ 0x8 ];

        ftransform( )
            : Rotation( ), Translation( ), Pad_38 { }, Scale3D( 1.0, 1.0, 1.0 ), Pad_58 { } {
        }

        fvector transform_position( const fvector& v ) const {
            const fvector scaled { v.X * Scale3D.X, v.Y * Scale3D.Y, v.Z * Scale3D.Z };
            return Rotation.rotate_vector( scaled ) + Translation;
        }

        fmatrix to_matrix_with_scale( ) const {
            fmatrix matrix;

            const double x2 = Rotation.X + Rotation.X;
            const double y2 = Rotation.Y + Rotation.Y;
            const double z2 = Rotation.Z + Rotation.Z;

            const double xx2 = Rotation.X * x2;
            const double yy2 = Rotation.Y * y2;
            const double zz2 = Rotation.Z * z2;
            const double yz2 = Rotation.Y * z2;
            const double wx2 = Rotation.W * x2;
            const double xy2 = Rotation.X * y2;
            const double wz2 = Rotation.W * z2;
            const double xz2 = Rotation.X * z2;
            const double wy2 = Rotation.W * y2;

            matrix.XPlane.X = ( 1.0 - ( yy2 + zz2 ) ) * Scale3D.X;
            matrix.XPlane.Y = ( xy2 + wz2 ) * Scale3D.X;
            matrix.XPlane.Z = ( xz2 - wy2 ) * Scale3D.X;

            matrix.YPlane.X = ( xy2 - wz2 ) * Scale3D.Y;
            matrix.YPlane.Y = ( 1.0 - ( xx2 + zz2 ) ) * Scale3D.Y;
            matrix.YPlane.Z = ( yz2 + wx2 ) * Scale3D.Y;

            matrix.ZPlane.X = ( xz2 + wy2 ) * Scale3D.Z;
            matrix.ZPlane.Y = ( yz2 - wx2 ) * Scale3D.Z;
            matrix.ZPlane.Z = ( 1.0 - ( xx2 + yy2 ) ) * Scale3D.Z;

            matrix.WPlane.X = Translation.X;
            matrix.WPlane.Y = Translation.Y;
            matrix.WPlane.Z = Translation.Z;
            matrix.WPlane.W = 1.0;
            return matrix;
        }
    };

    static_assert( sizeof( ftransform ) == 0x60, "ftransform must be 96 bytes" );

    inline fvector operator*( const ftransform& transform, const fvector& v ) {
        return transform.transform_position( v );
    }

    // cached view matrices read from UWorld::CachedViewInfo
    struct fworld_cached_view_info {
        fmatrix view_matrix;
        fmatrix projection_matrix;
        fmatrix view_projection_matrix;
        fmatrix view_to_world;

        bool world_to_screen( const fvector& world, double screen_width, double screen_height, fvector2d& screen ) const {
            return view_projection_matrix.world_to_screen( world, screen_width, screen_height, screen );
        }
    };

    static_assert( sizeof( fworld_cached_view_info ) == 0x200, "fworld_cached_view_info must be 512 bytes" );
}