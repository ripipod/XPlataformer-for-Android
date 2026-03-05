#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xplat {

enum class GameState { Menu, Playing, Demo };

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] bool intersects(const Rect& other) const;
    [[nodiscard]] bool contains(int px, int py) const;
};

struct Triangle {
    std::array<int, 3> xs{};
    std::array<int, 3> ys{};
};

struct Intersection {
    int w = 0;
    int h = 0;
    [[nodiscard]] bool valid() const { return w > 0 && h > 0; }
};

struct MovingPlatform {
    float x = 0.0f;
    float y = 0.0f;
    int w = 0;
    int h = 0;
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
    float speed = 0.0f;
    float dirX = 0.0f;
    float dirY = 0.0f;

    MovingPlatform() = default;
    MovingPlatform(int x_, int y_, int w_, int h_, float minX_, float maxX_, float minY_, float maxY_, float speed_);

    void update(float dt);
    [[nodiscard]] Rect rect() const;
};

struct DraggablePlatform {
    Rect rect;
};

struct Boss {
    Rect rect;
    int hp = 1;
    int maxHp = 1;
    bool alive = true;
    bool tired = false;
    float tiredTimer = 0.0f;
    float attackTimer = 3.6f;
    float fireTimer = 1.0f;
    int homeY = 0;
    int tiredY = 0;

    Boss() = default;
    Boss(int x, int y, int w, int h, int hp_);
};

struct Fireball {
    Rect rect;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float spawnX = 0.0f;
    float spawnY = 0.0f;
    float maxTravel = 520.0f;
    bool alive = true;

    Fireball() = default;
    Fireball(int x_, int y_, int w_, int h_, float vx_, float vy_);

    void update(float dt);
};

struct Level {
    std::string name;
    int startX = 50;
    int startY = 400;
    std::vector<Rect> platforms;
    std::vector<Triangle> killers;
    std::vector<MovingPlatform> movingPlatforms;
    std::vector<Rect> invisiblePlatforms;
    std::vector<DraggablePlatform> dragPlatforms;
    std::vector<Boss> bosses;
    Rect goal{720, 210, 40, 40};
    std::optional<Rect> checkpoint;

    static Level defaultLevel1();
    static Level defaultLevel2();
    static Level fromFile(const std::filesystem::path& file);
};

struct InputState {
    bool left = false;
    bool right = false;
    bool jump = false;
};

class GameCore {
public:
    explicit GameCore(int width = 1067, int height = 600);

    bool loadLevels(const std::filesystem::path& levelsDir);
    bool loadCustomPack(const std::filesystem::path& directory);
    void loadLevel(int idx);
    void update(float dt, const InputState& input);

    bool startDrag(int logicalX, int logicalY);
    void dragTo(int logicalX, int logicalY);
    void endDrag();

    [[nodiscard]] int width() const { return W; }
    [[nodiscard]] int height() const { return H; }
    [[nodiscard]] float cameraX() const { return cameraX_; }
    [[nodiscard]] float levelWidth() const { return levelWidth_; }
    [[nodiscard]] const Rect& playerRect() const { return player_; }
    [[nodiscard]] const std::vector<Rect>& platforms() const { return platforms_; }
    [[nodiscard]] const std::vector<Rect>& invisiblePlatforms() const { return invisiblePlatforms_; }
    [[nodiscard]] const std::vector<DraggablePlatform>& dragPlatforms() const { return dragPlatforms_; }
    [[nodiscard]] const std::vector<Triangle>& killers() const { return killers_; }
    [[nodiscard]] const std::vector<MovingPlatform>& movingPlatforms() const { return movingPlatforms_; }
    [[nodiscard]] const std::vector<Boss>& bosses() const { return bosses_; }
    [[nodiscard]] const std::vector<Fireball>& fireballs() const { return fireballs_; }
    [[nodiscard]] const Rect& goal() const { return goal_; }
    [[nodiscard]] std::optional<Rect> checkpoint() const { return checkpoint_; }
    [[nodiscard]] bool checkpointActive() const { return checkpointActive_; }
    [[nodiscard]] int currentLevel() const { return currentLevel_; }
    [[nodiscard]] int levelCount() const { return static_cast<int>(levels_.size()); }
    [[nodiscard]] const std::string& levelName(int idx) const;
    [[nodiscard]] const std::vector<Level>& levels() const { return levels_; }
    [[nodiscard]] int attemptCount() const { return attemptCount_; }
    [[nodiscard]] int playerHp() const { return playerHp_; }
    [[nodiscard]] bool onGround() const { return onGround_; }
    [[nodiscard]] GameState gameState() const { return gameState_; }
    [[nodiscard]] bool showingMessage() const { return showingMessage_; }
    [[nodiscard]] const std::string& messageText() const { return messageText_; }

    void setGameState(GameState state) { gameState_ = state; }

private:
    int W;
    int H;
    int lavaKillY_;

    GameState gameState_ = GameState::Menu;

    Rect player_{};
    float posX_ = 0.0f;
    float posY_ = 0.0f;
    float vx_ = 0.0f;
    float vy_ = 0.0f;
    bool onGround_ = false;

    float cameraX_ = 0.0f;
    float levelWidth_ = 1067.0f;

    std::vector<Level> levels_;
    int currentLevel_ = 0;
    int attemptCount_ = 1;

    std::vector<Rect> platforms_;
    std::vector<Triangle> killers_;
    std::vector<MovingPlatform> movingPlatforms_;
    std::vector<Rect> invisiblePlatforms_;
    std::vector<DraggablePlatform> dragPlatforms_;
    std::vector<Boss> bosses_;
    std::vector<Fireball> fireballs_;

    Rect goal_{};
    std::optional<Rect> checkpoint_;
    bool checkpointActive_ = false;
    int checkpointRespawnX_ = 0;
    int checkpointRespawnY_ = 0;
    int checkpointDeaths_ = 0;

    int playerMaxHp_ = 3;
    int playerHp_ = 3;
    int damageInvulnTimer_ = 0;
    bool respawnPending_ = false;
    int explosionTimer_ = 0;
    float explosionX_ = 0.0f;
    float explosionY_ = 0.0f;
    std::string pendingDeathReason_;

    bool showingMessage_ = false;
    int messageTimer_ = 0;
    std::string messageText_;
    int pendingNextLevel_ = -1;

    DraggablePlatform* activeDrag_ = nullptr;
    int dragOffsetX_ = 0;
    int dragOffsetY_ = 0;

    std::vector<int> mouseLevels_;

    void createDefaultLevels(const std::filesystem::path& levelsDir) const;
    void updatePlaying(float dt, const InputState& input);
    void updateDemo(float dt, const InputState& input);
    void updateBossesAndFire(float dt);
    void resolveRectCollision(const Rect& obstacle);
    void resolveStageCollision();
    void respawnPlayer(const std::string& reason);
    void damagePlayer(const std::string& reason);
    [[nodiscard]] bool allBossesDefeated() const;
    [[nodiscard]] bool containsAnyCorner(const Triangle& tri, const Rect& r) const;
    void recomputeMouseLevels();
};

} // namespace xplat
