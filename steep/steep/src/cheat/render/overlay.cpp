#include <impl/includes.h>

#include <cheat/render/overlay.h>

#include <cheat/cache/camera_cache.h>
#include <cheat/cache/player_cache.h>
#include <cheat/render/menu.h>
#include <cheat/visuals/players.h>

#include <deps/imgui/imgui_impl_win32.h>
#include <deps/imgui/imgui_impl_dx11.h>

namespace overlay {
    bool create_window( ) {
        const auto width = GetSystemMetrics( SM_CXSCREEN );
        const auto height = GetSystemMetrics( SM_CYSCREEN );

        window_class.cbSize = sizeof( WNDCLASSEXA );
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = wndproc;
        window_class.hInstance = GetModuleHandleA( oxorany( "steep-framework-on-imgui" ) );
        if ( !window_class.hInstance ) window_class.hInstance = GetModuleHandleA( nullptr );
        std::strcpy( class_name, oxorany( "steep_framework_on_imgui" ) );
        window_class.lpszClassName = class_name;
        window_class.hCursor = LoadCursor( nullptr, IDC_ARROW );

        if ( !RegisterClassExA( &window_class ) ) {
            if ( GetLastError( ) != ERROR_CLASS_ALREADY_EXISTS ) {
                logging::error( oxorany( "registerclass failed: 0x%08x" ), GetLastError( ) );
                return false;
            }
        }

        resources_t.hwnd = CreateWindowExA(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            window_class.lpszClassName, oxorany( "steep - framework" ), WS_POPUP,
            0, 0, width, height, nullptr, nullptr, window_class.hInstance, nullptr );
        if ( !resources_t.hwnd ) {
            logging::error( oxorany( "createwindowa failed: 0x%08x" ), GetLastError( ) );
            return false;
        }

        MARGINS margins{ -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea( resources_t.hwnd, &margins );
        SetLayeredWindowAttributes( resources_t.hwnd, RGB( 0, 0, 0 ), 0, LWA_COLORKEY );
        SetWindowPos( resources_t.hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE );
        ShowWindow( resources_t.hwnd, SW_SHOW );
        UpdateWindow( resources_t.hwnd );

        logging::print( oxorany( "overlay window -> 0x%llx (%dx%d)" ),
            reinterpret_cast< std::uint64_t >( resources_t.hwnd ), width, height );
        return true;
    }

    bool setup_d3d( ) {
        DXGI_SWAP_CHAIN_DESC swap_chain_desc{ };
        swap_chain_desc.BufferCount = 2;
        swap_chain_desc.BufferDesc.Width = 0;
        swap_chain_desc.BufferDesc.Height = 0;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.OutputWindow = resources_t.hwnd;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.Windowed = TRUE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        const D3D_FEATURE_LEVEL feature_levels[ ] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL selected_feature_level{ };
        ID3D11Device* tmp_device = nullptr;
        ID3D11DeviceContext* tmp_context = nullptr;
        IDXGISwapChain* tmp_swap_chain = nullptr;

        if ( FAILED( D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            feature_levels, _countof( feature_levels ), D3D11_SDK_VERSION,
            &swap_chain_desc, &tmp_swap_chain, &tmp_device,
            &selected_feature_level, &tmp_context ) ) ) {
            logging::error( oxorany( "d3d11 create failed" ) );
            return false;
        }

        resources_t.device = tmp_device;
        resources_t.device_context = tmp_context;
        resources_t.swap_chain = tmp_swap_chain;

        ID3D11Texture2D* back_buffer = nullptr;
        if ( FAILED( resources_t.swap_chain->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) ) ) || !back_buffer ) {
            logging::error( oxorany( "swap getbuffer failed" ) );
            return false;
        }

        if ( FAILED( resources_t.device->CreateRenderTargetView( back_buffer, nullptr, &resources_t.render_target_view ) ) ) {
            back_buffer->Release( );
            logging::error( oxorany( "rtv create failed" ) );
            return false;
        }
        back_buffer->Release( );

        ImGui::CreateContext( );
        ImGui::StyleColorsDark( );

        auto& io = ImGui::GetIO( );
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        resources_t.esp_font = io.Fonts->AddFontFromFileTTF(
            oxorany( "C:\\Windows\\Fonts\\consolab.ttf" ), 13.0f, nullptr, io.Fonts->GetGlyphRangesDefault( ) );

        ImGui_ImplWin32_Init( resources_t.hwnd );
        ImGui_ImplDX11_Init( resources_t.device, resources_t.device_context );

        tmp_device->Release( );
        logging::print( oxorany( "d3d11 ready" ) );
        return true;
    }

    void loop( ) {
        constexpr float clear_color[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
        MSG msg{ };
        bool last_menu_active = menu_active;

        set_clickthrough( !menu_active );

        while ( msg.message != WM_QUIT ) {
            while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }

            if ( GetAsyncKeyState( VK_INSERT ) & 1 ) {
                menu_active = !menu_active;
                if ( menu_active ) logging::print( oxorany( "menu shown" ) );
                else logging::print( oxorany( "menu hidden" ) );
            }

            if ( menu_active != last_menu_active ) {
                set_clickthrough( !menu_active );
                last_menu_active = menu_active;
            }

            auto& io = ImGui::GetIO( );
            io.DeltaTime = 1.0f / 60.0f;

            if ( menu_active ) {
                POINT cursor_pos{ };
                GetCursorPos( &cursor_pos );
                io.MousePos = ImVec2( static_cast< float >( cursor_pos.x ), static_cast< float >( cursor_pos.y ) );
                io.MouseDown[ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
            }
            else {
                io.MousePos = ImVec2( -1, -1 );
                io.MouseDown[ 0 ] = false;
            }

            ImGui_ImplDX11_NewFrame( );
            ImGui_ImplWin32_NewFrame( );
            ImGui::NewFrame( );

            auto* draw_list = ImGui::GetBackgroundDrawList( );

            float overlay_width = io.DisplaySize.x;
            float overlay_height = io.DisplaySize.y;
            if ( overlay_width <= 0.0f ) overlay_width = static_cast< float >( GetSystemMetrics( SM_CXSCREEN ) );
            if ( overlay_height <= 0.0f ) overlay_height = static_cast< float >( GetSystemMetrics( SM_CYSCREEN ) );

            if ( resources_t.esp_font ) ImGui::PushFont( resources_t.esp_font );
            cache::g_camera.update( cache::g_players.world( ) );
            visuals::draw_players( cache::g_players, cache::g_camera, draw_list, overlay_width, overlay_height );
            if ( resources_t.esp_font ) ImGui::PopFont( );

            menu( );

            ImGui::Render( );

            resources_t.device_context->OMSetRenderTargets( 1, &resources_t.render_target_view, nullptr );
            resources_t.device_context->ClearRenderTargetView( resources_t.render_target_view, clear_color );
            ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
            resources_t.swap_chain->Present( visuals::vsync ? 1 : 0, 0 );
        }

        ImGui_ImplDX11_Shutdown( );
        ImGui_ImplWin32_Shutdown( );
        ImGui::DestroyContext( );
    }

    bool run( ) {
        if ( !create_window( ) ) return false;
        if ( !setup_d3d( ) ) {
            logging::error( oxorany( "overlay setup failed" ) );
            return false;
        }
        logging::print( oxorany( "overlay running, insert toggles menu" ) );
        loop( );
        return true;
    }
}