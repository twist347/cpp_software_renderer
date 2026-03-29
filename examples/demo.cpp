#include "sr/core/color.h"
#include "sr/gfx/framebuffer.h"
#include "sr/gfx/rasterizer.h"
#include "sr/platform/window.h"

constexpr sr::i32 WIDTH = 800;
constexpr sr::i32 HEIGHT = 600;
constexpr sr::u32 TARGET_FPS = 60;

int main() {
    auto win = sr::unwrap(sr::Window::create("SR Demo", WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));

    sr::Window::set_target_fps(TARGET_FPS);

    while (win.is_open()) {
        fb.clear(sr::colors::black);

        // Row 1: rect, circle, ellipse
        sr::raster::fill_rect(fb, {50, 50}, {200, 170}, sr::colors::red);
        sr::raster::draw_rect(fb, {230, 50}, {380, 170}, sr::colors::green);
        sr::raster::fill_circle(fb, {480, 110}, 60, sr::colors::blue);
        sr::raster::draw_circle(fb, {620, 110}, 60, sr::colors::white);
        sr::raster::fill_ellipse(fb, {480, 110}, 60, 40, sr::Color{0, 180, 180});
        sr::raster::draw_ellipse(fb, {720, 110}, 50, 80, sr::Color{255, 200, 0});

        // Row 2: triangle, line
        sr::raster::fill_triangle(fb, {100, 230}, {50, 370}, {200, 370},
                                  sr::Color{255, 0, 255});
        sr::raster::draw_line(fb, {250, 230}, {500, 370}, sr::Color{255, 255, 0});

        // Row 3: polygon
        constexpr sr::Vec2i star[] = {
            {600, 250}, {620, 320}, {690, 320}, {635, 360},
            {655, 430}, {600, 385}, {545, 430}, {565, 360},
            {510, 320}, {580, 320},
        };
        sr::raster::fill_polygon(fb, star, sr::Color{255, 180, 0});
        sr::raster::draw_polygon(fb, star, sr::colors::white);

        constexpr sr::Vec2i hexagon[] = {
            {175, 430}, {245, 460}, {245, 520},
            {175, 550}, {105, 520}, {105, 460},
        };
        sr::raster::fill_polygon(fb, hexagon, sr::Color{100, 50, 200});
        sr::raster::draw_polygon(fb, hexagon, sr::colors::white);

        win.present(fb);
    }

    return 0;
}
