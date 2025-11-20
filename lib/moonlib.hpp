#pragma once
#include <random>
#include <Windows.h>
#include <map>
#include <string>
#include <cmath>
#include <cstdio>
#include <d3d11.h>
#include <dxgi.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_internal.h"
#include "fonts.h"

#pragma comment(lib, "d3d11.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator*(const ImVec2& a, float f) { return ImVec2(a.x * f, a.y * f); }
inline ImVec2 operator*(float f, const ImVec2& a) { return ImVec2(f * a.x, f * a.y); }

namespace fonts {
    inline ImFont* main = nullptr;
    inline ImFont* main_bold = nullptr;
}

namespace utils {
    std::string getrandomstring(int length = 64) {
        static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
        std::string str;
        str.reserve(length);
        for (int i = 0; i < length; ++i)
            str.push_back(charset[dist(gen)]);
        return str;
    }
}

namespace window {
    IDXGISwapChain* swapchain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    HWND hwnd = nullptr;
    WNDCLASSEXW wc{};

    void begin_frame() {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
	}
    void render_frame() {
        ImGui::Render();
        const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
        context->OMSetRenderTargets(1, &rtv, nullptr);
        context->ClearRenderTargetView(rtv, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swapchain->Present(1, 0);
    }
    void end_frame() {
		// honestly just used for style consistency
        // will actually need this when i use a custom engine
	}
    void destroy() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        cleanup();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}

    void cleanup() {
        if (rtv) rtv->Release();
        if (swapchain) swapchain->Release();
        if (context) context->Release();
        if (device) device->Release();
    }

    bool create_device(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
            D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &context)))
            return false;

        ID3D11Texture2D* bb;
        swapchain->GetBuffer(0, IID_PPV_ARGS(&bb));
        device->CreateRenderTargetView(bb, nullptr, &rtv);
        bb->Release();
        return true;
    }

    LRESULT WINAPI wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

        if (msg == WM_SIZE && device && wParam != SIZE_MINIMIZED) {
            if (rtv) { rtv->Release(); rtv = nullptr; }
            swapchain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* bb;
            swapchain->GetBuffer(0, IID_PPV_ARGS(&bb));
            device->CreateRenderTargetView(bb, nullptr, &rtv);
            bb->Release();
        }
        if (msg == WM_DESTROY) PostQuitMessage(0);
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void init() {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        ImFontConfig cfg;
        cfg.OversampleH = cfg.OversampleV = 5;
        cfg.RasterizerMultiply = 1.2f;

        static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0400, 0x052F, 0x2DE0, 0x2DFF, 0xA640, 0xA69F, 0xE000, 0xE226, 0 };

        fonts::main = io.Fonts->AddFontFromMemoryTTF(InterMedium, sizeof(InterMedium), 15.0f, &cfg, ranges);
        fonts::main_bold = io.Fonts->AddFontFromMemoryTTF(InterSemiBold, sizeof(InterSemiBold), 17.0f, &cfg, ranges);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.f;
        style.ChildRounding = 4.f;
        style.FrameRounding = 4.f;
        style.PopupRounding = 4.f;
        style.ScrollbarRounding = 4.f;
        style.GrabRounding = 4.f;
        style.TabRounding = 4.f;
        style.Alpha = 1.0f;

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(device, context);
    }

    void create(HINSTANCE hInst) {
        std::string rndClass = utils::getrandomstring();
        std::string rndTitle = utils::getrandomstring();
        std::wstring wClass(rndClass.begin(), rndClass.end());
        std::wstring wTitle(rndTitle.begin(), rndTitle.end());

        wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, wnd_proc, 0, 0, hInst, nullptr, nullptr, nullptr, nullptr, wClass.c_str(), nullptr };
        RegisterClassExW(&wc);

        RECT rc; GetClientRect(GetDesktopWindow(), &rc);
        hwnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, wTitle.c_str(),
            WS_POPUP, 0, 0, rc.right, rc.bottom, nullptr, nullptr, hInst, nullptr);

        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        if (!create_device(hwnd)) {
            cleanup();
            return;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        init();
    }
}

class c_widgets {
    struct anim_t {
        float hover = 0.f;
        float value = 0.f;
    };

    static inline std::map<ImGuiID, anim_t> anim;
    static inline std::map<std::string, float> progress;

