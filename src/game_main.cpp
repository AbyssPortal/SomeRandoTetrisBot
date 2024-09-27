#include <SDL2/SDL.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "Stacker.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int SQUARE_SIZE = 20;

SDL_Color get_piece_color(Stacker::Piece_Type type) {
    static const std::unordered_map<Stacker::Piece_Type, SDL_Color> piece_colors = {
        {Stacker::Piece_Type::I, {0, 255, 255, 255}},  // teal
        {Stacker::Piece_Type::T, {255, 0, 255, 255}},  // fuchsia
        {Stacker::Piece_Type::J, {0, 0, 255, 255}},    // blue
        {Stacker::Piece_Type::L, {255, 165, 0, 255}},  // orange
        {Stacker::Piece_Type::Z, {255, 0, 0, 255}},    // red
        {Stacker::Piece_Type::S, {0, 255, 0, 255}},    // green
        {Stacker::Piece_Type::O, {255, 255, 0, 255}}   // yellow
    };

    auto it = piece_colors.find(type);
    if (it != piece_colors.end()) {
        return it->second;
    } else {
        return {255, 255, 255, 255};  // default to white if not found
    }
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Basic SDL Program",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    Stacker::StackerGame game;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_DOWN:
                        game.send_event(Stacker::Event::tap_down);
                        break;
                    case SDLK_LEFT:
                        game.send_event(Stacker::Event::tap_left);
                        break;
                    case SDLK_RIGHT:
                        game.send_event(Stacker::Event::tap_right);
                        break;
                    case SDLK_UP:
                        game.send_event(Stacker::Event::tap_cw);
                        break;
                    case SDLK_c:
                        game.send_event(Stacker::Event::hold);
                        break;
                    case SDLK_SPACE:
                        game.send_event(Stacker::Event::hard_drop);
                        break;
                    case SDLK_z:
                        game.send_event(Stacker::Event::tap_ccw);
                }
            }
        }
        game.tick();
        // std::cout << std::endl << std::endl << std::endl << std::endl << "current boardstate:" << std::endl;
        // game.debug_print();

        // extract info
        Stacker::Matrix piece = game.get_active().location();
        SDL_Color col = get_piece_color(game.get_active().get_type());
        const Stacker::Matrix& board = game.get_board();

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Render the grid
        for (int i = 0; i < BOARD_HEIGHT; ++i) {
            for (int j = 0; j < BOARD_WIDTH; ++j) {
                int middle_offset = SCREEN_WIDTH/2 - (SQUARE_SIZE * BOARD_WIDTH)/2;
                SDL_Rect rect = {j * SQUARE_SIZE + middle_offset, i * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                if (piece.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);  // piece's color
                } else if (board.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);  // White
                } else {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);  // Black
                }
                SDL_RenderFillRect(renderer, &rect);

                // Draw the outline
                SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);  // Gray
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}