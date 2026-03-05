#include "game_core.hpp"

#if __has_include(<SDL.h>)
#include <SDL.h>
#elif __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#error "SDL2 headers not found"
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

void setColor(SDL_Renderer* renderer, const Color& c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

void fillRect(SDL_Renderer* renderer, int x, int y, int w, int h, const Color& c) {
    setColor(renderer, c);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(renderer, &r);
}

void drawRect(SDL_Renderer* renderer, int x, int y, int w, int h, const Color& c) {
    setColor(renderer, c);
    SDL_Rect r{x, y, w, h};
    SDL_RenderDrawRect(renderer, &r);
}

void drawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, const Color& c) {
    setColor(renderer, c);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius, const Color& c) {
    setColor(renderer, c);
    for (int dy = -radius; dy <= radius; ++dy) {
        const int dx = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void fillTriangle(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int x3, int y3, const Color& c) {
    const int minX = std::min({x1, x2, x3});
    const int maxX = std::max({x1, x2, x3});
    const int minY = std::min({y1, y2, y3});
    const int maxY = std::max({y1, y2, y3});

    const auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
        return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
    };

    setColor(renderer, c);
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const int w0 = edge(x2, y2, x3, y3, x, y);
            const int w1 = edge(x3, y3, x1, y1, x, y);
            const int w2 = edge(x1, y1, x2, y2, x, y);
            const bool hasNeg = (w0 < 0) || (w1 < 0) || (w2 < 0);
            const bool hasPos = (w0 > 0) || (w1 > 0) || (w2 > 0);
            if (!(hasNeg && hasPos)) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

const std::array<uint8_t, 7>& glyphFor(char c) {
    static const std::array<uint8_t, 7> empty{{0, 0, 0, 0, 0, 0, 0}};
    static const std::array<uint8_t, 7> A{{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}};
    static const std::array<uint8_t, 7> B{{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}};
    static const std::array<uint8_t, 7> C{{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> D{{0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}};
    static const std::array<uint8_t, 7> E{{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}};
    static const std::array<uint8_t, 7> F{{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}};
    static const std::array<uint8_t, 7> G{{0x0E, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0F}};
    static const std::array<uint8_t, 7> H{{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}};
    static const std::array<uint8_t, 7> I{{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F}};
    static const std::array<uint8_t, 7> J{{0x1F, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C}};
    static const std::array<uint8_t, 7> K{{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}};
    static const std::array<uint8_t, 7> L{{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}};
    static const std::array<uint8_t, 7> M{{0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}};
    static const std::array<uint8_t, 7> N{{0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}};
    static const std::array<uint8_t, 7> O{{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> P{{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}};
    static const std::array<uint8_t, 7> Q{{0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}};
    static const std::array<uint8_t, 7> R{{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}};
    static const std::array<uint8_t, 7> S{{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}};
    static const std::array<uint8_t, 7> T{{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}};
    static const std::array<uint8_t, 7> U{{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> V{{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}};
    static const std::array<uint8_t, 7> W{{0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}};
    static const std::array<uint8_t, 7> X{{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}};
    static const std::array<uint8_t, 7> Y{{0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}};
    static const std::array<uint8_t, 7> Z{{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}};
    static const std::array<uint8_t, 7> D0{{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> D1{{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}};
    static const std::array<uint8_t, 7> D2{{0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}};
    static const std::array<uint8_t, 7> D3{{0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}};
    static const std::array<uint8_t, 7> D4{{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}};
    static const std::array<uint8_t, 7> D5{{0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}};
    static const std::array<uint8_t, 7> D6{{0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> D7{{0x1F, 0x01, 0x02, 0x04, 0x04, 0x04, 0x04}};
    static const std::array<uint8_t, 7> D8{{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}};
    static const std::array<uint8_t, 7> D9{{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E}};
    static const std::array<uint8_t, 7> COLON{{0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}};
    static const std::array<uint8_t, 7> DASH{{0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}};
    static const std::array<uint8_t, 7> SLASH{{0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}};
    static const std::array<uint8_t, 7> DOT{{0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}};
    static const std::array<uint8_t, 7> EXCL{{0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00}};
    static const std::array<uint8_t, 7> LP{{0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}};
    static const std::array<uint8_t, 7> RP{{0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}};

    switch (c) {
        case 'A': return A; case 'B': return B; case 'C': return C; case 'D': return D; case 'E': return E;
        case 'F': return F; case 'G': return G; case 'H': return H; case 'I': return I; case 'J': return J;
        case 'K': return K; case 'L': return L; case 'M': return M; case 'N': return N; case 'O': return O;
        case 'P': return P; case 'Q': return Q; case 'R': return R; case 'S': return S; case 'T': return T;
        case 'U': return U; case 'V': return V; case 'W': return W; case 'X': return X; case 'Y': return Y;
        case 'Z': return Z;
        case '0': return D0; case '1': return D1; case '2': return D2; case '3': return D3; case '4': return D4;
        case '5': return D5; case '6': return D6; case '7': return D7; case '8': return D8; case '9': return D9;
        case ':': return COLON; case '-': return DASH; case '/': return SLASH; case '.': return DOT;
        case '!': return EXCL; case '(': return LP; case ')': return RP;
        default: return empty;
    }
}

void drawText(SDL_Renderer* renderer, int x, int y, int scale, std::string text, const Color& c) {
    setColor(renderer, c);
    int cx = x;
    for (char ch : text) {
        if (ch == ' ') {
            cx += 6 * scale;
            continue;
        }
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        const auto& g = glyphFor(ch);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if ((g[row] >> (4 - col)) & 1U) {
                    SDL_Rect p{cx + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &p);
                }
            }
        }
        cx += 6 * scale;
    }
}

class SdlPortGame {
public:
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
            return false;
        }

        window_ = SDL_CreateWindow("XPlataformer SDL2 C++ Port", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   1067, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window_) {
            return false;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) {
            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        }
        if (!renderer_) {
            return false;
        }

        game_.loadLevels(resolveLevelsDirectory());
        game_.setGameState(xplat::GameState::Menu);
        return true;
    }

    void run() {
        bool running = true;
        auto last = std::chrono::steady_clock::now();

        while (running) {
            running = handleEvents();

            auto now = std::chrono::steady_clock::now();
            const float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            update(std::min(dt, 0.033f));
            render();
        }
    }

    ~SdlPortGame() {
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        SDL_Quit();
    }

private:
    static constexpr int W = 1067;
    static constexpr int H = 600;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    xplat::GameCore game_{W, H};

    SDL_Rect btnPlay_{150, 200, 500, 60};
    SDL_Rect btnCustom_{100, 290, 600, 60};
    SDL_Rect debugPanel_{760, 80, 290, 420};

    bool left_ = false;
    bool right_ = false;
    bool jump_ = false;
    bool touchLeft_ = false;
    bool touchRight_ = false;
    bool touchJump_ = false;
    SDL_FingerID touchLeftFinger_ = -1;
    SDL_FingerID touchRightFinger_ = -1;
    SDL_FingerID touchJumpFinger_ = -1;
    SDL_FingerID touchDragFinger_ = -1;
    bool iKeyPressed_ = false;
    bool debugMode_ = false;
    int debugSelectedLevel_ = 0;
    int selectedMenu_ = 0;
    int mouseX_ = 0;
    int mouseY_ = 0;
    float renderScale_ = 1.0f;
    int renderOffsetX_ = 0;
    int renderOffsetY_ = 0;

    SDL_Rect touchLeftBtn_{20, H - 110, 80, 80};
    SDL_Rect touchRightBtn_{120, H - 110, 80, 80};
    SDL_Rect touchJumpBtn_{W - 110, H - 110, 90, 80};

    std::filesystem::path resolveLevelsDirectory() {
        std::filesystem::path fallback{"levels"};

        const char* prefPathRaw = SDL_GetPrefPath("XPlataformer", "XPlataformer");
        if (!prefPathRaw) {
            return fallback;
        }

        std::filesystem::path prefPath{prefPathRaw};
        SDL_free(const_cast<char*>(prefPathRaw));

        std::filesystem::path levelsDir = prefPath / "levels";
        std::error_code ec;
        std::filesystem::create_directories(levelsDir, ec);
        if (ec) {
            return fallback;
        }

        int copied = 0;
        for (int i = 1; i <= 99; ++i) {
            const std::string name = "level" + std::to_string(i) + ".txt";
            const std::array<std::string, 2> assetCandidates = {"levels/" + name, name};
            bool wroteFile = false;

            for (const auto& assetPath : assetCandidates) {
                SDL_RWops* rw = SDL_RWFromFile(assetPath.c_str(), "rb");
                if (!rw) {
                    continue;
                }

                const Sint64 size = SDL_RWsize(rw);
                if (size <= 0) {
                    SDL_RWclose(rw);
                    continue;
                }

                std::vector<char> bytes(static_cast<size_t>(size));
                const size_t read = SDL_RWread(rw, bytes.data(), 1, bytes.size());
                SDL_RWclose(rw);
                if (read != bytes.size()) {
                    continue;
                }

                std::ofstream out(levelsDir / name, std::ios::binary);
                if (!out.is_open()) {
                    continue;
                }
                out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                if (!out.fail()) {
                    wroteFile = true;
                    ++copied;
                }
                break;
            }

            if (!wroteFile && i > 20) {
                // Avoid scanning too far when the asset set is short.
                break;
            }
        }

        if (copied > 0) {
            return levelsDir;
        }
        return fallback;
    }

    int toLogicalX(int sx) const {
        return static_cast<int>((sx - renderOffsetX_) / std::max(renderScale_, 0.0001f));
    }

    int toLogicalY(int sy) const {
        return static_cast<int>((sy - renderOffsetY_) / std::max(renderScale_, 0.0001f));
    }

    bool pointInRect(int x, int y, const SDL_Rect& r) const {
        return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
    }

    void fingerToLogical(float fx, float fy, int& lx, int& ly) const {
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        const int sx = static_cast<int>(fx * static_cast<float>(ww));
        const int sy = static_cast<int>(fy * static_cast<float>(wh));
        lx = toLogicalX(sx);
        ly = toLogicalY(sy);
    }

    bool handleTouchControlDown(SDL_FingerID fingerId, int lx, int ly) {
        if (touchLeftFinger_ == -1 && pointInRect(lx, ly, touchLeftBtn_)) {
            touchLeftFinger_ = fingerId;
            touchLeft_ = true;
            return true;
        }
        if (touchRightFinger_ == -1 && pointInRect(lx, ly, touchRightBtn_)) {
            touchRightFinger_ = fingerId;
            touchRight_ = true;
            return true;
        }
        if (touchJumpFinger_ == -1 && pointInRect(lx, ly, touchJumpBtn_)) {
            touchJumpFinger_ = fingerId;
            touchJump_ = true;
            return true;
        }
        return false;
    }

    void updateTouchControlState(SDL_FingerID fingerId, int lx, int ly) {
        if (fingerId == touchLeftFinger_) {
            touchLeft_ = pointInRect(lx, ly, touchLeftBtn_);
        }
        if (fingerId == touchRightFinger_) {
            touchRight_ = pointInRect(lx, ly, touchRightBtn_);
        }
        if (fingerId == touchJumpFinger_) {
            touchJump_ = pointInRect(lx, ly, touchJumpBtn_);
        }
    }

    void releaseTouchControl(SDL_FingerID fingerId) {
        if (fingerId == touchLeftFinger_) {
            touchLeftFinger_ = -1;
            touchLeft_ = false;
        }
        if (fingerId == touchRightFinger_) {
            touchRightFinger_ = -1;
            touchRight_ = false;
        }
        if (fingerId == touchJumpFinger_) {
            touchJumpFinger_ = -1;
            touchJump_ = false;
        }
    }

    bool handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return false;
            }

            if (e.type == SDL_MOUSEMOTION) {
                mouseX_ = toLogicalX(e.motion.x);
                mouseY_ = toLogicalY(e.motion.y);
                if (game_.gameState() == xplat::GameState::Menu) {
                    if (pointInRect(mouseX_, mouseY_, btnPlay_)) {
                        selectedMenu_ = 0;
                    } else if (pointInRect(mouseX_, mouseY_, btnCustom_)) {
                        selectedMenu_ = 1;
                    }
                }
                game_.dragTo(mouseX_, mouseY_);
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                const int lx = toLogicalX(e.button.x);
                const int ly = toLogicalY(e.button.y);
                if (game_.gameState() == xplat::GameState::Menu) {
                    if (pointInRect(lx, ly, btnPlay_)) {
                        if (game_.levelCount() > 0) {
                            game_.loadLevel(0);
                            game_.setGameState(xplat::GameState::Playing);
                        }
                    } else if (pointInRect(lx, ly, btnCustom_)) {
                        game_.loadCustomPack("custom_levels");
                        if (game_.levelCount() > 0) {
                            game_.loadLevel(0);
                            game_.setGameState(xplat::GameState::Playing);
                        }
                    }
                } else if (game_.gameState() == xplat::GameState::Playing) {
                    if (debugMode_ && pointInRect(lx, ly, debugPanel_)) {
                        const int rowH = 18;
                        const int listY = debugPanel_.y + 56;
                        if (ly >= listY) {
                            const int clicked = (ly - listY) / rowH;
                            const int visible = std::min(18, game_.levelCount());
                            int startIndex = 0;
                            if (debugSelectedLevel_ >= visible) {
                                startIndex = debugSelectedLevel_ - visible + 1;
                            }
                            const int idx = startIndex + clicked;
                            if (idx >= 0 && idx < game_.levelCount()) {
                                if (idx == debugSelectedLevel_) {
                                    game_.loadLevel(debugSelectedLevel_);
                                    debugMode_ = false;
                                } else {
                                    debugSelectedLevel_ = idx;
                                }
                                continue;
                            }
                        }
                    }
                    game_.startDrag(lx, ly);
                }
            }

            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                game_.endDrag();
            }

            if (e.type == SDL_FINGERDOWN) {
                int lx = 0;
                int ly = 0;
                fingerToLogical(e.tfinger.x, e.tfinger.y, lx, ly);
                mouseX_ = lx;
                mouseY_ = ly;

                if (handleTouchControlDown(e.tfinger.fingerId, lx, ly)) {
                    continue;
                }

                if (game_.gameState() == xplat::GameState::Menu) {
                    if (pointInRect(lx, ly, btnPlay_)) {
                        if (game_.levelCount() > 0) {
                            game_.loadLevel(0);
                            game_.setGameState(xplat::GameState::Playing);
                        }
                    } else if (pointInRect(lx, ly, btnCustom_)) {
                        game_.loadCustomPack("custom_levels");
                        if (game_.levelCount() > 0) {
                            game_.loadLevel(0);
                            game_.setGameState(xplat::GameState::Playing);
                        }
                    }
                } else if (game_.gameState() == xplat::GameState::Playing) {
                    if (debugMode_ && pointInRect(lx, ly, debugPanel_)) {
                        const int rowH = 18;
                        const int listY = debugPanel_.y + 56;
                        if (ly >= listY) {
                            const int clicked = (ly - listY) / rowH;
                            const int visible = std::min(18, game_.levelCount());
                            int startIndex = 0;
                            if (debugSelectedLevel_ >= visible) {
                                startIndex = debugSelectedLevel_ - visible + 1;
                            }
                            const int idx = startIndex + clicked;
                            if (idx >= 0 && idx < game_.levelCount()) {
                                if (idx == debugSelectedLevel_) {
                                    game_.loadLevel(debugSelectedLevel_);
                                    debugMode_ = false;
                                } else {
                                    debugSelectedLevel_ = idx;
                                }
                                continue;
                            }
                        }
                    }

                    if (game_.startDrag(lx, ly)) {
                        touchDragFinger_ = e.tfinger.fingerId;
                    }
                }
            }

            if (e.type == SDL_FINGERMOTION) {
                int lx = 0;
                int ly = 0;
                fingerToLogical(e.tfinger.x, e.tfinger.y, lx, ly);
                mouseX_ = lx;
                mouseY_ = ly;
                updateTouchControlState(e.tfinger.fingerId, lx, ly);
                if (e.tfinger.fingerId == touchDragFinger_) {
                    game_.dragTo(lx, ly);
                }
            }

            if (e.type == SDL_FINGERUP) {
                releaseTouchControl(e.tfinger.fingerId);
                if (e.tfinger.fingerId == touchDragFinger_) {
                    game_.endDrag();
                    touchDragFinger_ = -1;
                }
            }

            if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                const SDL_Keycode key = e.key.keysym.sym;

                if (game_.gameState() == xplat::GameState::Menu) {
                    if (key == SDLK_LEFT || key == SDLK_a) {
                        selectedMenu_ = (selectedMenu_ - 1 + 2) % 2;
                    } else if (key == SDLK_RIGHT || key == SDLK_d) {
                        selectedMenu_ = (selectedMenu_ + 1) % 2;
                    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                        if (selectedMenu_ == 0) {
                            if (game_.levelCount() > 0) {
                                game_.loadLevel(0);
                                game_.setGameState(xplat::GameState::Playing);
                            }
                        } else {
                            game_.loadCustomPack("custom_levels");
                            if (game_.levelCount() > 0) {
                                game_.loadLevel(0);
                                game_.setGameState(xplat::GameState::Playing);
                            }
                        }
                    }
                    continue;
                }

                if (debugMode_) {
                    if (key == SDLK_UP) {
                        debugSelectedLevel_ = std::max(0, debugSelectedLevel_ - 1);
                        continue;
                    }
                    if (key == SDLK_DOWN) {
                        debugSelectedLevel_ = std::min(std::max(0, game_.levelCount() - 1), debugSelectedLevel_ + 1);
                        continue;
                    }
                    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                        if (game_.levelCount() > 0) {
                            game_.loadLevel(debugSelectedLevel_);
                            debugMode_ = false;
                        }
                        continue;
                    }
                }

                switch (key) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        left_ = true;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        right_ = true;
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                    case SDLK_SPACE:
                        jump_ = true;
                        break;
                    case SDLK_ESCAPE:
                        game_.setGameState(xplat::GameState::Menu);
                        selectedMenu_ = 0;
                        break;
                    case SDLK_c:
                        game_.loadCustomPack("custom_levels");
                        break;
                    case SDLK_i:
                        iKeyPressed_ = true;
                        break;
                    case SDLK_e:
                        if (iKeyPressed_) {
                            debugMode_ = !debugMode_;
                            if (debugMode_) {
                                debugSelectedLevel_ = std::max(0, std::min(debugSelectedLevel_, game_.levelCount() - 1));
                            }
                        }
                        break;
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8:
                    case SDLK_9:
                        if (debugMode_) {
                            const int num = static_cast<int>(key - SDLK_1) + 1;
                            if (num > 0 && num <= game_.levelCount()) {
                                game_.loadLevel(num - 1);
                                debugMode_ = false;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            if (e.type == SDL_KEYUP) {
                const SDL_Keycode key = e.key.keysym.sym;
                switch (key) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        left_ = false;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        right_ = false;
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                    case SDLK_SPACE:
                        jump_ = false;
                        break;
                    case SDLK_i:
                        iKeyPressed_ = false;
                        break;
                    default:
                        break;
                }
            }
        }
        return true;
    }

    void update(float dt) {
        if (game_.gameState() != xplat::GameState::Playing) {
            return;
        }

        xplat::InputState input;
        input.left = left_ || touchLeft_;
        input.right = right_ || touchRight_;
        input.jump = jump_ || touchJump_;

        game_.update(dt, input);
    }

    SDL_Rect txRect(int x, int y, int w, int h) const {
        return SDL_Rect{static_cast<int>(x * renderScale_) + renderOffsetX_, static_cast<int>(y * renderScale_) + renderOffsetY_,
                        static_cast<int>(w * renderScale_), static_cast<int>(h * renderScale_)};
    }

    void drawWorldRect(int x, int y, int w, int h, const Color& c, bool camera = true) {
        if (camera) {
            x = static_cast<int>(x - game_.cameraX());
        }
        SDL_Rect r = txRect(x, y, w, h);
        fillRect(renderer_, r.x, r.y, r.w, r.h, c);
    }

    void drawUiRect(int x, int y, int w, int h, const Color& c) {
        SDL_Rect r = txRect(x, y, w, h);
        fillRect(renderer_, r.x, r.y, r.w, r.h, c);
    }

    void drawUiText(int x, int y, int scale, const std::string& text, const Color& c) {
        drawText(renderer_, static_cast<int>(x * renderScale_) + renderOffsetX_, static_cast<int>(y * renderScale_) + renderOffsetY_,
                 std::max(1, static_cast<int>(scale * renderScale_)), text, c);
    }

    void renderTouchControls() {
        drawUiRect(touchLeftBtn_.x, touchLeftBtn_.y, touchLeftBtn_.w, touchLeftBtn_.h, touchLeft_ ? Color{240, 190, 100, 180} : Color{60, 80, 110, 130});
        drawUiRect(touchRightBtn_.x, touchRightBtn_.y, touchRightBtn_.w, touchRightBtn_.h, touchRight_ ? Color{240, 190, 100, 180} : Color{60, 80, 110, 130});
        drawUiRect(touchJumpBtn_.x, touchJumpBtn_.y, touchJumpBtn_.w, touchJumpBtn_.h, touchJump_ ? Color{240, 190, 100, 180} : Color{60, 80, 110, 130});

        drawUiText(touchLeftBtn_.x + 32, touchLeftBtn_.y + 28, 2, "L", {250, 250, 240, 255});
        drawUiText(touchRightBtn_.x + 32, touchRightBtn_.y + 28, 2, "R", {250, 250, 240, 255});
        drawUiText(touchJumpBtn_.x + 10, touchJumpBtn_.y + 28, 2, "JUMP", {250, 250, 240, 255});
    }

    void render() {
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);

        setColor(renderer_, {0, 0, 0, 255});
        SDL_RenderClear(renderer_);

        const float sx = ww / static_cast<float>(W);
        const float sy = wh / static_cast<float>(H);
        renderScale_ = std::max(0.0001f, std::min(sx, sy));
        renderOffsetX_ = static_cast<int>((ww - W * renderScale_) * 0.5f);
        renderOffsetY_ = static_cast<int>((wh - H * renderScale_) * 0.5f);

        if (game_.gameState() == xplat::GameState::Menu) {
            renderMenu();
        } else {
            renderGame();
        }

        SDL_RenderPresent(renderer_);
    }

    void renderMenu() {
        const int scaledW = std::max(1, static_cast<int>(W * renderScale_));
        const int scaledH = std::max(1, static_cast<int>(H * renderScale_));
        const float menuDen = static_cast<float>(std::max(1, scaledH - 1));
        for (int y = 0; y < scaledH; ++y) {
            const float t = y / menuDen;
            const Color c{static_cast<uint8_t>(35 - 30 * t), static_cast<uint8_t>(30 - 25 * t), static_cast<uint8_t>(45 - 35 * t), 255};
            drawLine(renderer_, renderOffsetX_, renderOffsetY_ + y, renderOffsetX_ + scaledW - 1, renderOffsetY_ + y, c);
        }

        drawUiText(380, 70, 3, "XPLATAFORMER", {245, 225, 180, 255});
        drawUiText(500, 105, 2, "RC1", {230, 220, 210, 255});

        const bool hoverPlay = pointInRect(mouseX_, mouseY_, btnPlay_);
        const bool hoverCustom = pointInRect(mouseX_, mouseY_, btnCustom_);

        drawUiRect(btnPlay_.x, btnPlay_.y, btnPlay_.w, btnPlay_.h, (hoverPlay || selectedMenu_ == 0) ? Color{230, 190, 100, 255} : Color{170, 170, 185, 255});
        drawUiRect(btnCustom_.x, btnCustom_.y, btnCustom_.w, btnCustom_.h, (hoverCustom || selectedMenu_ == 1) ? Color{230, 190, 100, 255} : Color{170, 170, 185, 255});

        drawUiText(btnPlay_.x + 210, btnPlay_.y + 20, 2, "PLAY", {30, 20, 15, 255});
        drawUiText(btnCustom_.x + 90, btnCustom_.y + 20, 2, "PLAY CUSTOM LEVEL PACK", {30, 20, 15, 255});

        drawUiText(245, H - 46, 2, "ARROWS OR MOUSE. ENTER OR CLICK.", {230, 230, 230, 255});
        drawUiText(225, H - 26, 2, "CUSTOM PACK LOOKS AT ./CUSTOM_LEVELS", {170, 210, 235, 255});
    }

    void renderGame() {
        const int scaledW = std::max(1, static_cast<int>(W * renderScale_));
        const int scaledH = std::max(1, static_cast<int>(H * renderScale_));
        const float gameDen = static_cast<float>(std::max(1, scaledH - 1));
        for (int y = 0; y < scaledH; ++y) {
            const float t = y / gameDen;
            const Color c{static_cast<uint8_t>(35 - 25 * t), static_cast<uint8_t>(60 - 50 * t), static_cast<uint8_t>(120 - 95 * t), 255};
            drawLine(renderer_, renderOffsetX_, renderOffsetY_ + y, renderOffsetX_ + scaledW - 1, renderOffsetY_ + y, c);
        }

        for (const auto& p : game_.platforms()) {
            drawWorldRect(p.x, p.y, p.w, p.h, {110, 90, 70, 255});
        }
        for (const auto& p : game_.invisiblePlatforms()) {
            drawWorldRect(p.x, p.y, p.w, p.h, {85, 85, 105, 120});
        }
        for (const auto& mp : game_.movingPlatforms()) {
            const auto r = mp.rect();
            drawWorldRect(r.x, r.y, r.w, r.h, {90, 130, 185, 255});
        }
        for (const auto& dp : game_.dragPlatforms()) {
            drawWorldRect(dp.rect.x, dp.rect.y, dp.rect.w, dp.rect.h, {120, 165, 225, 255});
        }

        for (const auto& t : game_.killers()) {
            const int x1 = static_cast<int>((t.xs[0] - game_.cameraX()) * renderScale_) + renderOffsetX_;
            const int y1 = static_cast<int>(t.ys[0] * renderScale_) + renderOffsetY_;
            const int x2 = static_cast<int>((t.xs[1] - game_.cameraX()) * renderScale_) + renderOffsetX_;
            const int y2 = static_cast<int>(t.ys[1] * renderScale_) + renderOffsetY_;
            const int x3 = static_cast<int>((t.xs[2] - game_.cameraX()) * renderScale_) + renderOffsetX_;
            const int y3 = static_cast<int>(t.ys[2] * renderScale_) + renderOffsetY_;
            fillTriangle(renderer_, x1, y1, x2, y2, x3, y3, {170, 30, 30, 255});
            drawLine(renderer_, x1, y1, x2, y2, {255, 220, 220, 255});
            drawLine(renderer_, x2, y2, x3, y3, {255, 220, 220, 255});
            drawLine(renderer_, x3, y3, x1, y1, {255, 220, 220, 255});
        }

        const int lavaTop = H - 8;
        drawWorldRect(-100, lavaTop, static_cast<int>(game_.levelWidth()) + 250, 28, {210, 50, 10, 220});

        for (const auto& b : game_.bosses()) {
            if (!b.alive) {
                continue;
            }
            drawWorldRect(b.rect.x, b.rect.y, b.rect.w, b.rect.h, b.tired ? Color{200, 150, 70, 255} : Color{180, 40, 40, 255});
            if (b.tired) {
                drawUiText(static_cast<int>(b.rect.x - game_.cameraX()) + 4, b.rect.y - 12, 1, "TIRED", {255, 240, 130, 255});
            }
        }

        for (const auto& f : game_.fireballs()) {
            const int cx = static_cast<int>((f.rect.x - game_.cameraX()) * renderScale_) + renderOffsetX_ + static_cast<int>(f.rect.w * renderScale_ / 2);
            const int cy = static_cast<int>(f.rect.y * renderScale_) + renderOffsetY_ + static_cast<int>(f.rect.h * renderScale_ / 2);
            fillCircle(renderer_, cx, cy, std::max(2, static_cast<int>(6 * renderScale_)), {255, 120, 30, 255});
        }

        if (game_.checkpoint().has_value()) {
            const auto cp = game_.checkpoint().value();
            drawWorldRect(cp.x, cp.y, cp.w, cp.h, game_.checkpointActive() ? Color{80, 220, 150, 255} : Color{150, 150, 150, 255});
        }

        const auto g = game_.goal();
        drawWorldRect(g.x, g.y, g.w, g.h, {235, 190, 90, 255});

        const auto p = game_.playerRect();
        drawWorldRect(p.x, p.y, p.w, p.h, {70, 150, 190, 255});

        drawUiRect(10, 10, 380, 100, {45, 55, 70, 220});
        drawUiText(20, 22, 2, "CONTROLS: A D MOVE, W OR SPACE JUMP", {230, 235, 245, 255});
        drawUiText(20, 40, 2, "LEVEL: " + game_.levelName(game_.currentLevel()), {230, 235, 245, 255});
        drawUiText(20, 58, 2, "ATTEMPT: " + std::to_string(game_.attemptCount()) + "  HP: " + std::to_string(game_.playerHp()) + "/3", {230, 235, 245, 255});
        drawUiText(20, 76, 1, "TOUCH: LEFT/RIGHT/JUMP. DRAG BLUE BLOCKS.", {205, 215, 235, 255});

        if (game_.showingMessage()) {
            drawUiRect(330, 95, 410, 34, {245, 230, 185, 235});
            drawUiText(342, 106, 2, game_.messageText(), {45, 35, 20, 255});
        }

        renderTouchControls();

        if (debugMode_) {
            drawUiRect(debugPanel_.x, debugPanel_.y, debugPanel_.w, debugPanel_.h, {50, 58, 72, 240});
            drawUiText(debugPanel_.x + 12, debugPanel_.y + 12, 2, "DEBUG LEVEL SELECTOR", {245, 245, 210, 255});
            drawUiText(debugPanel_.x + 12, debugPanel_.y + 30, 1, "UP DOWN SELECT ENTER LOAD", {220, 226, 235, 255});

            const int visible = std::min(18, game_.levelCount());
            int startIndex = 0;
            if (debugSelectedLevel_ >= visible) {
                startIndex = debugSelectedLevel_ - visible + 1;
            }

            for (int i = 0; i < visible; ++i) {
                const int idx = startIndex + i;
                if (idx >= game_.levelCount()) {
                    break;
                }
                const int ry = debugPanel_.y + 56 + i * 18;
                if (idx == debugSelectedLevel_) {
                    drawUiRect(debugPanel_.x + 8, ry - 10, debugPanel_.w - 16, 16, {230, 190, 95, 220});
                }
                drawUiText(debugPanel_.x + 14, ry - 8, 1, std::to_string(idx + 1) + ". " + game_.levelName(idx), {32, 26, 20, 255});
            }
        }
    }
};

} // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    SdlPortGame game;
    if (!game.init()) {
        return 1;
    }

    game.run();
    return 0;
}
