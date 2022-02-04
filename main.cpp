#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>

constexpr uint32_t GREEN = 0xFF00FF00;

struct Window
{
    uint32_t w;
    uint32_t h;
    SDL_Window *win;
};

struct Coord
{
    uint32_t x;
    uint32_t y;
};

const  static
void error_message_memory(const char* msg)
{
    std::cerr << "[Error] Memory: Unable to allocate memory for "
              << msg << " exiting..." << '\n';
}

constexpr size_t coord_to_index(Window &win, uint32_t x, uint32_t y)
{
    return (win.w * y + x);
}

constexpr Coord index_to_coord(Window &win, size_t index)
{
    return (Coord { (uint32_t)index % win.w, (uint32_t)index / win.w });
}

constexpr float distance(Coord point, Coord pixel)
{
    return sqrt(
           ((float)point.x - (float)pixel.x) * ((float)point.x - (float)pixel.x) +
           ((float)point.y - (float)pixel.y) * ((float)point.y - (float)pixel.y));
}

constexpr float max_distance(Window *win, int mouse_x, int mouse_y)
{
    if ((uint32_t)mouse_x <= (win->w / 2) && (uint32_t)mouse_y <= (win->h / 2)) { // Top left corner
        return distance(Coord{(uint32_t)mouse_x, (uint32_t)mouse_y}, Coord{win->w, win->h});
    } 
    else if ((uint32_t)mouse_x > (win->w / 2) && (uint32_t)mouse_y < (win->h / 2)) { // Top right corner
        return distance(Coord{(uint32_t)mouse_x, (uint32_t)mouse_y}, Coord{0, win->h});
    } 
    else if ((uint32_t)mouse_x > win->w / 2 && (uint32_t)mouse_y >= win->h / 2) { // Bottom right corner
        return distance(Coord{(uint32_t)mouse_x, (uint32_t)mouse_y}, Coord{0, 0});
    } 
    else {                                                  // Bottom left corner
        return distance(Coord{(uint32_t)mouse_x, (uint32_t)mouse_y}, Coord{win->w, 0});
    }
    return 0;
}

const uint32_t val_to_grayscale(uint32_t x)
{
    return (0xFF000000 +
            (x << 16)  +
            (x << 8)   +
            (x));
}

const void draw_rectangle(std::vector<uint32_t> &pix,
                          Window &win,
                          uint32_t x,
                          uint32_t y,
                          uint32_t rect_width,
                          uint32_t rect_height,
                          uint32_t col)
{
    uint32_t line_start = win.w * y + x;

    for (size_t i = 0; i < rect_height; i++)
    {
        for (size_t j = 0; j < rect_width; j++)
        {
            pix[line_start + j] = col;
        }
        line_start = win.w * (y + i) + x;
    }
}

uint32_t parse_string(std::string str)
{
    uint32_t acc = 0;
    uint32_t mult = 0;
    for (int i = str.size()-1; i >= 0; i--)
    {
        mult = str[i] - 48;
        if (mult < 0 || mult > 9)
        {
            std::cerr << "[Error] Arguments: Dimensions given contain non-numerical symbol." << '\n';
            exit(-1);
        }
        if (mult > 0)
        {
            for (int j = str.size() - (i+1); j > 0; j--)
            {
                mult *= 10;
            }
            acc += mult;
        }
    }
    return acc;
}

void clear_screen(std::vector<uint32_t> &screen_pixels, uint32_t color)
{
    for (auto &p: screen_pixels) {
        p = color;
    }
}

int main(const int argc, const char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "[Error] Usage: Missing arguments for window_global dimensions." << '\n';
        return -1;
    }
    SDL_Event event;
    Window window_global;
    window_global.w = parse_string(argv[1]);
    window_global.h = parse_string(argv[2]);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "[Error] SDL2: trouble initializing the SDL2 library." << '\n';
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
    int x, y = 0; // Mouse position
   
    bool running = true;
    while (running)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        SDL_PumpEvents();
        SDL_GetMouseState(&x, &y);

        float max = max_distance(&window_global, x, y);
        for (size_t i = 0; i < pixels.size(); i++) {
            Coord pixel_coord = index_to_coord(window_global, i);
            float calculated_distance = distance(Coord{(uint32_t)x, (uint32_t)y}, pixel_coord);

            pixels[i] = val_to_grayscale(floor(calculated_distance / max * 255));
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
