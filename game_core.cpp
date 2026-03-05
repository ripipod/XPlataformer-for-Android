#include "game_core.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace xplat {

namespace {

Intersection intersection(const Rect& a, const Rect& b) {
    const int left = std::max(a.x, b.x);
    const int top = std::max(a.y, b.y);
    const int right = std::min(a.x + a.w, b.x + b.w);
    const int bottom = std::min(a.y + a.h, b.y + b.h);
    if (right <= left || bottom <= top) {
        return {0, 0};
    }
    return {right - left, bottom - top};
}

bool pointInTriangle(float px, float py, const Triangle& tri) {
    auto sign = [](float x1, float y1, float x2, float y2, float x3, float y3) {
        return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
    };

    const float x1 = static_cast<float>(tri.xs[0]);
    const float y1 = static_cast<float>(tri.ys[0]);
    const float x2 = static_cast<float>(tri.xs[1]);
    const float y2 = static_cast<float>(tri.ys[1]);
    const float x3 = static_cast<float>(tri.xs[2]);
    const float y3 = static_cast<float>(tri.ys[2]);

    const bool b1 = sign(px, py, x1, y1, x2, y2) < 0.0f;
    const bool b2 = sign(px, py, x2, y2, x3, y3) < 0.0f;
    const bool b3 = sign(px, py, x3, y3, x1, y1) < 0.0f;

    return (b1 == b2) && (b2 == b3);
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::istringstream in(s);
    std::vector<std::string> parts;
    std::string part;
    while (in >> part) {
        parts.push_back(part);
    }
    return parts;
}

std::string trim(std::string s) {
    const auto isNotSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), isNotSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), isNotSpace).base(), s.end());
    return s;
}

int extractLevelNumber(const std::string& name) {
    std::string digits;
    digits.reserve(name.size());
    for (char c : name) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }
    if (digits.empty()) {
        return std::numeric_limits<int>::max();
    }
    try {
        return std::stoi(digits);
    } catch (...) {
        return std::numeric_limits<int>::max();
    }
}

} // namespace

bool Rect::intersects(const Rect& other) const {
    return x < other.x + other.w && x + w > other.x && y < other.y + other.h && y + h > other.y;
}

bool Rect::contains(int px, int py) const {
    return px >= x && py >= y && px < (x + w) && py < (y + h);
}

MovingPlatform::MovingPlatform(int x_, int y_, int w_, int h_, float minX_, float maxX_, float minY_, float maxY_, float speed_)
    : x(static_cast<float>(x_)),
      y(static_cast<float>(y_)),
      w(w_),
      h(h_),
      minX(minX_),
      maxX(maxX_),
      minY(minY_),
      maxY(maxY_),
      speed(speed_) {
    if (maxX > minX) {
        dirX = 1.0f;
        dirY = 0.0f;
    } else {
        dirX = 0.0f;
        dirY = 1.0f;
    }
}

void MovingPlatform::update(float dt) {
    x += dirX * speed * dt;
    y += dirY * speed * dt;

    if (dirX != 0.0f) {
        if (x <= minX) {
            x = minX;
            dirX = 1.0f;
        }
        if (x >= maxX) {
            x = maxX;
            dirX = -1.0f;
        }
    } else {
        if (y <= minY) {
            y = minY;
            dirY = 1.0f;
        }
        if (y >= maxY) {
            y = maxY;
            dirY = -1.0f;
        }
    }
}

Rect MovingPlatform::rect() const {
    return Rect{static_cast<int>(x), static_cast<int>(y), w, h};
}

Boss::Boss(int x, int y, int w, int h, int hp_)
    : rect{x, y, w, h}, hp(std::max(1, hp_)), maxHp(std::max(1, hp_)), homeY(y), tiredY(y + 70) {}

Fireball::Fireball(int x_, int y_, int w_, int h_, float vx_, float vy_)
    : rect{x_, y_, w_, h_}, x(static_cast<float>(x_)), y(static_cast<float>(y_)), vx(vx_), vy(vy_), spawnX(static_cast<float>(x_)), spawnY(static_cast<float>(y_)) {}

