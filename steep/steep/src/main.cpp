#include <impl/includes.h>
#include <core/loader/boot/boot.h>
#include <core/loader/boot/game.h>

int main( ) {
    SetConsoleTitleA( oxorany( "steep - framework" ) );

    HANDLE singleton = CreateMutexW( nullptr, FALSE, L"Global\\steep-framework-on-imgui-singleton" );
    if ( !singleton ) {
        logging::error( oxorany( "could not create singleton mutex: 0x%08X" ), GetLastError( ) );
        return std::getchar( );
    }
    if ( GetLastError( ) == ERROR_ALREADY_EXISTS ) {
        logging::error( oxorany( "steep - framework is already running, unload it first" ) );
        CloseHandle( singleton );
        return std::getchar( );
    }

    SetUnhandledExceptionFilter( crash::crash_handler );
    SetConsoleCtrlHandler(
        reinterpret_cast< PHANDLER_ROUTINE >( crash::unload_handler ),
        true
    );

    auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
    DWORD mode;
    GetConsoleMode( std_handle, &mode );
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode( std_handle, mode );

    CONSOLE_FONT_INFOEX cfi{ };
    cfi.cbSize = sizeof( cfi );
    cfi.nFont = 0;
    cfi.dwFontSize.X = 8;
    cfi.dwFontSize.Y = 15;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s( cfi.FaceName, oxorany( L"Raster Fonts" ) );
    SetCurrentConsoleFontEx( GetStdHandle( STD_OUTPUT_HANDLE ), FALSE, &cfi );

    int argc;
    auto argv = CommandLineToArgvW( GetCommandLineW( ), &argc );
    const auto is_system = argc > 1 && wcscmp( argv[ 1 ], L"--system" ) == 0;
    LocalFree( argv );

    if ( !is_system ) {
        logging::print( oxorany( "steep requires elevation, relaunching as SYSTEM..." ) );

        if ( !utility::is_admin( ) ) {
            logging::error( oxorany( "please run the application as admin!" ) );
            return std::getchar( );
        }

        if ( !g_authority->impersonate_system( ) ) {
            logging::error( oxorany( "could not elevate process." ) );
            return std::getchar( );
        }

        HANDLE h_thread_token = nullptr;
        if ( !OpenThreadToken( GetCurrentThread( ),
            TOKEN_DUPLICATE | TOKEN_QUERY, FALSE, &h_thread_token ) ) {
            logging::error( oxorany( "OpenThreadToken failed: 0x%08X" ), GetLastError( ) );
            return std::getchar( );
        }

        HANDLE h_primary = nullptr;
        if ( !DuplicateTokenEx( h_thread_token,
            TOKEN_ALL_ACCESS, nullptr,
            SecurityImpersonation, TokenPrimary, &h_primary ) ) {
            logging::error( oxorany( "DuplicateTokenEx failed: 0x%08X" ), GetLastError( ) );
            CloseHandle( h_thread_token );
            return std::getchar( );
        }
        CloseHandle( h_thread_token );

        wchar_t path[ MAX_PATH ];
        GetModuleFileNameW( nullptr, path, MAX_PATH );

        wchar_t cmd_line[ MAX_PATH + 16 ];
        swprintf_s( cmd_line, L"\"%s\" --system", path );

        STARTUPINFOW si{ sizeof( si ) };
        PROCESS_INFORMATION pi{ };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;

        if ( !CreateProcessWithTokenW( h_primary, LOGON_WITH_PROFILE,
            nullptr, cmd_line,
            CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi ) ) {
            logging::error( oxorany( "CreateProcessWithTokenW failed: 0x%08X" ), GetLastError( ) );
            CloseHandle( h_primary );
            return std::getchar( );
        }

        if ( !g_authority->lock_process_dacl( pi.hProcess ) ) {
            logging::error( oxorany( "could not lock process: 0x%08X" ), GetLastError( ) );
            CloseHandle( h_primary );
            return std::getchar( );
        }

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        CloseHandle( h_primary );
        return 0;
        //
    }

    logging::print( oxorany( "running as system process\n" ) );

    if ( !boot::run_all( ) )
        return std::getchar( );

    if ( !game::open( ) )
        return std::getchar( );

    if ( !game::start( ) )
        return std::getchar( );

    boot::restore( );
    return std::getchar( );
}