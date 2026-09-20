 #pragma once

#include <cheat/render/overlay.h>
#include <cheat/sdk.h>
#include <cheat/updater.h>

namespace game {
    inline bool open( ) {
        logging::print( oxorany( "opening fortnite..." ) );
        cheat::g_game = std::make_unique< process::c_process >( std::wstring( oxorany( L"FortniteClient-Win64-Shipping.exe" ) ) );
        if ( !cheat::g_game->open( ) ) {
            logging::error( oxorany( "fortnite not found, open fortnite and sit in lobby" ) );
            logging::warn( oxorany( "cheat waiting for process" ) );
            return false;
        }

        cheat::g_base = cheat::g_game->base_address( );
        logging::print( oxorany( "fortnite pid=%u eprocess=0x%llx base=0x%llx" ),
            cheat::g_game->pid( ), cheat::g_game->eprocess( ), cheat::g_base );
        if ( !cheat::g_base ) {
            logging::error( oxorany( "fortnite base null" ) );
            return false;
        }
        return true;
    }

    inline bool start( ) {
        logging::print( oxorany( "starting updater thread" ) );
        std::thread( &updater::run ).detach( );

        logging::print( oxorany( "creating overlay window" ) );
        if ( !overlay::run( ) ) {
            logging::error( oxorany( "overlay failed" ) );
            return false;
        }
        return true;
    }
}