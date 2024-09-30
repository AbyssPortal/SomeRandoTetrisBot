#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Stacker.h"
#include "StackerBot.h"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int SQUARE_SIZE = SCREEN_WIDTH / 50;

const static int middle_offset_horizontal = SCREEN_WIDTH / 2 - (SQUARE_SIZE * BOARD_WIDTH) / 2;

Stacker::BotParameters load_params_from_file(const std::string& filename) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        return Stacker::BotParameters::deserialize(buffer.str());
    } else {
        std::cerr << "Unable to open file for reading: " << filename << std::endl;
        return Stacker::BotParameters();
    }
}

bool isAppFocused(SDL_Window* window) {
    Uint32 flags = SDL_GetWindowFlags(window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void draw_piece(SDL_Renderer* renderer, Stacker::Piece_Type type, int offsetx, int offsety);

SDL_Texture* load_texture(SDL_Renderer* renderer, const char* path) {
    // Load BMP image
    SDL_Surface* image = SDL_LoadBMP(path);
    if (image == nullptr) {
        std::cerr << path << " ";
        std::cerr << "Unable to load image! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    // t spin texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
    if (texture == nullptr) {
        std::cerr << "Unable to create texture! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    return texture;
}

SDL_Color get_piece_color(Stacker::Piece_Type type) {
    static const std::unordered_map<Stacker::Piece_Type, SDL_Color> piece_colors = {
        {Stacker::Piece_Type::I, {0, 255, 255, 255}},  // teal
        {Stacker::Piece_Type::T, {255, 0, 255, 255}},  // fuchsia`
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

void draw_next_queue(SDL_Renderer* renderer, const Stacker::StackerGame& game) {
    int counter = 0;
    for (Stacker::Piece_Type piece : game.get_next_queue()) {
        if (counter >= Stacker::StackerGame::NEXT_QUEUE_MIN_SIZE) {
            break;
        }
        draw_piece(renderer, piece, middle_offset_horizontal + 11 * SQUARE_SIZE, 4 * counter * SQUARE_SIZE);
        counter++;
    }
}

std::map<std::string, Mix_Chunk*> load_sounds() {
    std::map<std::string, Mix_Chunk*> sounds;
    std::vector<std::string> sound_files = {
        "Assets/double.ogg",
        "Assets/hard_drop.ogg",
        "Assets/quad.ogg",
        "Assets/single.ogg",
        "Assets/t_spin_double.ogg",
        "Assets/t_spin_single.ogg",
        "Assets/t_spin_triple.ogg",
        "Assets/test.ogg",
        "Assets/triple.ogg"};

    for (const auto& file : sound_files) {
        Mix_Chunk* sound = Mix_LoadWAV(file.c_str());
        if (sound == nullptr) {
            std::cerr << "Failed to load sound " << file << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
            continue;
        }
        sounds[file] = sound;
    }

    return sounds;
}

int main(int argc, char* args[]) {
    int seed = (argc > 1) ? std::atoi(args[1]) : static_cast<int>(std::time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    auto sounds = load_sounds();

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

    auto* t_spin_texture = load_texture(renderer, "Assets/t-spin.bmp");
    auto* single_texture = load_texture(renderer, "Assets/single.bmp");
    auto* double_texture = load_texture(renderer, "Assets/double.bmp");
    auto* triple_texture = load_texture(renderer, "Assets/triple.bmp");
    auto* quadruple_texture = load_texture(renderer, "Assets/quadruple.bmp");

    bool quit = false;
    SDL_Event e;
    auto god = load_params_from_file("god.txt");
    Stacker::StackerBot bot = Stacker::StackerBot(god, seed);

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        bool do_bot_move = false;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1) {
                do_bot_move = true;
            }
            if (e.type == SDL_KEYDOWN && e.key.repeat == false) {
                switch (e.key.keysym.sym) {
                    case SDLK_DOWN:
                        bot.get_game().send_event(Stacker::Event::press_down);
                        break;
                    case SDLK_LEFT:
                        bot.get_game().send_event(Stacker::Event::press_left);
                        break;
                    case SDLK_RIGHT:
                        bot.get_game().send_event(Stacker::Event::press_right);
                        break;
                    case SDLK_UP:
                        bot.get_game().send_event(Stacker::Event::tap_cw);
                        break;
                    case SDLK_c:
                        bot.get_game().send_event(Stacker::Event::hold);
                        break;
                    case SDLK_SPACE:
                        bot.get_game().send_event(Stacker::Event::hard_drop);
                        Mix_PlayChannel(-1, sounds["Assets/hard_drop.ogg"], 0);
                        break;
                    case SDLK_z:
                        bot.get_game().send_event(Stacker::Event::tap_ccw);
                        break;
                    case SDLK_a:
                        bot.get_game().send_event(Stacker::Event::tap_180);
                        break;
                    case SDLK_r:
                        bot.get_game().reset();
                        break;
                }
            }
            if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_DOWN:
                        bot.get_game().send_event(Stacker::Event::release_down);
                        break;
                    case SDLK_LEFT:
                        bot.get_game().send_event(Stacker::Event::release_left);
                        break;
                    case SDLK_RIGHT:
                        bot.get_game().send_event(Stacker::Event::release_right);
                        break;
                }
            }
        }
        if (isAppFocused(window)) {
            bot.get_game().tick();
            if (bot.get_game().is_dead()) {
                bot.get_game().reset();
            }
        }
        // std::cout << std::endl << std::endl << std::endl << std::endl << "current boardstate:" << std::endl;
        // game.debug_print();

        // extract info
        Stacker::Matrix piece = bot.get_game().get_active().location();
        Stacker::Matrix ghost = bot.get_game().get_ghost().location();
        Stacker::BlockPiece bot_suggestion = bot.get_game().get_active();
        {
            Uint32 startTicks = SDL_GetTicks();
            Stacker::MoveInfo best_move = bot.suggest_move();
            Uint32 endTicks = SDL_GetTicks();
            Uint32 elapsedTicks = endTicks - startTicks;
            std::cout << "Ponderation time elapsed: " << elapsedTicks << " ms" << std::endl;

            Stacker::print_score(bot.get_game().get_board(), bot.get_params());

            if (best_move.hold) {
                bot_suggestion = bot.get_game().is_hold_empty() ? Stacker::BlockPiece(bot.get_game().get_next_queue().front()) : Stacker::BlockPiece(bot.get_game().get_hold());
            }

            if (do_bot_move) {
                Stacker::do_move(best_move, bot.get_game());
                Mix_PlayChannel(-1, sounds["Assets/hard_drop.ogg"], 0);
            }

            Stacker::move_block_piece(best_move, bot.get_game().get_board(), bot_suggestion);
        }

        Stacker::Matrix bot_ghost = bot_suggestion.location();

        SDL_Color col = get_piece_color(bot.get_game().get_active().get_type());
        const Stacker::Matrix& board = bot.get_game().get_board();

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Render the grid
        for (int i = 0; i < BOARD_HEIGHT; ++i) {
            for (int j = 0; j < BOARD_WIDTH; ++j) {
                SDL_Rect rect = {j * SQUARE_SIZE + middle_offset_horizontal, i * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                if (piece.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);  // piece's color
                } else if (board.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);  // White
                } else if (bot_ghost.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, 0x87, 0xB5, 0x38, col.a);  // olive oil
                    if (ghost.at(j, BOARD_HEIGHT - i - 1)) {
                        SDL_SetRenderDrawColor(renderer, col.r / 1.2, col.g / 1.2, col.b / 1.2, col.a);  // piece's color
                    }
                } else if (ghost.at(j, BOARD_HEIGHT - i - 1)) {
                    SDL_SetRenderDrawColor(renderer, col.r / 2, col.g / 2, col.b / 2, col.a);  // piece's color
                } else {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);  // Black
                }
                SDL_RenderFillRect(renderer, &rect);

                // Draw the outline
                SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);  // Gray
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        if (!bot.get_game().is_hold_empty()) {
            draw_piece(renderer, bot.get_game().get_hold(), middle_offset_horizontal - SQUARE_SIZE * 5, 0);
        }

        {
            auto clear = bot.get_game().get_last_clear();
            if (clear.is_spin == true) {
                SDL_Rect t_spin_rect;
                t_spin_rect.x = middle_offset_horizontal - 350;  // x position
                t_spin_rect.y = 200;                             // y position
                t_spin_rect.w = 350;                             // width
                t_spin_rect.h = 200;                             // height

                SDL_RenderCopy(renderer, t_spin_texture, nullptr, &t_spin_rect);
            }
            if (clear.clear_count >= 0) {
                SDL_Rect count_rect;
                count_rect.x = middle_offset_horizontal - 350;  // x position
                count_rect.y = 400;                             // y position
                count_rect.w = 350;                             // width
                count_rect.h = 200;                             // height

                SDL_Texture* count_texture;

                switch (clear.clear_count) {
                    case 1:
                        count_texture = single_texture;
                        break;
                    case 2:
                        count_texture = double_texture;
                        break;
                    case 3:
                        count_texture = triple_texture;
                        break;
                    case 4:
                        count_texture = quadruple_texture;
                        break;
                    default:
                        count_texture = nullptr;
                        break;
                }
                if (count_texture != nullptr) {
                    SDL_RenderCopy(renderer, count_texture, nullptr, &count_rect);
                }
            }
            if (bot.get_game().get_lock_last_frame()) {
                if (!clear.is_spin) {
                    switch (clear.clear_count) {
                        case 0:
                            break;
                        case 1:
                            Mix_PlayChannel(-1, sounds["Assets/single.ogg"], 0);
                            break;
                        case 2:
                            Mix_PlayChannel(-1, sounds["Assets/double.ogg"], 0);

                            break;
                        case 3:
                            Mix_PlayChannel(-1, sounds["Assets/triple.ogg"], 0);

                            break;
                        case 4:
                            Mix_PlayChannel(-1, sounds["Assets/quad.ogg"], 0);
                            break;
                        default:
                            break;
                    }
                } else {
                    switch (clear.clear_count) {
                        case 0:
                            break;
                        case 1:
                            Mix_PlayChannel(-1, sounds["Assets/t_spin_single.ogg"], 0);
                            break;
                        case 2:
                            Mix_PlayChannel(-1, sounds["Assets/t_spin_double.ogg"], 0);

                            break;
                        case 3:
                            Mix_PlayChannel(-1, sounds["Assets/t_spin_triple.ogg"], 0);

                            break;
                        default:
                            break;
                    }
                }
            }
        }

        draw_next_queue(renderer, bot.get_game());

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

void draw_piece(SDL_Renderer* renderer, Stacker::Piece_Type type, int offsetx, int offsety) {  // slow implementation - improve later if neededd
    for (int j = 2; j < 7; ++j) {
        SDL_Rect rect = {(j - 2) * SQUARE_SIZE + offsetx, offsety, SQUARE_SIZE, SQUARE_SIZE};

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);  // Black

        SDL_RenderFillRect(renderer, &rect);

        // Draw the outline
        SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);  // Gray
        SDL_RenderDrawRect(renderer, &rect);
    }
    Stacker::Matrix location = Stacker::BlockPiece(type).location();

    for (int i = 0; i < 3; ++i) {
        for (int j = 2; j < 7; ++j) {
            SDL_Rect rect = {(j - 2) * SQUARE_SIZE + offsetx, (i + 1) * SQUARE_SIZE + offsety, SQUARE_SIZE, SQUARE_SIZE};

            SDL_Color col = get_piece_color(type);

            if (location.at(j, BOARD_HEIGHT - i - 1)) {
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);  // piece's color
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);  // Black
            }
            SDL_RenderFillRect(renderer, &rect);

            // Draw the outline
            SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);  // Gray
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}