void Fireball::update(float dt) {
    x += vx * dt;
    y += vy * dt;
    rect.x = static_cast<int>(x);
    rect.y = static_cast<int>(y);

    const float dx = x - spawnX;
    const float dy = y - spawnY;
    if ((dx * dx + dy * dy) > (maxTravel * maxTravel)) {
        alive = false;
        return;
    }

    if (rect.x < -80 || rect.y < -80 || rect.x > 5000 || rect.y > 2000) {
        alive = false;
    }
}

Level Level::defaultLevel1() {
    Level l;
    l.name = "Level 1-1";
    l.startX = 50;
    l.startY = 400;
    l.platforms.push_back(Rect{0, 550, 800, 50});
    l.platforms.push_back(Rect{150, 470, 120, 20});
    l.platforms.push_back(Rect{320, 380, 100, 20});
    l.platforms.push_back(Rect{480, 340, 140, 20});
    l.platforms.push_back(Rect{660, 260, 100, 20});
    l.killers.push_back(Triangle{{400, 450, 425}, {550, 550, 520}});
    l.killers.push_back(Triangle{{560, 610, 585}, {400, 400, 370}});
    l.goal = Rect{720, 210, 40, 40};
    return l;
}

Level Level::defaultLevel2() {
    Level l;
    l.name = "Level 1-2";
    l.startX = 50;
    l.startY = 400;
    l.platforms.push_back(Rect{0, 550, 800, 50});
    l.platforms.push_back(Rect{120, 470, 100, 20});
    l.platforms.push_back(Rect{260, 420, 80, 20});
    l.platforms.push_back(Rect{420, 360, 120, 20});
    l.platforms.push_back(Rect{600, 300, 120, 20});
    l.killers.push_back(Triangle{{300, 350, 325}, {550, 550, 520}});
    l.goal = Rect{680, 260, 40, 40};
    return l;
}

Level Level::fromFile(const std::filesystem::path& file) {
    std::ifstream in(file);
    if (!in.is_open()) {
        throw std::runtime_error("Could not open level file: " + file.string());
    }

    Level l;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto pos = line.find(':');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string data = trim(line.substr(pos + 1));
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const auto parts = splitWhitespace(data);

        if (key == "name") {
            l.name = data;
        } else if (key == "start" && parts.size() >= 2) {
            l.startX = std::stoi(parts[0]);
            l.startY = std::stoi(parts[1]);
        } else if (key == "platform" && parts.size() >= 4) {
            l.platforms.push_back(Rect{std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3])});
        } else if (key == "movingplatform" && parts.size() >= 9) {
            l.movingPlatforms.emplace_back(std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3]),
                                           std::stof(parts[4]), std::stof(parts[5]), std::stof(parts[6]), std::stof(parts[7]),
                                           std::stof(parts[8]));
        } else if (key == "dragplatform" && parts.size() >= 4) {
            l.dragPlatforms.push_back(DraggablePlatform{Rect{std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3])}});
        } else if (key == "invisibleplatform" && parts.size() >= 4) {
            l.invisiblePlatforms.push_back(Rect{std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3])});
        } else if (key == "boss" && parts.size() >= 4) {
            const int hp = (parts.size() > 4) ? std::stoi(parts[4]) : 3;
            l.bosses.emplace_back(std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3]), hp);
        } else if (key == "killer" && parts.size() >= 6) {
            l.killers.push_back(Triangle{{std::stoi(parts[0]), std::stoi(parts[2]), std::stoi(parts[4])},
                                         {std::stoi(parts[1]), std::stoi(parts[3]), std::stoi(parts[5])}});
        } else if (key == "goal" && parts.size() >= 4) {
            l.goal = Rect{std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3])};
        } else if (key == "checkpoint" && parts.size() >= 4) {
            l.checkpoint = Rect{std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3])};
        }
    }

    return l;
}

GameCore::GameCore(int width, int height)
    : W(width), H(height), lavaKillY_(height + 80) {}

const std::string& GameCore::levelName(int idx) const {
    static const std::string empty;
    if (idx < 0 || idx >= static_cast<int>(levels_.size())) {
        return empty;
    }
    return levels_[idx].name;
}

