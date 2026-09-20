#include <impl/includes.h>

#include <cheat/unreal/classes.h>

namespace unreal {
    bool UObject::read_raw( uptr source, void* destination, u64 size ) {
        if ( !destination || !size )
            return false;
        if ( !cheat::g_game )
            return false;
        return cheat::g_game->read_memory(
            static_cast< std::uint64_t >( source ),
            destination,
            static_cast< std::size_t >( size ) );
    }
}