#include "sr/sr.h"

constexpr sr::i32 WIDTH = 1920;
constexpr sr::i32 HEIGHT = 1080;
constexpr auto WIN_TITLE = "SR Primitives";
constexpr sr::u32 TARGET_FPS = 60;

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    sr::Window::set_target_fps(TARGET_FPS);

    while (win.is_open()) {
        ren.clear();

        // Row 1: rects, circles, ellipses
        ren.fill_rect({80, 80}, {350, 280}, sr::colors::red);
        ren.draw_rect({480, 80}, {750, 280}, sr::colors::green);
        ren.fill_circle({1000, 180}, 120, sr::colors::blue);
        ren.draw_circle({1300, 180}, 120, sr::colors::white);
        ren.fill_ellipse({1600, 180}, 120, 80, sr::Color{0, 180, 180});
        ren.draw_ellipse({1800, 180}, 80, 140, sr::Color{255, 200, 0});

        // Row 2: triangle, line
        ren.fill_triangle({200, 420}, {80, 680}, {400, 680}, sr::Color{255, 0, 255});
        ren.draw_line({550, 420}, {950, 680}, sr::Color{255, 255, 0});

        // Row 3: polygons
        constexpr sr::Vec2i star[] = {
            {1200, 420}, {1240, 580}, {1400, 580}, {1275, 680},
            {1320, 840}, {1200, 750}, {1080, 840}, {1125, 680},
            {1000, 580}, {1160, 580},
        };
        ren.fill_polygon(star, sr::Color{255, 180, 0});
        ren.draw_polygon(star, sr::colors::white);

        constexpr sr::Vec2i hexagon[] = {
            {350, 780}, {490, 840}, {490, 960},
            {350, 1020}, {210, 960}, {210, 840},
        };
        ren.fill_polygon(hexagon, sr::Color{100, 50, 200});
        ren.draw_polygon(hexagon, sr::colors::white);

        // Rotated rect (origin at center → pos = visual center)
        ren.fill_rect_ex({1700, 740}, {350, 180}, {175, 90}, 0.5f, sr::Color{0, 200, 100});
        ren.draw_rect_ex({1700, 740}, {350, 180}, {175, 90}, 0.5f, sr::colors::white);

        win.present(fb);
    }
}