bool GameCore::loadLevels(const std::filesystem::path& levelsDir) {
    levels_.clear();
    try {
        std::filesystem::path dir = levelsDir;
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            std::filesystem::create_directories(dir);
            createDefaultLevels(dir);
        }

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto ext = entry.path().extension().string();
            if (ext == ".txt" || ext == ".lvl") {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            createDefaultLevels(dir);
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                const auto ext = entry.path().extension().string();
                if (ext == ".txt" || ext == ".lvl") {
                    files.push_back(entry.path());
                }
            }
        }

        std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
            const int na = extractLevelNumber(a.filename().string());
            const int nb = extractLevelNumber(b.filename().string());
            if (na != nb) {
                return na < nb;
            }
            return a.filename().string() < b.filename().string();
        });

        for (const auto& file : files) {
            try {
                levels_.push_back(Level::fromFile(file));
            } catch (...) {
                // Skip malformed level files and keep loading others.
            }
        }
    } catch (...) {
        // Filesystem access can fail on Android/emulators; default levels are used below.
    }

    if (levels_.empty()) {
        levels_.push_back(Level::defaultLevel1());
        levels_.push_back(Level::defaultLevel2());
    }

    recomputeMouseLevels();
    return !levels_.empty();
}

bool GameCore::loadCustomPack(const std::filesystem::path& directory) {
    levels_.clear();
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return false;
    }

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext == ".txt" || ext == ".lvl") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        const int na = extractLevelNumber(a.filename().string());
        const int nb = extractLevelNumber(b.filename().string());
        if (na != nb) {
            return na < nb;
        }
        return a.filename().string() < b.filename().string();
    });

    for (const auto& file : files) {
        try {
            levels_.push_back(Level::fromFile(file));
        } catch (...) {
            // Skip malformed custom levels.
        }
    }

    recomputeMouseLevels();
    return !levels_.empty();
}

void GameCore::createDefaultLevels(const std::filesystem::path& levelsDir) const {
    for (int i = 1; i <= 39; ++i) {
        const auto file = levelsDir / ("level" + (i < 10 ? std::string("0") : std::string("")) + std::to_string(i) + ".txt");
        std::ofstream out(file);
        if (!out.is_open()) {
            continue;
        }

        const int world = ((i - 1) / 10) + 1;
        const int indexInWorld = ((i - 1) % 10) + 1;
        const bool hard = indexInWorld == 10;

        out << "name: World " << world << " - " << (hard ? "Hard " + std::to_string(world) : "Level " + std::to_string(indexInWorld)) << "\n";
        out << "start: 50 400\n";
        out << "platform: 0 550 1200 50\n";

        const int offsetX = 120 * (indexInWorld - 1);
        const int step = hard ? 70 : 90;
        const int count = hard ? 12 : 6;
        for (int p = 0; p < count; ++p) {
            const int px = 120 + offsetX + p * step;
            const int py = 520 - (p % 4) * 60;
            const int pw = 80 + (p % 3) * 20;
            out << "platform: " << px << ' ' << py << ' ' << pw << " 20\n";
        }

        const int traps = hard ? 6 : 3;
        for (int t = 0; t < traps; ++t) {
            const int tx = 260 + offsetX + t * 140;
            const int ty = 550;
            out << "killer: " << tx << ' ' << ty << ' ' << (tx + 40) << ' ' << ty << ' ' << (tx + 20) << ' ' << (ty - 30) << "\n";
        }

        if (world >= 2) {
            const int dx = 300 + offsetX;
            const int dy = 260;
            out << "dragplatform: " << dx << ' ' << dy << " 120 20\n";
            if (hard) {
                out << "dragplatform: " << (dx + 220) << ' ' << (dy - 80) << " 120 20\n";
            }
        }

        if (indexInWorld >= 5) {
            out << "invisibleplatform: " << (500 + offsetX) << " 320 100 15\n";
        }

        if (hard) {
            out << "boss: " << (800 + offsetX) << " 350 80 80 5\n";
        }

        out << "goal: " << (950 + offsetX) << " 260 40 40\n";
    }
}

