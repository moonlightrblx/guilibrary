#pragma once
#include <random>
#include <Windows.h>
#include <map>
#include <string>
#include <cmath>
#include <cstdio>       

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
inline ImVec2 operator*(float f, const ImVec2& a) { return ImVec2(a.x * f, a.y * f); }

namespace fonts {
    inline ImFont* main = nullptr;
    inline ImFont* main2 = nullptr;
}
namespace utils {
    std::string getrandomstring(int length = 64) {
        static const char charset[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

        std::string str;
        str.reserve(length);
        for (int i = 0; i < length; i++)
            str.push_back(charset[dist(gen)]);

        return str;
    }
}
namespace window {
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    ID3D11RenderTargetView* rtv;
    
    HWND hwnd;
    WNDCLASSEXW wc;

    void cleanup() {
        if (rtv) rtv->Release();
        if (swapchain) swapchain->Release();
        if (context) context->Release();
        if (device) device->Release();
    }

    bool create_device(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 600;
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


    LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return true;
        if (msg == WM_SIZE && device && wparam != SIZE_MINIMIZED) {
            if (rtv) rtv->Release();
            swapchain->ResizeBuffers(0, LOWORD(lparam), HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* bb; 
            swapchain->GetBuffer(0, IID_PPV_ARGS(&bb));
            device->CreateRenderTargetView(bb, nullptr, &rtv); bb->Release();
        }
        if (msg == WM_DESTROY) PostQuitMessage(0);
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void init() {
        ImGuiIO& io = ImGui::GetIO();;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImFontConfig font_config;
        font_config.PixelSnapH = false;
        font_config.OversampleH = 5;
        font_config.OversampleV = 5;
        font_config.RasterizerMultiply = 1.2f;

        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B
            0xE000, 0xE226, // icons
            0,
        };

        font_config.GlyphRanges = ranges;

        fonts::main = io.Fonts->AddFontFromMemoryTTF(InterMedium, sizeof(InterMedium), 15.0f, &font_config, ranges);
        fonts::main2 = io.Fonts->AddFontFromMemoryTTF(InterSemiBold, sizeof(InterSemiBold), 17.0f, &font_config, ranges);


        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.f; 
        style.ChildRounding = 4.f; 
        style.FrameRounding = 4.f;
        style.PopupRounding = 4.f;
        style.ScrollbarRounding = 4.f;
        style.GrabRounding = 4.f; 
        style.TabRounding = 4.f;
    }

    void create_window(HINSTANCE hInst) {
        std::string rndClass = utils::getrandomstring();
        std::string rndTitle = utils::getrandomstring();

        std::wstring wClass(rndClass.begin(), rndClass.end());
        std::wstring wTitle(rndTitle.begin(), rndTitle.end());


       wc = {
            sizeof(WNDCLASSEXW),
            CS_CLASSDC,
            window::wnd_proc,
            0, 0,
            hInst,
            nullptr, nullptr, nullptr, nullptr,
            wClass.c_str(),
            nullptr
        };

        RegisterClassEx(&wc);
        RECT rc; GetClientRect(GetDesktopWindow(), &rc);
        hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            wc.lpszClassName,
            wTitle.c_str(),
            WS_POPUP,
            0, 0,
            rc.right, rc.bottom,
            nullptr, nullptr,
            wc.hInstance,
            nullptr
        );
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        if (!window::create_device(hwnd)) 
        { 
            window::cleanup(); 
            return; 
        }
        
        ShowWindow(hwnd, SW_SHOWDEFAULT); 
        UpdateWindow(hwnd);
      
    }
}


struct tab_element {
    float element_opacity;
};

namespace widgets {
    template<typename T>
    constexpr T clamp(T v, T lo, T hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    template<typename T>
    constexpr T lerp(T a, T b, float t) noexcept { return a + (b - a) * t; }

    inline float smoothstep(float e0, float e1, float x) noexcept {
        x = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
        return x * x * (3.f - 2.f * x);
    }

    inline float ease_in_out(float t) noexcept {
        return t < .5f ? 2.f * t * t : -1.f + (4.f - 2.f * t) * t;
    }


    inline void fmt(char* buf, size_t buf_size, const char* format, ...) {
        va_list args;
        va_start(args, format);
        vsnprintf(buf, buf_size, format, args);
        va_end(args);
        buf[buf_size - 1] = '\0';
    }


    struct progressdata { std::string id; float current_frame = 0.f; };
    static std::map <ImGuiID, tab_element> anim;
    inline std::map<std::string, progressdata> animations;

    inline float get_anim(const char* id, float duration = 0.25f, bool reset = false) {
        std::string sid = id;
        auto it = animations.find(sid);
        if (it == animations.end() || reset) {
            animations[sid] = { sid, 0.f };
            it = animations.find(sid);
        }
        progressdata& a = it->second;
        float dt = ImGui::GetIO().DeltaTime;
        a.current_frame = clamp(a.current_frame + dt, 0.f, duration);
        return a.current_frame / duration;               // 0..1
    }

    inline ImDrawList* dl() { return ImGui::GetWindowDrawList(); }


    inline void progresscircle(const char* id, float fraction,
        const ImVec2& center, float radius,
        ImU32 col = IM_COL32(100, 170, 255, 255),
        float thickness = 4.f) {
        fraction = clamp(fraction, 0.f, 1.f);
        float t = get_anim(id, 0.3f);
        float draw_frac = lerp(0.f, fraction, ease_in_out(t));

        ImDrawList* draw = dl();
        const int segs = 64;
        const float a_max = draw_frac * IM_PI * 2.f;

        draw->PathClear();
        for (int i = 0; i <= segs; ++i) {
            float a = (i / (float)segs) * a_max;
            draw->PathLineTo(ImVec2(center.x + cosf(a) * radius,
                center.y + sinf(a) * radius));
        }
        draw->PathStroke(col, false, thickness);
    }

    inline bool tab(const char* name, bool boolean)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(name);
        const ImVec2 label_size = ImGui::CalcTextSize(name, NULL, true);
        ImVec2 pos = window->DC.CursorPos;

        const ImRect rect(pos, ImVec2(pos.x + label_size.x, pos.y + label_size.y));
        ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x + 10, rect.Max.y), style.FramePadding.y);
        if (!ImGui::ItemAdd(rect, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);


        auto it_anim = anim.find(id);
        if (it_anim == anim.end()) {
            anim.insert({ id, { 0.0f } });
            it_anim = anim.find(id);
        }

        it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, (boolean ? 0.8f : hovered ? 0.6f : 0.4f), 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

        window->DrawList->AddText(rect.Min, ImColor(1.0f, 1.0f, 1.0f, it_anim->second.element_opacity), name);

        return pressed;
    }
}

