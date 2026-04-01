#include "sr/gfx/texture.h"

#include "stb_image.h"

namespace sr {
    static auto rgba_to_argb(const u8 *data, usize pixel_count) noexcept -> std::vector<u32>;

    Texture::Texture(i32 w, i32 h, std::vector<u32> buf) noexcept
        : m_width{w}, m_height{h}, m_buf{std::move(buf)} {
    }

    auto Texture::load(const char *path) noexcept -> Result<Texture> {
        i32 w{};
        i32 h{};
        i32 channels{};
        auto *data = stbi_load(path, &w, &h, &channels, 4);
        if (!data) {
            const auto *reason = stbi_failure_reason();
            if (reason && std::string_view{reason}.find("not find") != std::string_view::npos) {
                return std::unexpected{Error::FileNotFound};
            }
            return std::unexpected{Error::InvalidFormat};
        }

        auto buf = rgba_to_argb(data, static_cast<usize>(w) * h);
        stbi_image_free(data);
        return Texture{w, h, std::move(buf)};
    }

    auto Texture::get_pixel(i32 x, i32 y) const noexcept -> Color {
        if (in_bounds(x, y)) {
            return Color::from_argb(m_buf[static_cast<usize>(y) * m_width + x]);
        }
        return {};
    }

    auto Texture::get_pixel_argb(i32 x, i32 y) const noexcept -> u32 {
        if (in_bounds(x, y)) {
            return m_buf[static_cast<usize>(y) * m_width + x];
        }
        return 0;
    }

    static auto rgba_to_argb(const u8 *data, usize pixel_count) noexcept -> std::vector<u32> {
        std::vector<u32> buf(pixel_count);
        for (usize i = 0; i < pixel_count; ++i) {
            const auto *px = data + i * 4;
            buf[i] = static_cast<u32>(px[3]) << 24
                     | static_cast<u32>(px[0]) << 16
                     | static_cast<u32>(px[1]) << 8
                     | static_cast<u32>(px[2]);
        }
        return buf;
    }
}