void GameCore::loadLevel(int idx) {
    if (idx < 0 || idx >= static_cast<int>(levels_.size())) {
        return;
    }

    currentLevel_ = idx;
    attemptCount_ = 1;
    const Level& l = levels_[idx];

    platforms_ = l.platforms;
    killers_ = l.killers;
    movingPlatforms_ = l.movingPlatforms;
    invisiblePlatforms_ = l.invisiblePlatforms;
    dragPlatforms_ = l.dragPlatforms;
    bosses_ = l.bosses;
    fireballs_.clear();

    goal_ = l.goal;
    checkpoint_ = l.checkpoint;
    checkpointActive_ = false;
    checkpointDeaths_ = 0;

    player_ = Rect{l.startX, l.startY, 32, 48};
    posX_ = static_cast<float>(player_.x);
    posY_ = static_cast<float>(player_.y);
    vx_ = 0.0f;
    vy_ = 0.0f;
    onGround_ = false;

    playerHp_ = playerMaxHp_;
    damageInvulnTimer_ = 0;
    respawnPending_ = false;
    explosionTimer_ = 0;
    cameraX_ = 0.0f;

    levelWidth_ = static_cast<float>(W);
    for (const Rect& p : platforms_) {
        levelWidth_ = std::max(levelWidth_, static_cast<float>(p.x + p.w));
    }
    for (const Triangle& k : killers_) {
        levelWidth_ = std::max(levelWidth_, static_cast<float>(std::max({k.xs[0], k.xs[1], k.xs[2]})));
    }
    for (const MovingPlatform& mp : movingPlatforms_) {
        levelWidth_ = std::max(levelWidth_, mp.maxX + static_cast<float>(mp.w));
    }
    for (const Boss& b : bosses_) {
        levelWidth_ = std::max(levelWidth_, static_cast<float>(b.rect.x + b.rect.w));
    }
    if (checkpoint_.has_value()) {
        levelWidth_ = std::max(levelWidth_, static_cast<float>(checkpoint_->x + checkpoint_->w));
    }
    levelWidth_ = std::max(levelWidth_, static_cast<float>(goal_.x + goal_.w));
    levelWidth_ += 50.0f;
}

void GameCore::update(float dt, const InputState& input) {
    switch (gameState_) {
        case GameState::Menu:
            return;
        case GameState::Demo:
            updateDemo(dt, input);
            return;
        case GameState::Playing:
            updatePlaying(dt, input);
            return;
    }
}

