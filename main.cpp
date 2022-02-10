#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

constexpr uint32_t GREEN = 0xFF00FF00;
constexpr uint32_t BLACK = 0x00000000;

struct Window
{
    uint32_t w;
    uint32_t h;
    SDL_Window *win;
};

struct Coord
{
    int32_t x;
    int32_t y;
};

struct Rect
{
    Coord pos;
    uint32_t width;
    uint32_t height;
};

static inline void error_message_memory(const char* msg)
{
    std::cerr << "[Error] Memory: Unable to allocate memory for "
              << msg << "." << '\n';
}

inline constexpr size_t coord_to_index(Window *win, uint32_t x, uint32_t y)
{
    return (win->w * y + x);
}

inline constexpr Coord index_to_coord(Window *win, size_t index)
{
    return (Coord { (int32_t)index % (int32_t)win->w, (int32_t)index / (int32_t)win->w });
}

constexpr float distance(Coord a, Coord b)
{
    return sqrt(
           ((float)a.x - (float)b.x) * ((float)a.x - (float)b.x) +
           ((float)a.y - (float)b.y) * ((float)a.y - (float)b.y));
}

inline constexpr uint32_t val_to_grayscale(uint8_t x)
{
    return (0xFF000000 | (x << (8 * 2)) | (x << (8 * 1)) | (x << (8 * 0)));
}

inline constexpr float lerp(int32_t a, int32_t b, float t) {
    return a + (b - a) * t;
}
  
void draw_line(std::vector<uint32_t> *pix,
               Window *win,
               Coord a,
               Coord b,
               uint32_t col)
{
    uint32_t x_point = 0;
    uint32_t y_point = 0;

    for (float i = 0.f; i <= 1.f; i += 1.f / distance(a, b)) {
        x_point = (uint32_t)lerp(a.x, b.x, i);
        y_point = (uint32_t)lerp(a.y, b.y, i);
        (*pix)[coord_to_index(win, x_point, y_point)] = col;
    }
}

void draw_rectangle(std::vector<uint32_t> *pix,
                          Window *win,
                          Rect *rect,
                          uint32_t col)
{
    if (rect->pos.x < 0) {
        rect->pos.x = 0;
        if (rect->pos.y < 0) 
            rect->pos.y = 0;
    } else if (rect->pos.y < 0) {
        rect->pos.y = 0;
        if (rect->pos.x < 0) 
            rect->pos.x = 0;
    }

    size_t line_start = win->w * rect->pos.y + rect->pos.x;

    for (size_t i = 0; i < rect->height; i++) {
        for (size_t j = 0; j < rect->width; j++) {
            (*pix)[line_start + j] = col;
        }
        line_start = win->w * (rect->pos.y + i) + rect->pos.x;
    }
}

constexpr bool mouse_in_rect(Rect *rect, int *mouse_x, int *mouse_y) {
    Coord up_left_bound{rect->pos.x,
                        rect->pos.y};
    Coord down_right_bound{rect->pos.x + (int32_t)rect->width,
                           rect->pos.y + (int32_t)rect->height};    

    return (*mouse_x >= up_left_bound.x && *mouse_x <= down_right_bound.x &&
            *mouse_y >= up_left_bound.y && *mouse_y <= down_right_bound.y);
}

const uint32_t parse_string(std::string str)
{
    uint32_t acc = 0;
    uint32_t mult = 0;
    for (int i = str.size()-1; i >= 0; i--) {
        mult = str[i] - 48;
        if (mult < 0 || mult > 9) {
            std::cerr << "[Error] Arguments: Dimensions given contain non-numerical symbol." << '\n';
            exit(-1);
        }
        if (mult > 0) {
            for (int j = str.size() - (i+1); j > 0; j--) {
                mult *= 10;
            }
            acc += mult;
        }
    }
    return acc;
}

inline void clear_screen(std::vector<uint32_t> *screen_pixels, uint32_t color)
{
    for (auto &p: *screen_pixels) {
        p = color;
    }
}

int main(const int argc, const char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: imageViewer [WINDOW_WIDTH] [WINDOW_HEIGHT]" << '\n';
        return 1;
    }

    SDL_Event event;
    Window window_global;
    window_global.w = parse_string(argv[1]);
    window_global.h = parse_string(argv[2]);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[Error] Trouble initializing the SDL2 library." << '\n';
        return -1;
    }

     window_global.win = SDL_CreateWindow("imageViewer",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          window_global.w,
                                          window_global.h,
                                          0);

    if (window_global.win == nullptr) error_message_memory("window");

    SDL_Renderer *renderer = SDL_CreateRenderer(window_global.win, -1, 0);
    if (renderer == nullptr) error_message_memory("renderer");

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STATIC,
                                             window_global.w,
                                             window_global.h);

    if (texture == nullptr) error_message_memory("texture");

    std::vector<uint32_t> pixels(window_global.w * window_global.h);
    int x = 0, y = 0; // Mouse positions
    Rect rect = { 20, 20, 50, 50 };
    bool running = true;
    bool mouse_hold = false;

    while (running)
    {
        while(SDL_PollEvent(&event))
        {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mouse_hold = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse_hold = false;
                    break;
                default:
                    break;
            }
        }

        SDL_PumpEvents();
        SDL_GetMouseState(&x, &y);

        clear_screen(&pixels, BLACK);
        draw_rectangle(&pixels, &window_global, &rect, val_to_grayscale(100));
        draw_line(&pixels, &window_global, rect.pos, Coord{x, y}, GREEN);

        if (mouse_hold && mouse_in_rect(&rect, &x, &y)) {
            rect.pos.x = x - rect.width / 2;
            rect.pos.y = y - rect.height / 2;
        }

        SDL_UpdateTexture(texture, nullptr, static_cast<const void*>(pixels.data()), window_global.w * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window_global.win);
    SDL_Quit();
}
