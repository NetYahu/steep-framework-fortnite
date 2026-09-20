#pragma once

namespace memory {
    extern std::function<bool( std::uint64_t, void*, std::size_t )> m_read_physical;
    extern std::function<bool( std::uint64_t, void*, std::size_t )> m_write_physical;

    inline bool read_physical( std::uint64_t addr, void* buffer, std::size_t size ) {
        if ( !utility::is_physical_address_valid( addr, size ) )
            return false;
        return m_read_physical( addr, buffer, size );
    }

    inline bool write_physical( std::uint64_t addr, void* buffer, std::size_t size ) {
        if ( !utility::is_physical_address_valid( addr, size ) )
            return false;
        return m_write_physical( addr, buffer, size );
    }

    inline bool read_virtual( std::uint64_t addr, void* buffer, std::size_t size ) {
        if ( !buffer || !size )
            return false;
        auto* dst = static_cast< std::uint8_t* >( buffer );
        std::size_t offset = 0;
        while ( offset < size ) {
            const auto cur_va = addr + offset;
            const std::size_t page_remain = 0x1000 - ( cur_va & 0xFFF );
            const std::size_t chunk = ( size - offset < page_remain ) ? ( size - offset ) : page_remain;

            auto va = g_paging->translate_linear( cur_va );
            if ( !va ) return false;

            if ( !read_physical( va, dst + offset, chunk ) )
                return false;
            offset += chunk;
        }
        return true;
    }

    inline bool write_virtual( std::uint64_t addr, void* buffer, std::size_t size ) {
        if ( !buffer || !size )
            return false;
        auto* src = static_cast< std::uint8_t* >( buffer );
        std::size_t offset = 0;
        while ( offset < size ) {
            const auto cur_va = addr + offset;
            const std::size_t page_remain = 0x1000 - ( cur_va & 0xFFF );
            const std::size_t chunk = ( size - offset < page_remain ) ? ( size - offset ) : page_remain;

            auto va = g_paging->translate_linear( cur_va );
            if ( !va ) return false;

            if ( !write_physical( va, src + offset, chunk ) )
                return false;
            offset += chunk;
        }
        return true;
    }

    template <typename ret_t = std::uint64_t, typename addr_t>
    ret_t read( addr_t va ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> )
            va64 = reinterpret_cast< std::uint64_t >( va );
        else if constexpr ( std::is_integral_v<addr_t> )
            va64 = static_cast< std::uint64_t >( va );
        else
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                "addr_t must be pointer or integral" );

        ret_t ret{};
        read_virtual( va64, &ret, sizeof( ret ) );
        return ret;
    }

    template <typename val_t, typename addr_t>
    bool write( addr_t va, val_t val ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> )
            va64 = reinterpret_cast< std::uint64_t >( va );
        else if constexpr ( std::is_integral_v<addr_t> )
            va64 = static_cast< std::uint64_t >( va );
        else
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                "addr_t must be pointer or integral" );

        return write_virtual( va64, &val, sizeof( val_t ) );
    }
}