void GameCore::updatePlaying(float dt, const InputState& input) {
    if (damageInvulnTimer_ > 0) {
        --damageInvulnTimer_;
    }

    if (respawnPending_) {
        --explosionTimer_;
        if (explosionTimer_ <= 0) {
            respawnPlayer(pendingDeathReason_.empty() ? "You died!" : pendingDeathReason_);
            pendingDeathReason_.clear();
        }
        return;
    }

    for (auto& mp : movingPlatforms_) {
        mp.update(dt);
    }

    if (input.left) {
        vx_ = -180.0f;
    } else if (input.right) {
        vx_ = 180.0f;
    } else {
        vx_ = 0.0f;
    }

    if (input.jump && onGround_) {
        vy_ = -520.0f;
        onGround_ = false;
    }

    vy_ += 1200.0f * dt;
    posX_ += vx_ * dt;
    posY_ += vy_ * dt;

    player_.x = std::max(0, static_cast<int>(posX_));
    const int maxPlayerX = std::max(0, static_cast<int>(levelWidth_) - player_.w);
    if (player_.x > maxPlayerX) {
        player_.x = maxPlayerX;
    }
    posX_ = static_cast<float>(player_.x);
    player_.y = static_cast<int>(posY_);

    cameraX_ = posX_ - static_cast<float>(W) / 4.0f;
    if (cameraX_ < 0.0f) {
        cameraX_ = 0.0f;
    }
    if (cameraX_ + static_cast<float>(W) > levelWidth_) {
        cameraX_ = levelWidth_ - static_cast<float>(W);
    }

    resolveStageCollision();

    updateBossesAndFire(dt);

    for (auto& b : bosses_) {
        if (!b.alive) {
            continue;
        }
        if (!player_.intersects(b.rect)) {
            continue;
        }

        const auto inter = intersection(player_, b.rect);
        if (!inter.valid()) {
            continue;
        }

        const bool stompFromTop = b.tired && vy_ > 60.0f && (player_.y + player_.h) <= (b.rect.y + std::max(14, b.rect.h / 2));
        if (stompFromTop) {
            --b.hp;
            if (b.hp <= 0) {
                b.alive = false;
            }
            player_.y = b.rect.y - player_.h;
            posY_ = static_cast<float>(player_.y);
            vy_ = -420.0f;
            onGround_ = false;
            b.tired = false;
            b.tiredTimer = 0.0f;
            b.attackTimer = 1.8f;
            b.rect.y = b.homeY;
        } else {
            damagePlayer("Boss hit!");
        }
    }

    for (const auto& t : killers_) {
        if (containsAnyCorner(t, player_)) {
            damagePlayer("You got spiked!");
        }
    }

    for (int i = static_cast<int>(fireballs_.size()) - 1; i >= 0; --i) {
        fireballs_[i].update(dt);
        if (!fireballs_[i].alive) {
            fireballs_.erase(fireballs_.begin() + i);
            continue;
        }
        if (player_.intersects(fireballs_[i].rect)) {
            fireballs_.erase(fireballs_.begin() + i);
            damagePlayer("BOOM! Fireball hit.");
            break;
        }
    }

    if (checkpoint_.has_value() && player_.intersects(*checkpoint_) && !checkpointActive_) {
        checkpointActive_ = true;
        checkpointDeaths_ = 0;
        checkpointRespawnX_ = checkpoint_->x + std::max(0, (checkpoint_->w - player_.w) / 2);
        checkpointRespawnY_ = checkpoint_->y - player_.h;
        showingMessage_ = true;
        messageText_ = "Checkpoint reached!";
        messageTimer_ = 60;
    }

    if (player_.y > lavaKillY_) {
        damagePlayer("You fell into lava!");
        return;
    }

    if (player_.intersects(goal_) && !showingMessage_ && allBossesDefeated()) {
        int next = currentLevel_ + 1;
        bool wrapped = false;
        if (next >= static_cast<int>(levels_.size())) {
            wrapped = true;
            next = 0;
        }
        showingMessage_ = true;
        messageTimer_ = 84;
        messageText_ = std::string("Level Complete!") + (wrapped ? " (looping to first level)" : "");
        pendingNextLevel_ = next;
    }

    if (showingMessage_) {
        --messageTimer_;
        if (messageTimer_ <= 0) {
            showingMessage_ = false;
            if (pendingNextLevel_ >= 0) {
                loadLevel(pendingNextLevel_);
                pendingNextLevel_ = -1;
            }
        }
    }
}

void GameCore::updateDemo(float dt, const InputState& input) {
    for (auto& mp : movingPlatforms_) {
        mp.update(dt);
    }

    vx_ = 0.0f;
    if (input.jump && onGround_) {
        vy_ = -520.0f;
        onGround_ = false;
    }

    vy_ += 1200.0f * dt;
    posX_ += vx_ * dt;
    posY_ += vy_ * dt;

    player_.x = std::max(0, static_cast<int>(posX_));
    const int maxPlayerX = std::max(0, static_cast<int>(levelWidth_) - player_.w);
    if (player_.x > maxPlayerX) {
        player_.x = maxPlayerX;
    }
    posX_ = static_cast<float>(player_.x);
    player_.y = static_cast<int>(posY_);

    cameraX_ = posX_ - static_cast<float>(W) / 4.0f;
    if (cameraX_ < 0.0f) {
        cameraX_ = 0.0f;
    }
    if (cameraX_ + static_cast<float>(W) > levelWidth_) {
        cameraX_ = levelWidth_ - static_cast<float>(W);
    }

    resolveStageCollision();

    for (const auto& t : killers_) {
        if (!containsAnyCorner(t, player_)) {
            continue;
        }
        ++attemptCount_;
        const Level& l = levels_[currentLevel_];
        posX_ = static_cast<float>(l.startX);
        posY_ = static_cast<float>(l.startY);
        vy_ = 0.0f;
        vx_ = 0.0f;
        player_.x = static_cast<int>(posX_);
        player_.y = static_cast<int>(posY_);
    }

    if (player_.y > lavaKillY_) {
        ++attemptCount_;
        const Level& l = levels_[currentLevel_];
        posX_ = static_cast<float>(l.startX);
        posY_ = static_cast<float>(l.startY);
        vy_ = 0.0f;
        vx_ = 0.0f;
        player_.x = static_cast<int>(posX_);
        player_.y = static_cast<int>(posY_);
    }

    if (player_.intersects(goal_) && allBossesDefeated()) {
        showingMessage_ = true;
        messageTimer_ = 84;
        messageText_ = "Demo Complete!";
        pendingNextLevel_ = -1;
    }

    if (showingMessage_) {
        --messageTimer_;
        if (messageTimer_ <= 0) {
            showingMessage_ = false;
        }
    }
}