    static float get_progress(const char* id, float duration = 0.25f, bool reset = false) {
        std::string sid = id;
        if (reset || progress.find(sid) == progress.end())
            progress[sid] = 0.f;
        progress[sid] = ImMin(progress[sid] + ImGui::GetIO().DeltaTime, duration);
        return progress[sid] / duration;
    }

public:
    static bool tab(const char* name, bool selected) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiID id = window->GetID(name);
        ImVec2 label_size = ImGui::CalcTextSize(name, nullptr, true);
        ImVec2 pos = window->DC.CursorPos;
        ImRect rect(pos, pos + ImVec2(label_size.x + 20, label_size.y + 16));

        ImGui::ItemSize(rect, 0);
        if (!ImGui::ItemAdd(rect, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);

        auto& a = anim[id];
        float target = selected ? 1.0f : hovered ? 0.7f : 0.4f;
        a.hover = ImLerp(a.hover, target, ImGui::GetIO().DeltaTime * 12.f);

        ImU32 col = ImColor(1.0f, 1.0f, 1.0f, a.hover);

        if (selected) {
            window->DrawList->AddRectFilled(rect.Min - ImVec2(0, 4), rect.Max + ImVec2(0, 4), ImColor(100, 170, 255, 120), 4.0f);
            window->DrawList->AddText(rect.Min + ImVec2(10, 6), IM_COL32(100, 170, 255, 255), name);
        }
        else {
            window->DrawList->AddText(rect.Min + ImVec2(10, 6), col, name);
        }

        return pressed;
    }

    static bool subtab(const char* name, bool selected) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiID id = window->GetID(name);
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImVec2(110, 34);
        ImRect rect(pos, pos + size);

        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(rect, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);

        auto& a = anim[id];
        float target = selected ? 1.0f : hovered ? 0.8f : 0.5f;
        a.hover = ImLerp(a.hover, target, ImGui::GetIO().DeltaTime * 14.f);

        window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(40, 40, 45, (int)(255 * a.hover)), 5.0f);
        window->DrawList->AddText(rect.Min + ImVec2(12, 8), ImColor(255, 255, 255, (int)(255 * a.hover)), name);

        return pressed;
    }

    static void progress_circle(const char* id, float fraction, const ImVec2& center, float radius, ImU32 col = IM_COL32(100, 170, 255, 255), float thickness = 4.5f) {
        fraction = ImClamp(fraction, 0.f, 1.f);
        float t = get_progress(id, 0.3f);
        float draw_frac = ImLerp(0.f, fraction, t < 0.5f ? 2.f * t * t : -1.f + (4.f - 2.f * t) * t);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const int segs = 64;
        const float a_max = draw_frac * IM_PI * 2.f;

        draw->PathClear();
        for (int i = 0; i <= segs; ++i) {
            float a = (i / (float)segs) * a_max;
            draw->PathLineTo(center + ImVec2(cosf(a), sinf(a)) * radius);
        }
        draw->PathStroke(col, false, thickness);
    }
};

class c_draw {
public:
    ImDrawList* draw = ImGui::GetWindowDrawList();

    void outlined_text(ImFont* font, float size, ImVec2 pos, ImU32 color, const char* text) {
        draw->AddText(font, size, pos + ImVec2(1, 1), IM_COL32(0, 0, 0, color >> 24), text);
        draw->AddText(font, size, pos + ImVec2(-1, -1), IM_COL32(0, 0, 0, color >> 24), text);
        draw->AddText(font, size, pos + ImVec2(1, -1), IM_COL32(0, 0, 0, color >> 24), text);
        draw->AddText(font, size, pos + ImVec2(-1, 1), IM_COL32(0, 0, 0, color >> 24), text);
        draw->AddText(font, size, pos, color, text);
    }

    void filled_rect(ImVec2 a, ImVec2 b, ImU32 col, float rounding = 0.f) {
        draw->AddRectFilled(a, b, col, rounding);
        draw->AddRect(a - ImVec2(1, 1), b + ImVec2(1, 1), IM_COL32(0, 0, 0, 150), rounding);
        draw->AddRect(a - ImVec2(2, 2), b + ImVec2(2, 2), IM_COL32(0, 0, 0, 80), rounding);
    }

    void circle(ImVec2 center, float radius, ImU32 col, float thickness = 1.5f) {
        draw->AddCircle(center, radius, col, 64, thickness);
    }
};