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

const inline static
void error_message_memory(const char* msg)
{
    std::cerr << "[Error] Memory: Unable to allocate memory for "
              << msg << " exiting..." << '\n';
}

const inline
size_t coord_to_index(Window &win, uint32_t x, uint32_t y)
{
    return (win.w * y + x);
}

const inline
Coord index_to_coord(Window &win, size_t index)
{
    return (Coord { (uint32_t)index % win.w, (uint32_t)index / win.w });
}

const inline
uint32_t distance(Coord &point, Coord &pixel)
{
    return floor(sqrt(
            pow(abs((float)point.x - pixel.x), 2) +
            pow(abs((float)point.y - pixel.y), 2)));
}

std::vector<uint32_t>
calculate_distances(Window &win, Coord point, std::vector<uint32_t> &pixels)
{
    std::vector<uint32_t> distances(win.w * win.h);
    Coord pixel;
    for (size_t i = 0; i < pixels.size(); i++)
    {
        pixel = index_to_coord(win, i);
        distances[i] = distance(point, pixel);
    }
    return distances;
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

std::vector<uint32_t>
normalize_values(std::vector<uint32_t> distances)
{
    std::vector<uint32_t> values(distances.size());
    uint32_t min_value = *std::min_element(distances.begin(), distances.end());
    uint32_t max_value = *std::max_element(distances.begin(), distances.end());
    for (size_t i = 0; i < distances.size(); i++)
    {
        uint32_t val = floor(255 * ((float)distances[i] - (float)min_value) / (float)max_value);
        values[i] = 0xFF000000; // Alpha
        values[i] += val << 16; // Red
        values[i] += val << 8; // Green
        values[i] += val << 0; // Blue
       
    }
    return values;
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
    std::vector<uint32_t> distances(pixels.size());
    std::vector<uint32_t> colors(pixels.size());
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
        distances = calculate_distances(window_global,
                                        Coord {
                                            (uint32_t)(x),
                                            (uint32_t)(y)
                                            },
                                        pixels);

        colors = normalize_values(distances);
        pixels = colors;

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
