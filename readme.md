<p align="center">
  <img src="assets/logo.png" width="140" />
</p>

<h1 align="center">Moonlight</h1>

<p align="center">
  <b>A tiny, header-only C++ GUI framework built by ellii.</b><br>
  Create a window + render a full GUI in under 100 lines of code.
</p>

---

## ✨ Overview

**Moonlight** is a **header-only**, **Windows/DX11-based** GUI helper designed to make ImGui window creation trivial.
It removes all boilerplate: window class setup, DX11 device creation, swapchains, render targets, cleanup, etc.

Just include the header(s), call the init functions, and render with your own ImGui code.

A full rewrite that removes the ImGui dependency entirely is planned.

---

## 🚀 Features

* **Header-only** — drop in and use immediately
* **Zero setup** — no project files, no linking
* **DX11 backend** (Win32 only)
* **Built for ImGui** (for now)
* **Minimal API** — simple window creation and destruction
* **Clean structure** — handles swapchain, device, context, RTV, WNDPROC

---

## 📦 Requirements

* **Windows**
* **DirectX 11**
* **Dear ImGui**
* Compiler with **C++17** support

---

## 🔧 Example Usage

Below is a complete runnable example using Moonlight to create a DX11 window and run an ImGui render loop:

```cpp
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {

    ImGui::CreateContext();

    window::init();
    window::create_window(hInst);

    ImGui_ImplWin32_Init(window::hwnd);
    ImGui_ImplDX11_Init(window::device, window::context);

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }

        if (GetAsyncKeyState(VK_END) & 1)
            exit(0);

        if (GetAsyncKeyState(VK_INSERT) & 1)
            settings::show_menu = !settings::show_menu;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        gui_render(); // your imgui rendering function

        ImGui::Render();

        const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
        window::context->OMSetRenderTargets(1, &window::rtv, nullptr);
        window::context->ClearRenderTargetView(window::rtv, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        window::swapchain->Present(0, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    window::cleanup();
    DestroyWindow(window::hwnd);
    UnregisterClass(window::wc.lpszClassName, hInst);

    return 0;
}
```

---

## 📁 Directory Structure

```
Moonlight/
│
├─ moonlib.hpp        # main header (window + dx11 helpers)
├─ imgui/
│   └─ imgui headers
└─ main.cpp
```

---

## 🛠 Planned Features

* Custom rendering backend (no ImGui required)
* Internal widget system
* Style/theme support
* DPI scaling

---

## 🤝 Contributing

If you have ideas, improvements, or rewrites, feel free to open an issue or PR.

---

## 📜 License
Moonlight is licensed under the **Moonlight Attribution License**.  
See the full text here: [license.txt](license.txt)