void GameCore::updateBossesAndFire(float dt) {
    for (auto& b : bosses_) {
        if (!b.alive) {
            continue;
        }

        if (b.tired) {
            b.rect.y = std::min(b.tiredY, b.rect.y + std::max(1, static_cast<int>(260.0f * dt)));
            b.tiredTimer -= dt;
            if (b.tiredTimer <= 0.0f) {
                b.tired = false;
                b.attackTimer = 3.6f;
            }
            continue;
        }

        if (b.rect.y > b.homeY) {
            b.rect.y = std::max(b.homeY, b.rect.y - std::max(1, static_cast<int>(220.0f * dt)));
        }

        b.attackTimer -= dt;
        b.fireTimer -= dt;

        if (b.attackTimer <= 0.0f) {
            b.tired = true;
            b.tiredTimer = 2.2f;
            b.fireTimer = 0.0f;
            continue;
        }

        if (b.fireTimer <= 0.0f) {
            const float px = static_cast<float>(player_.x + player_.w / 2);
            const float py = static_cast<float>(player_.y + player_.h / 2);
            const float bx = static_cast<float>(b.rect.x + b.rect.w / 2);
            const float by = static_cast<float>(b.rect.y + b.rect.h / 2);
            const float dx = px - bx;
            const float dy = py - by;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len < 0.0001f) {
                len = 1.0f;
            }
            const float speed = 280.0f;
            const float vxf = (dx / len) * speed;
            const float vyf = (dy / len) * speed;
            fireballs_.emplace_back(static_cast<int>(bx) - 6, static_cast<int>(by) - 6, 12, 12, vxf, vyf);
            b.fireTimer = 1.15f;
        }
    }
}

void GameCore::resolveRectCollision(const Rect& obstacle) {
    if (!player_.intersects(obstacle)) {
        return;
    }

    const auto inter = intersection(player_, obstacle);
    if (!inter.valid()) {
        return;
    }

    if (inter.h < inter.w) {
        if (player_.y < obstacle.y) {
            player_.y = obstacle.y - player_.h;
            posY_ = static_cast<float>(player_.y);
            vy_ = 0.0f;
            onGround_ = true;
        } else {
            player_.y = obstacle.y + obstacle.h;
            posY_ = static_cast<float>(player_.y);
            vy_ = 0.0f;
        }
    } else {
        if (player_.x < obstacle.x) {
            player_.x = obstacle.x - player_.w;
        } else {
            player_.x = obstacle.x + obstacle.w;
        }
        posX_ = static_cast<float>(player_.x);
    }
}

void GameCore::resolveStageCollision() {
    onGround_ = false;
    for (const auto& p : platforms_) {
        resolveRectCollision(p);
    }
    for (const auto& p : invisiblePlatforms_) {
        resolveRectCollision(p);
    }
    for (const auto& mp : movingPlatforms_) {
        resolveRectCollision(mp.rect());
    }
    for (const auto& dp : dragPlatforms_) {
        resolveRectCollision(dp.rect);
    }
}

