#include "sr/platform/window.h"

#include <algorithm>
#include <cassert>
#include <format>
#include <utility>

#include <MiniFB.h>

namespace sr {
    Window::Window(mfb_window *win, i32 w, i32 h, std::string title) noexcept
        : m_window{win}, m_timer{mfb_timer_create()}, m_width{w}, m_height{h}, m_title{std::move(title)} {
    }

    Window::~Window() noexcept {
        if (m_timer) {
            mfb_timer_destroy(m_timer);
        }
        if (m_window) {
            mfb_close(m_window);
        }
    }

    Window::Window(Window &&other) noexcept
        : m_window{std::exchange(other.m_window, nullptr)},
          m_timer{std::exchange(other.m_timer, nullptr)},
          m_width{other.m_width},
          m_height{other.m_height},
          m_dt{other.m_dt},
          m_title{std::move(other.m_title)},
          m_prev_keys{other.m_prev_keys},
          m_prev_mouse{other.m_prev_mouse} {
    }

    auto Window::operator=(Window &&other) noexcept -> Window & {
        if (this != &other) {
            if (m_window) {
                mfb_close(m_window);
            }
            if (m_timer) {
                mfb_timer_destroy(m_timer);
            }
            m_window = std::exchange(other.m_window, nullptr);
            m_timer = std::exchange(other.m_timer, nullptr);
            m_width = other.m_width;
            m_height = other.m_height;
            m_dt = other.m_dt;
            m_title = std::move(other.m_title);
            m_prev_keys = other.m_prev_keys;
            m_prev_mouse = other.m_prev_mouse;
        }
        return *this;
    }

    auto Window::create(const char *title, i32 width, i32 height) noexcept -> Result<Window> {
        auto *win = mfb_open(title, static_cast<unsigned>(width), static_cast<unsigned>(height));
        if (!win) {
            return std::unexpected{Error::WindowCreationFailed};
        }
        return Window{win, width, height, std::string{title}};
    }

    auto Window::set_target_fps(u32 fps) noexcept -> void {
        mfb_set_target_fps(fps);
    }

    auto Window::set_title(const char *title) noexcept -> void {
        assert(m_window);
        m_title = title;
        mfb_set_title(m_window, title);
    }

    auto Window::key_down(Key key) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_key_buffer(m_window);
        assert(buf);
        return buf[static_cast<i32>(key)];
    }

    auto Window::key_pressed(Key key) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_key_buffer(m_window);
        assert(buf);
        const auto k = static_cast<i32>(key);
        return buf[k] && !m_prev_keys[k];
    }

    auto Window::key_released(Key key) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_key_buffer(m_window);
        assert(buf);
        const auto k = static_cast<i32>(key);
        return !buf[k] && m_prev_keys[k];
    }

    auto Window::mouse_down(MouseButton btn) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_mouse_button_buffer(m_window);
        assert(buf);
        return buf[static_cast<u8>(btn)];
    }

    auto Window::mouse_pressed(MouseButton btn) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_mouse_button_buffer(m_window);
        assert(buf);
        const auto b = static_cast<u8>(btn);
        return buf[b] && !m_prev_mouse[b];
    }

    auto Window::mouse_released(MouseButton btn) const noexcept -> bool {
        assert(m_window);
        const auto *buf = mfb_get_mouse_button_buffer(m_window);
        assert(buf);
        const auto b = static_cast<u8>(btn);
        return !buf[b] && m_prev_mouse[b];
    }

    auto Window::mouse_pos() const noexcept -> Vec2f {
        assert(m_window);
        return {
            static_cast<f32>(mfb_get_mouse_x(m_window)),
            static_cast<f32>(mfb_get_mouse_y(m_window))
        };
    }

    auto Window::mouse_scroll_y() const noexcept -> f32 {
        assert(m_window);
        return mfb_get_mouse_scroll_y(m_window);
    }

    // No assert: called from present() before mfb_update_ex which may close the window
    auto Window::update_input() noexcept -> void {
        if (!m_window) {
            return;
        }
        if (const auto *keys = mfb_get_key_buffer(m_window)) {
            std::copy_n(keys, MAX_KEYS, m_prev_keys.begin());
        }
        if (const auto *mouse = mfb_get_mouse_button_buffer(m_window)) {
            std::copy_n(mouse, MAX_MOUSE_BUTTONS, m_prev_mouse.begin());
        }
    }

    auto Window::update_dt() noexcept -> void {
        m_dt = static_cast<f32>(mfb_timer_delta(m_timer));
    }

    auto Window::update_fps_title() noexcept -> void {
        m_fps_accum += m_dt;
        ++m_fps_frames;
        if (m_fps_accum >= 0.5f) {
            const auto fps = static_cast<i32>(static_cast<f64>(m_fps_frames) / static_cast<f64>(m_fps_accum));
            const auto title = std::format("{} [{} FPS]", m_title, fps);
            mfb_set_title(m_window, title.c_str());
            m_fps_accum = 0.0f;
            m_fps_frames = 0;
        }
    }

    // No assert: mfb_update_ex/mfb_wait_sync may destroy the window at runtime
    auto Window::present(FrameBuffer &fb) noexcept -> void {
        if (!m_window) {
            return;
        }
        update_dt();
        update_fps_title();
        update_input();
        const auto st = mfb_update_ex(
            m_window,
            fb.data(),
            static_cast<unsigned>(fb.width()),
            static_cast<unsigned>(fb.height())
        );
        if (st != MFB_STATE_OK) {
            m_window = nullptr;
            return;
        }
        if (!mfb_wait_sync(m_window)) {
            m_window = nullptr;
        }
    }
}
