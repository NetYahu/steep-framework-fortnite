#pragma once

#include <memory>
#include <cstdint>

#include <core/loader/process/process.h>

namespace cheat {
    inline std::unique_ptr< process::c_process > g_game;
    inline std::uint64_t g_base = 0;

    template < typename t >
    t readx( std::uint64_t va ) {
        t value { };
        if ( !utility::is_valid_pointer( va ) ) return value;
        if ( !g_game ) return value;
        g_game->read_memory( va, &value, sizeof( t ) );
        return value;
    }

    template < typename t >
    bool writex( std::uint64_t va, t value ) {
        if ( !utility::is_valid_pointer( va ) ) return false;
        if ( !g_game ) return false;
        return g_game->write_memory( va, &value, sizeof( t ) );
    }
}