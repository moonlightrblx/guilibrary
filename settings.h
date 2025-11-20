#pragma once
namespace settings
{
	namespace keys {
		inline constexpr int _menu = VK_INSERT;
		inline constexpr int _exit = VK_END;
	}
	namespace vars {
		inline bool option_1 = false;
		inline float slider_val = 50.0f;
	}

	// constants
	inline constexpr int WINDOW_W = 400;
	inline constexpr int WINDOW_H = 400;
	inline const char* PRODUCT_NAME = "moonlight.cc";

	// menu variables
	inline bool show_menu = true;
	inline int current_tab = 0;
}