void GameCore::respawnPlayer(const std::string& reason) {
    ++attemptCount_;
    const Level& l = levels_[currentLevel_];

    int rx = l.startX;
    int ry = l.startY;
    std::string actualReason = reason;

    if (checkpointActive_) {
        ++checkpointDeaths_;
        if (checkpointDeaths_ >= 10) {
            checkpointActive_ = false;
            checkpointDeaths_ = 0;
            actualReason = "Checkpoint lost (10 deaths). Return to start.";
        } else {
            rx = checkpointRespawnX_;
            ry = checkpointRespawnY_;
        }
    }

    posX_ = static_cast<float>(rx);
    posY_ = static_cast<float>(ry);
    vy_ = 0.0f;
    vx_ = 0.0f;
    player_.x = static_cast<int>(posX_);
    player_.y = static_cast<int>(posY_);

    playerHp_ = playerMaxHp_;
    damageInvulnTimer_ = 20;
    respawnPending_ = false;

    showingMessage_ = true;
    messageText_ = actualReason;
    messageTimer_ = 45;
    fireballs_.clear();
}

void GameCore::damagePlayer(const std::string& reason) {
    if (respawnPending_) {
        return;
    }
    if (damageInvulnTimer_ > 0) {
        return;
    }

    playerHp_ = std::max(0, playerHp_ - 1);
    damageInvulnTimer_ = 26;

    if (playerHp_ <= 0) {
        respawnPending_ = true;
        explosionTimer_ = 26;
        explosionX_ = static_cast<float>(player_.x + player_.w / 2);
        explosionY_ = static_cast<float>(player_.y + player_.h / 2);
        pendingDeathReason_ = reason;
        fireballs_.clear();
        showingMessage_ = true;
        messageText_ = "You exploded!";
        messageTimer_ = 35;
        return;
    }

    vy_ = std::min(vy_, -220.0f);
    showingMessage_ = true;
    messageText_ = "HP: " + std::to_string(playerHp_) + "/" + std::to_string(playerMaxHp_);
    messageTimer_ = 25;
}

bool GameCore::allBossesDefeated() const {
    if (bosses_.empty()) {
        return true;
    }
    for (const auto& b : bosses_) {
        if (b.alive) {
            return false;
        }
    }
    return true;
}

bool GameCore::containsAnyCorner(const Triangle& tri, const Rect& r) const {
    const std::array<std::pair<float, float>, 4> corners = {
        std::make_pair(static_cast<float>(r.x), static_cast<float>(r.y)),
        std::make_pair(static_cast<float>(r.x + r.w), static_cast<float>(r.y)),
        std::make_pair(static_cast<float>(r.x), static_cast<float>(r.y + r.h)),
        std::make_pair(static_cast<float>(r.x + r.w), static_cast<float>(r.y + r.h)),
    };

    for (const auto& c : corners) {
        if (pointInTriangle(c.first, c.second, tri)) {
            return true;
        }
    }
    return false;
}

void GameCore::recomputeMouseLevels() {
    mouseLevels_.clear();
    for (int i = 0; i < static_cast<int>(levels_.size()); ++i) {
        if (!levels_[i].dragPlatforms.empty()) {
            mouseLevels_.push_back(i);
        }
    }
}

bool GameCore::startDrag(int logicalX, int logicalY) {
    if (gameState_ != GameState::Playing) {
        return false;
    }

    for (auto& dp : dragPlatforms_) {
        if (dp.rect.contains(logicalX, logicalY)) {
            activeDrag_ = &dp;
            dragOffsetX_ = logicalX - dp.rect.x;
            dragOffsetY_ = logicalY - dp.rect.y;
            return true;
        }
    }

    return false;
}

void GameCore::dragTo(int logicalX, int logicalY) {
    if (activeDrag_ == nullptr) {
        return;
    }

    Rect& r = activeDrag_->rect;
    r.x = logicalX - dragOffsetX_;
    r.y = logicalY - dragOffsetY_;

    if (r.x < 0) {
        r.x = 0;
    }
    if (r.x + r.w > static_cast<int>(levelWidth_)) {
        r.x = static_cast<int>(levelWidth_) - r.w;
    }
    if (r.y < 0) {
        r.y = 0;
    }
    if (r.y + r.h > H) {
        r.y = H - r.h;
    }
}

void GameCore::endDrag() {
    activeDrag_ = nullptr;
}

} // namespace xplat
