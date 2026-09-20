#pragma once

namespace boot {
    inline bool use_section = true;

    inline bool verify_dacl( ) {
        logging::print( oxorany( "[1/9] verifying access control list lock..." ) );
        if ( !utility::is_dacl_locked( GetCurrentProcess( ) ) ) {
            logging::error( oxorany( "could not verify dacl lock" ) );
            return false;
        }
        logging::print( oxorany( "      dacl lock verified" ) );
        return true;
    }

    inline bool load_pdb( ) {
        logging::print( oxorany( "[2/9] downloading ntoskrnl pdb..." ) );
        if ( !g_pdb->load( ) ) {
            logging::error( oxorany( "could not load pdb" ) );
            return false;
        }
        logging::print( oxorany( "      loaded %zu symbols from %s" ),
            g_pdb->m_symbols.size( ), g_pdb->m_module_bare.c_str( ) );

        if ( !utility::has_system_token( ) ) {
            logging::error( oxorany( "could not verify authority" ) );
            return false;
        }
        logging::print( oxorany( "      verified process authority" ) );
        return true;
    }

    inline bool create_service( ) {
        logging::print( oxorany( "[3/9] installing vulnerable driver service..." ) );
        if ( !g_service->create( ) ) {
            logging::error( oxorany( "could not setup service" ) );
            g_service->unload( );
            return false;
        }
        return true;
    }

    inline bool open_driver( ) {
        logging::print( oxorany( "[4/9] opening driver handle..." ) );
        if ( !g_driver->initialize( ) ) {
            logging::error( oxorany( "could not setup driver" ) );
            g_service->unload( );
            return false;
        }
        return true;
    }

    inline bool cleanup_image( ) {
        logging::print( oxorany( "[5/9] wiping driver image from disk..." ) );
        if ( !g_service->cleanup( ) ) {
            logging::error( oxorany( "could not clean driver" ) );
            g_service->unload( );
            return false;
        }
        return true;
    }

    inline bool setup_paging( ) {
        logging::print( oxorany( "[6/9] locating directory table base..." ) );
        if ( !g_paging->setup( ) ) {
            logging::error( oxorany( "could not setup paging" ) );
            g_driver->unload( );
            g_service->unload( );
            return false;
        }
        return true;
    }

    inline bool setup_syscall( ) {
        logging::print( oxorany( "[7/9] patching syscall entry..." ) );
        if ( !g_syscall->setup( ) ) {
            logging::error( oxorany( "could not setup syscall" ) );
            g_driver->unload( );
            g_service->unload( );
            return false;
        }
        return true;
    }

    inline bool setup_physical( ) {
        logging::print( oxorany( "[8/9] opening physicalmemory and allocating mappings..." ) );
        use_section = true;
        if ( !g_physical->setup( ) ) {
            logging::warn( oxorany( "section open failed, falling back to driver-backed phys r/w" ) );
            use_section = false;
        }

        if ( use_section ) {
            logging::print( oxorany( "[9/9] unloading vulnerable driver..." ) );
            g_service->unload( );
            g_driver->unload( );

            logging::print( oxorany( "installing physical memory read/write callbacks" ) );
            memory::m_read_physical = [ ]( std::uint64_t addr, void* buffer, std::size_t size ) -> bool {
                return g_physical->read( addr, buffer, size );
                };
            memory::m_write_physical = [ ]( std::uint64_t addr, void* buffer, std::size_t size ) -> bool {
                return g_physical->write( addr, buffer, size );
                };
        }
        else {
            logging::warn( oxorany( "[9/9] keeping vulnerable driver for phys r/w..." ) );
        }
        return true;
    }

    inline bool protect_self( ) {
        const auto process_id = GetCurrentProcessId( );
        auto our_process = std::make_unique< process::c_process >( process_id );
        if ( !our_process->enable_ppl( ) ) {
            logging::error( oxorany( "could not enable protection" ) );
            g_physical->unload( );
            return false;
        }

        if ( use_section ) {
            const auto handle = g_physical->get_handle( );
            if ( !g_physical->hide_handle( handle ) ) {
                logging::error( oxorany( "could not hide handle" ) );
                g_physical->unload( );
                return false;
            }
        }
        return true;
    }

    inline bool open_self( ) {
        logging::print( oxorany( "resolving own process page tables..." ) );
        if ( !g_process->open_self( ) ) {
            logging::error( oxorany( "could not open own process" ) );
            g_physical->unload( );
            return false;
        }
        logging::print( oxorany( "own process opened: pid=%u eprocess=0x%llx" ),
            g_process->pid( ), g_process->eprocess( ) );
        return true;
    }

    inline bool run_all( ) {
        if ( !verify_dacl( ) ) return false;
        if ( !load_pdb( ) ) return false;
        if ( !create_service( ) ) return false;
        if ( !open_driver( ) ) return false;
        if ( !cleanup_image( ) ) return false;
        if ( !setup_paging( ) ) return false;
        if ( !setup_syscall( ) ) return false;
        if ( !setup_physical( ) ) return false;
        if ( !protect_self( ) ) return false;
        if ( !open_self( ) ) return false;
        return true;
    }

    inline void restore( ) {
        logging::warn( oxorany( "restoring mappings" ) );
        if ( use_section ) g_physical->restore( );
        else {
            g_driver->unload( );
            g_service->unload( );
        }
    }
}