class c_draw {
public:
	ImDrawList* draw;
    c_draw(ImDrawList* draw_list) : draw(draw_list) {}

    __forceinline void draw_outlined_text(ImFont* font, float size, ImVec2 pos, ImColor color, const char* text){
		draw->AddText(font, size, ImVec2(pos.x + 1, pos.y), IM_COL32(0, 0, 0, color.Value.w * 255), text);
		draw->AddText(font, size, ImVec2(pos.x - 1, pos.y), IM_COL32(0, 0, 0, color.Value.w * 255), text);
		draw->AddText(font, size, ImVec2(pos.x, pos.y + 1), IM_COL32(0, 0, 0, color.Value.w * 255), text);
		draw->AddText(font, size, ImVec2(pos.x, pos.y - 1), IM_COL32(0, 0, 0, color.Value.w * 255), text);
		draw->AddText(font, size, pos, color, text);
    }

    __forceinline void draw_centered_text(ImFont* font, float size, ImVec2 pos, ImColor color, const char* text) {
        ImVec2 text_size = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
        ImVec2 centered_pos = ImVec2(pos.x - text_size.x / 2.0f, pos.y);
        draw->AddText(font, size, centered_pos, color, text);
	}

    __forceinline void draw_circle(ImVec2 center, float radius, ImU32 color, float thickness = 1.0f) {
        const int segments = 100;
        draw->AddCircle(center, radius, color, segments, thickness);
	}

    
    __forceinline void draw_filled_rect(ImVec2 top_left, ImVec2 bottom_right, ImU32 color) {
        draw->AddRectFilled(top_left, bottom_right, color);
        draw->AddRect(top_left, bottom_right, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
        draw->AddRect(top_left - ImVec2(1, 1), bottom_right + ImVec2(1, 1), IM_COL32(0, 0, 0, 150), 0.0f, 0, 1.0f);
        draw->AddRect(top_left - ImVec2(2, 2), bottom_right + ImVec2(2, 2), IM_COL32(0, 0, 0, 100), 0.0f, 0, 1.0f);
    }
};