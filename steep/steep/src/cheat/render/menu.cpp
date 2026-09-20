#include <impl/includes.h>

#include <cheat/render/menu.h>

#include <cheat/cache/camera_cache.h>
#include <cheat/cache/player_cache.h>
#include <cheat/visuals/players.h>

#include <deps/imgui/imgui.h>

namespace overlay {
    void menu( ) {
        if ( !menu_active ) return;

        ImGui::Begin( oxorany( "steep - framework" ), nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus );
        ImGui::SetWindowSize( ImVec2( 420, 300 ), ImGuiCond_FirstUseEver );

        ImGui::Text( oxorany( "ESP" ) );
        ImGui::Checkbox( oxorany( "boxes" ), &visuals::boxes );
        ImGui::Checkbox( oxorany( "skeleton" ), &visuals::skeleton );
        ImGui::Checkbox( oxorany( "self" ), &visuals::self );
        ImGui::Checkbox( oxorany( "vsync" ), &visuals::vsync );

        ImGui::End( );
    }
}