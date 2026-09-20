#pragma once

#include <cstring>

#include <d3d11.h>
#include <dwmapi.h>

#include <deps/imgui/imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param );

namespace overlay {
    struct resources {
        HWND hwnd = nullptr;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* device_context = nullptr;
        ID3D11RenderTargetView* render_target_view = nullptr;
        IDXGISwapChain* swap_chain = nullptr;
        ImFont* esp_font = nullptr;
    } inline resources_t;

    inline WNDCLASSEXA window_class{ };
    inline char class_name[ 64 ]{ };

    inline LRESULT __stdcall wndproc( HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param ) {
        if ( ImGui_ImplWin32_WndProcHandler( hwnd, msg, w_param, l_param ) ) return 1L;
        if ( msg == WM_DESTROY ) {
            PostQuitMessage( 0 );
            return 0L;
        }
        return DefWindowProcA( hwnd, msg, w_param, l_param );
    }

    inline void set_clickthrough( bool enabled ) {
        LONG_PTR ex_style = GetWindowLongPtrA( resources_t.hwnd, GWL_EXSTYLE );
        if ( enabled ) ex_style |= WS_EX_TRANSPARENT;
        else ex_style &= ~WS_EX_TRANSPARENT;
        SetWindowLongPtrA( resources_t.hwnd, GWL_EXSTYLE, ex_style );
    }

    bool create_window( );
    bool setup_d3d( );
    void loop( );
    bool run( );
}