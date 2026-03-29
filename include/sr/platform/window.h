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
        static Result<Window> create(const char *title, i32 width, i32 height) noexcept;

        ~Window() noexcept;

        Window(Window &&other) noexcept;
        Window &operator=(Window &&other) noexcept;

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] bool is_open() const noexcept { return m_window != nullptr; }

        void present(const FrameBuffer &fb) noexcept;

        static void set_target_fps(u32 fps) noexcept;

        [[nodiscard]] bool key_down(Key key) const noexcept;
        [[nodiscard]] bool key_pressed(Key key) const noexcept;
        [[nodiscard]] bool key_released(Key key) const noexcept;

        [[nodiscard]] bool mouse_down(MouseButton btn) const noexcept;
        [[nodiscard]] bool mouse_pressed(MouseButton btn) const noexcept;
        [[nodiscard]] bool mouse_released(MouseButton btn) const noexcept;

        [[nodiscard]] Vec2f mouse_pos() const noexcept;
        [[nodiscard]] f32 mouse_scroll_y() const noexcept;

        [[nodiscard]] f32 dt() const noexcept { return m_dt; }

        [[nodiscard]] i32 width() const noexcept { return m_width; }
        [[nodiscard]] i32 height() const noexcept { return m_height; }

    private:
        Window(mfb_window *win, i32 w, i32 h) noexcept;

        void update_input() noexcept;

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
