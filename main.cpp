#include <windows.h>
#include <d3d11.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <string_view>
#include "lib/moonlib.hpp"
#include <random>
#include "settings.h"

void gui_render() {
    // render watermarks here 
	auto draw = ImGui::GetForegroundDrawList();
	c_draw drawing(draw);
	drawing.outlined_text(fonts::main, 20.0f, ImVec2(10, 10), ImColor(255, 255, 255, 200), settings::PRODUCT_NAME);
	drawing.outlined_text(fonts::main, 20.0f, ImVec2(10, 30), ImColor(255, 255, 255, 200), ("FPS: " + std::to_string((int)ImGui::GetIO().Framerate)).c_str());
    
    // render menu here
    if (settings::show_menu) {
        ImGui::SetNextWindowSize({ 500, 370 });

        ImGui::Begin(settings::PRODUCT_NAME, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();

        draw->AddRectFilled(pos, ImVec2(pos.x - 5 + size.x, pos.y + 51), ImColor(24, 24, 24), 9.0f, ImDrawFlags_RoundCornersTop);
        draw->AddRectFilledMultiColorRounded(pos, ImVec2(pos.x + 55, pos.y + 51), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.05f), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.05f), 9.0f, ImDrawFlags_RoundCornersTopLeft);

        draw->AddText(fonts::main_bold, 17.0f, ImVec2(pos.x + ImGui::CalcTextSize(settings::PRODUCT_NAME).x - 60, pos.y + 18), ImColor(192, 203, 229), settings::PRODUCT_NAME);

        ImGui::SetCursorPos({ 125, 20 });

        ImGui::BeginGroup(); {
            if (c_widgets::tab("Features", settings::current_tab == 0)) settings::current_tab = 0;
            ImGui::SameLine();
            if (c_widgets::tab("Info", settings::current_tab == 1)) settings::current_tab = 1;
            ImGui::SameLine();
            if (c_widgets::tab("Settings", settings::current_tab == 2)) settings::current_tab = 2;
        }
        ImGui::EndGroup();

        ImGui::SetCursorPos({ 25, 85 });

        ImGui::BeginChild("##container", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {

            if (settings::current_tab == 0) {
                c_widgets::progress_circle("progress1", settings::slider_val / 100.f, ImVec2(pos.x + 120, pos.y + 220), 40.f, IM_COL32(100, 170, 255, 255), 6.f);
                draw->AddText(fonts::main, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Tab header 1");
                ImGui::Checkbox("Option 1", &settings::option_1);
                ImGui::SliderFloat("Progress Bar", &settings::slider_val, 0.f, 100.f);
            }
            if (settings::current_tab == 1) {
                draw->AddText(fonts::main, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Tab header 2");
                if (ImGui::Button("Exit")) exit(0);
            }
            if (settings::current_tab == 2) {
                draw->AddText(fonts::main, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Tab header 3");
                ImGui::Text("dx11 full-screen transparent overlay");
                ImGui::Text("Window size: %dx%d", settings::WINDOW_W, settings::WINDOW_H);
                ImGui::Text("Product Name: %s", settings::PRODUCT_NAME);
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            }
        }
        ImGui::EndChild();
        ImGui::End();
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
   

    ImGui::CreateContext();

    window::init();
    window::create(hInst);

    ImGui_ImplWin32_Init(window:: hwnd);
    ImGui_ImplDX11_Init(window::device, window::context);

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (GetAsyncKeyState(VK_END) & 1) { exit(0); }
        if (GetAsyncKeyState(VK_INSERT) & 1) settings::show_menu = !settings::show_menu;

		window::begin_frame();

        gui_render();

		window::render_frame();

        window::end_frame();
    }


    window::destroy();
  
    return 0;
}
