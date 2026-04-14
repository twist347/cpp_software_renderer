#include "sr/gfx/texture.h"

#include <filesystem>

#include "stb_image.h"

namespace sr {
    static auto rgba_to_argb(const u8 *data, usize pixel_count) noexcept -> std::vector<Pixel>;

    Texture::Texture(i32 w, i32 h, std::vector<Pixel> buf) noexcept
        : m_width{w}, m_height{h}, m_buf{std::move(buf)} {
    }

    auto Texture::from_pixels(i32 w, i32 h, std::span<const Pixel> pixels) noexcept -> Result<Texture> {
        if (w <= 0 || h <= 0) {
            return std::unexpected{Error::InvalidDimensions};
        }
        if (pixels.size() != static_cast<usize>(w) * h) {
            return std::unexpected{Error::InvalidDimensions};
        }
        return Texture{w, h, std::vector<Pixel>(pixels.cbegin(), pixels.cend())};
    }

    auto Texture::load(const char *path) noexcept -> Result<Texture> {
        if (std::error_code ec; !std::filesystem::exists(path, ec) || ec) {
            return std::unexpected{Error::FileNotFound};
        }

        i32 w{}, h{};
        i32 channels{};
        auto *data = stbi_load(path, &w, &h, &channels, 4);
        if (!data) {
            return std::unexpected{Error::InvalidFormat};
        }
        if (w <= 0 || h <= 0) {
            stbi_image_free(data);
            return std::unexpected{Error::InvalidDimensions};
        }

        auto buf = rgba_to_argb(data, static_cast<usize>(w) * h);
        stbi_image_free(data);
        return Texture{w, h, std::move(buf)};
    }

    static auto rgba_to_argb(const u8 *data, usize pixel_count) noexcept -> std::vector<Pixel> {
        std::vector<Pixel> buf(pixel_count);
        for (usize i = 0; i < pixel_count; ++i) {
            const auto *px = data + i * 4;
            buf[i] = static_cast<u32>(px[3]) << 24 |
                     static_cast<u32>(px[0]) << 16 |
                     static_cast<u32>(px[1]) << 8 |
                     static_cast<u32>(px[2]);
        }
        return buf;
    }
}
