#pragma once

#include <array>

#include "sr/core/types.h"
#include "sr/core/vec.h"
#include "sr/gfx/framebuffer.h"
#include "sr/platform/input.h"

struct mfb_window;
struct mfb_timer;

namespace sr {
    class Window {
    public:
        static auto create(const char *title, i32 width, i32 height) noexcept -> Result<Window>;

        ~Window() noexcept;

        Window(Window &&other) noexcept;
        auto operator=(Window &&other) noexcept -> Window &;

        Window(const Window &) = delete;
        auto operator=(const Window &) -> Window & = delete;

        [[nodiscard]] auto is_open() const noexcept -> bool { return m_window != nullptr; }

        // non-const: minifb expects void*, not const void*
        auto present(FrameBuffer &fb) noexcept -> void;

        static auto set_target_fps(u32 fps) noexcept -> void;

        [[nodiscard]] auto key_down(Key key) const noexcept -> bool;
        [[nodiscard]] auto key_pressed(Key key) const noexcept -> bool;
        [[nodiscard]] auto key_released(Key key) const noexcept -> bool;

        [[nodiscard]] auto mouse_down(MouseButton btn) const noexcept -> bool;
        [[nodiscard]] auto mouse_pressed(MouseButton btn) const noexcept -> bool;
        [[nodiscard]] auto mouse_released(MouseButton btn) const noexcept -> bool;

        [[nodiscard]] auto mouse_pos() const noexcept -> Vec2f;
        [[nodiscard]] auto mouse_scroll_y() const noexcept -> f32;

        [[nodiscard]] auto dt() const noexcept -> f32 { return m_dt; }

        [[nodiscard]] auto width() const noexcept -> i32 { return m_width; }
        [[nodiscard]] auto height() const noexcept -> i32 { return m_height; }

    private:
        Window(mfb_window *win, i32 w, i32 h) noexcept;

        auto update_input() noexcept -> void;

        static constexpr i32 MAX_KEYS = 349;
        static constexpr i32 MAX_MOUSE_BUTTONS = 8;

        mfb_window *m_window{nullptr};
        mfb_timer *m_timer{nullptr};
        i32 m_width{};
        i32 m_height{};
        f32 m_dt{};
        std::array<u8, MAX_KEYS> m_prev_keys{};
        std::array<u8, MAX_MOUSE_BUTTONS> m_prev_mouse{};
    };
}
