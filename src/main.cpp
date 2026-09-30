#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cmath>

using namespace geode::prelude;

// Configuracion de los rangos de tolerancia por color (NaN GD standard)
struct FrameTier {
    const char* label;
    double minF;
    double maxF;
    ccColor3B color;
};

static const std::vector<FrameTier> FRAME_TIERS = {
    { "1 F",    0.0,    1.0,  {255, 55,  55}  }, // Rojo / Frame Perfect
    { "2 F",    1.0001, 2.0,  {255, 140, 30}  }, // Naranja
    { "3 F",    2.0001, 3.0,  {255, 225, 40}  }, // Amarillo
    { "4 F",    3.0001, 4.0,  {175, 255, 45}  }, // Lima
    { "5-6 F",  4.0001, 6.0,  {60,  220, 80}  }, // Verde
    { "7-8 F",  6.0001, 8.0,  {70,  200, 255} }, // Azul claro
    { "9-12 F", 8.0001, 12.0, {60,  130, 255} }, // Azul
};

// Accion o salto precalculado estilo NaN GD
struct FrameAction {
    int frame = 0;
    double frameWindow = 1.0;
};

// Base64 con padding de la imagen suministrada
static const char* TOGGLE_BTN_BASE64 = 
    "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";

// Decodificador de base64 seguro para Cocos2d-x
static std::vector<unsigned char> decodeBase64(const std::string& input) {
    static const std::string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[b64[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// Creador de sprite de boton con fallback de imagen en memoria
static CCSprite* createToggleSprite() {
    auto spr = CCSprite::create("fwc_toggle_btn.png");
    if (!spr) {
        spr = CCSprite::create("user.frame_window_counter/fwc_toggle_btn.png");
    }
    if (spr) return spr;

    auto bytes = decodeBase64(TOGGLE_BTN_BASE64);
    if (!bytes.empty()) {
        auto img = new CCImage();
        if (img->initWithImageData(bytes.data(), bytes.size(), CCImage::kFmtPng)) {
            auto tex = new CCTexture2D();
            if (tex->initWithImage(img)) {
                img->release();
                tex->autorelease();
                return CCSprite::createWithTexture(tex);
            }
        }
        img->release();
    }

    return CCSprite::createWithSpriteFrameName("GJ_eyeBtn_001.png");
}

// Hook de PlayLayer para el HUD del contador y ejecucion de Frame Windows
class $modify(FWCPlayLayer, PlayLayer) {
    struct Fields {
        int m_counts[7] = {0};
        std::vector<CCLabelBMFont*> m_valueLabels;
        CCLabelBMFont* m_modeLabel = nullptr;
        CCNode* m_hudNode = nullptr;

        std::vector<FrameAction> m_actions;
        size_t m_nextActionIdx = 0;
        int m_lastFrame = -1;

        int m_pressFrame = 0;
        bool m_isHolding = false;
        int m_currentPhysicsFrame = 0;
        bool m_hasMacro = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        bool showHud = Mod::get()->getSavedValue<bool>("show-frame-window-hud", true);
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto hud = CCNode::create();
        hud->setID("frame-window-counter-hud"_spr);
        hud->setZOrder(100);
        hud->setVisible(showHud);

        // Posicion superior izquierda
        hud->setPosition({10.f, winSize.height - 12.f});

        // Fondo semitransparente oscuro con espacio para subtitulo
        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setColor({0, 0, 0});
        bg->setOpacity(120);
        bg->setContentSize({96.f, 130.f});
        bg->setAnchorPoint({0.f, 1.f});
        bg->setPosition({-5.f, 5.f});
        hud->addChild(bg);

        // Titulo del HUD
        auto title = CCLabelBMFont::create("FRAME WINDOWS", "goldFont.fnt");
        title->setScale(0.35f);
        title->setAnchorPoint({0.f, 1.f});
        title->setPosition({0.f, 0.f});
        hud->addChild(title);

        // Subtitulo de estado (Macro vs En Vivo)
        auto modeLbl = CCLabelBMFont::create("EN VIVO (Inputs)", "chatFont.fnt");
        modeLbl->setScale(0.31f);
        modeLbl->setAnchorPoint({0.f, 1.f});
        modeLbl->setPosition({0.f, -12.f});
        modeLbl->setColor({180, 210, 255});
        hud->addChild(modeLbl);
        m_fields->m_modeLabel = modeLbl;

        m_fields->m_valueLabels.clear();
        float startY = -26.f;
        float lineGap = 13.5f;

        for (size_t i = 0; i < FRAME_TIERS.size(); ++i) {
            const auto& tier = FRAME_TIERS[i];
            float yPos = startY - (static_cast<float>(i) * lineGap);

            auto nameLabel = CCLabelBMFont::create(tier.label, "chatFont.fnt");
            nameLabel->setScale(0.42f);
            nameLabel->setAnchorPoint({0.f, 0.5f});
            nameLabel->setPosition({0.f, yPos});
            nameLabel->setColor(tier.color);
            hud->addChild(nameLabel);

            auto valLabel = CCLabelBMFont::create("0", "chatFont.fnt");
            valLabel->setScale(0.42f);
            valLabel->setAnchorPoint({1.f, 0.5f});
            valLabel->setPosition({82.f, yPos});
            valLabel->setColor(tier.color);
            hud->addChild(valLabel);

            m_fields->m_valueLabels.push_back(valLabel);
        }

        if (m_uiLayer) {
            m_uiLayer->addChild(hud);
        } else {
            this->addChild(hud);
        }
        m_fields->m_hudNode = hud;

        // Intentar autodetectar macro precalculada del nivel
        this->loadMacroForLevel(level);
        this->updateModeSubtitle();

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        for (size_t i = 0; i < 7; ++i) {
            m_fields->m_counts[i] = 0;
        }
        this->updateValueLabels();

        m_fields->m_currentPhysicsFrame = 0;
        m_fields->m_pressFrame = 0;
        m_fields->m_isHolding = false;
        m_fields->m_lastFrame = -1;
        m_fields->m_nextActionIdx = 0;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        m_fields->m_currentPhysicsFrame++;

        // Si hay macro cargada, procesamos los frame windows predeterminados (estilo NaN GD)
        if (m_fields->m_hasMacro && !m_fields->m_actions.empty()) {
            int currentFrame = static_cast<int>(this->m_gameState.m_levelTime * 240.0);

            if (currentFrame < m_fields->m_lastFrame) {
                // Reinicio de intento
                m_fields->m_lastFrame = currentFrame - 1;
                m_fields->m_nextActionIdx = 0;
                for (int i = 0; i < 7; ++i) m_fields->m_counts[i] = 0;
                this->updateValueLabels();
            }

            if (currentFrame > m_fields->m_lastFrame) {
                bool changed = false;
                while (m_fields->m_nextActionIdx < m_fields->m_actions.size() &&
                       m_fields->m_actions[m_fields->m_nextActionIdx].frame <= currentFrame) {
                    const auto& act = m_fields->m_actions[m_fields->m_nextActionIdx];
                    double fw = act.frameWindow;

                    for (size_t i = 0; i < FRAME_TIERS.size(); ++i) {
                        const auto& tier = FRAME_TIERS[i];
                        if (fw >= tier.minF && fw <= tier.maxF) {
                            m_fields->m_counts[i]++;
                            changed = true;
                            this->spawnMarker(fw, tier.color);
                            break;
                        }
                    }
                    m_fields->m_nextActionIdx++;
                }
                m_fields->m_lastFrame = currentFrame;
                if (changed) {
                    this->updateValueLabels();
                }
            }
        }
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        PlayLayer::handleButton(down, button, isPlayer1);

        if (isPlayer1 && m_player1 && !m_player1->m_isDead) {
            int currentF = m_fields->m_currentPhysicsFrame;

            if (down) {
                m_fields->m_pressFrame = currentF;
                m_fields->m_isHolding = true;
            } else if (m_fields->m_isHolding) {
                m_fields->m_isHolding = false;
                int holdFrames = currentF - m_fields->m_pressFrame;
                if (holdFrames < 1) holdFrames = 1;

                // Si NO hay macro cargada en el nivel, medimos y contamos las pulsaciones en tiempo real
                if (!m_fields->m_hasMacro) {
                    double fw = static_cast<double>(holdFrames);
                    for (size_t i = 0; i < FRAME_TIERS.size(); ++i) {
                        const auto& tier = FRAME_TIERS[i];
                        if ((fw >= tier.minF && fw <= tier.maxF) || (i == 6 && fw > 12.0)) {
                            m_fields->m_counts[i]++;
                            this->spawnMarker(fw > 12.0 ? 12.0 : fw, tier.color);
                            this->updateValueLabels();
                            break;
                        }
                    }
                }
            }
        }
    }

    void spawnMarker(double fw, ccColor3B color) {
        if (!m_player1) return;

        std::string text;
        if (fw <= 1.0) text = "1 F";
        else if (fw <= 2.0) text = "2 F";
        else if (fw <= 3.0) text = "3 F";
        else if (fw <= 4.0) text = "4 F";
        else if (fw <= 6.0) text = "5-6 F";
        else if (fw <= 8.0) text = "7-8 F";
        else text = "9-12 F";

        auto label = CCLabelBMFont::create(text.c_str(), "goldFont.fnt");
        if (!label) return;

        label->setScale(0.40f);
        label->setColor(color);
        label->setPosition(m_player1->getPosition() + ccp(0.f, 26.f));
        label->setZOrder(100);

        auto move = CCMoveBy::create(0.45f, ccp(0.f, 25.f));
        auto fade = CCFadeOut::create(0.45f);
        auto spawn = CCSpawn::create(move, fade, nullptr);
        auto remove = CCRemoveSelf::create();
        label->runAction(CCSequence::create(spawn, remove, nullptr));

        if (m_objectLayer) {
            m_objectLayer->addChild(label);
        } else {
            this->addChild(label);
        }
    }

    void updateValueLabels() {
        for (size_t i = 0; i < FRAME_TIERS.size(); ++i) {
            if (i < m_fields->m_valueLabels.size() && m_fields->m_valueLabels[i]) {
                m_fields->m_valueLabels[i]->setString(std::to_string(m_fields->m_counts[i]).c_str());
            }
        }
    }

    void updateModeSubtitle() {
        if (m_fields->m_modeLabel) {
            if (m_fields->m_hasMacro) {
                m_fields->m_modeLabel->setString(fmt::format("MACRO ({} Saltos)", m_fields->m_actions.size()).c_str());
                m_fields->m_modeLabel->setColor({120, 255, 140});
            } else {
                m_fields->m_modeLabel->setString("EN VIVO (Inputs)");
                m_fields->m_modeLabel->setColor({180, 210, 255});
            }
        }
    }

    bool loadMacroForLevel(GJGameLevel* level) {
        if (!level) return false;
        std::string cleanName = "";
        if (level->m_levelName.c_str()) {
            for (char c : std::string(level->m_levelName.c_str())) {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') cleanName += c;
            }
        }
        int lvlId = static_cast<int>(level->m_levelID);

        auto baseDir = Mod::get()->getSaveDir();
        auto levelsDir = baseDir / "levels";
        std::error_code ec;
        std::filesystem::create_directories(levelsDir, ec);

        std::vector<std::filesystem::path> searchPaths = {
            levelsDir / fmt::format("{}.json", lvlId),
            levelsDir / fmt::format("{}.fwc", lvlId),
            levelsDir / fmt::format("{}.json", cleanName),
            levelsDir / fmt::format("{}.fwc", cleanName),
            baseDir / fmt::format("{}.json", lvlId),
            baseDir / fmt::format("{}.fwc", lvlId),
            baseDir / fmt::format("{}.json", cleanName),
            baseDir / fmt::format("{}.fwc", cleanName)
        };

        for (const auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                if (this->loadMacroFile(path)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool loadMacroFile(const std::filesystem::path& path) {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        std::vector<FrameAction> loadedActions;

        if (ext == ".json") {
            std::ifstream f(path);
            if (!f) return false;
            std::stringstream ss;
            ss << f.rdbuf();
            auto res = matjson::parse(ss.str());
            if (!res.isOk()) return false;
            auto root = res.unwrap();

            if (root.contains("frameWindows") && root["frameWindows"].isArray()) {
                for (auto& item : root["frameWindows"].asArray().unwrap()) {
                    double t = item["timePosition"].asDouble().unwrapOr(0.0);
                    double fw = item["frameWindow"].asDouble().unwrapOr(1.0);
                    int frame = static_cast<int>(std::round(t));
                    loadedActions.push_back({ frame, fw });
                }
            } else if (root.isArray()) {
                for (auto& item : root.asArray().unwrap()) {
                    double t = item["timePosition"].asDouble().unwrapOr(item["frame"].asDouble().unwrapOr(0.0));
                    double fw = item["frameWindow"].asDouble().unwrapOr(item["window"].asDouble().unwrapOr(1.0));
                    int frame = static_cast<int>(std::round(t));
                    loadedActions.push_back({ frame, fw });
                }
            }
        } else if (ext == ".fwc") {
            std::ifstream f(path, std::ios::binary);
            if (!f) return false;
            char magic[4] = {0};
            f.read(magic, 4);
            std::string magicStr(magic, 4);
            if (magicStr == "FWC2" || magicStr == "FWCB") {
                bool isLegacy = (magicStr == "FWCB");
                double fps = 240.0;
                f.read(reinterpret_cast<char*>(&fps), sizeof(double));
                uint32_t count = 0;
                f.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
                for (uint32_t i = 0; i < count; ++i) {
                    int32_t frame = 0;
                    double frameWindow = 1.0;
                    uint8_t flags = 0;
                    int32_t ifCount = 1;
                    f.read(reinterpret_cast<char*>(&frame), sizeof(int32_t));
                    if (isLegacy) {
                        int32_t lw = 1;
                        f.read(reinterpret_cast<char*>(&lw), sizeof(int32_t));
                        frameWindow = static_cast<double>(lw);
                    } else {
                        f.read(reinterpret_cast<char*>(&frameWindow), sizeof(double));
                    }
                    f.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
                    if (!isLegacy) {
                        f.read(reinterpret_cast<char*>(&ifCount), sizeof(int32_t));
                    }
                    loadedActions.push_back({ frame, frameWindow });
                }
            }
        }

        if (loadedActions.empty()) return false;

        std::stable_sort(loadedActions.begin(), loadedActions.end(), [](const FrameAction& a, const FrameAction& b) {
            return a.frame < b.frame;
        });

        m_fields->m_actions = std::move(loadedActions);
        m_fields->m_hasMacro = true;
        m_fields->m_nextActionIdx = 0;
        m_fields->m_lastFrame = -1;
        return true;
    }

    size_t getMacroActionCount() const {
        return m_fields->m_actions.size();
    }
};

// Hook de PauseLayer para el boton Toggle y el boton Cargar Macro
class $modify(FWCPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // 1. Boton Toggle HUD (con icono personalizado)
        auto spr = createToggleSprite();
        spr->setScale(0.70f);

        bool currentVisible = Mod::get()->getSavedValue<bool>("show-frame-window-hud", true);
        if (!currentVisible) {
            spr->setColor({120, 120, 120});
        }

        auto toggleBtn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(FWCPauseLayer::onToggleFrameWindowHUD)
        );
        toggleBtn->setID("toggle-fwc-btn"_spr);

        // 2. Boton Importar Macro (.fwc / .json)
        auto importSpr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        if (!importSpr) importSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if (importSpr) importSpr->setScale(0.65f);

        auto importBtn = CCMenuItemSpriteExtra::create(
            importSpr,
            this,
            menu_selector(FWCPauseLayer::onImportMacro)
        );
        importBtn->setID("import-fwc-btn"_spr);

        // 3. Boton Abrir Carpeta de Niveles
        auto folderSpr = CCSprite::createWithSpriteFrameName("folderBtn_001.png");
        if (!folderSpr) folderSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (folderSpr) folderSpr->setScale(0.65f);

        auto folderBtn = CCMenuItemSpriteExtra::create(
            folderSpr,
            this,
            menu_selector(FWCPauseLayer::onOpenLevelsFolder)
        );
        folderBtn->setID("folder-fwc-btn"_spr);

        auto menu = this->getChildByID("right-button-menu");
        if (!menu) menu = this->getChildByID("bottom-button-menu");
        if (!menu) {
            menu = CCMenu::create();
            menu->setPosition({winSize.width - 35.f, winSize.height - 35.f});
            this->addChild(menu);
        }

        menu->addChild(toggleBtn);
        menu->addChild(importBtn);
        menu->addChild(folderBtn);
        menu->updateLayout();
    }

    void onToggleFrameWindowHUD(CCObject* sender) {
        bool current = Mod::get()->getSavedValue<bool>("show-frame-window-hud", true);
        bool newState = !current;
        Mod::get()->setSavedValue("show-frame-window-hud", newState);

        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
            if (auto spr = typeinfo_cast<CCSprite*>(btn->getNormalImage())) {
                spr->setColor(newState ? ccColor3B{255, 255, 255} : ccColor3B{120, 120, 120});
            }
        }

        if (auto pl = PlayLayer::get()) {
            if (pl->m_uiLayer) {
                if (auto hud = pl->m_uiLayer->getChildByID("frame-window-counter-hud"_spr)) {
                    hud->setVisible(newState);
                }
            }
            if (auto hud = pl->getChildByID("frame-window-counter-hud"_spr)) {
                hud->setVisible(newState);
            }
        }

        Notification::create(
            newState ? "Frame Window HUD: Activado" : "Frame Window HUD: Desactivado",
            NotificationIcon::Info,
            0.75f
        )->show();
    }

    void onImportMacro(CCObject*) {
        file::FilePickOptions options;
        options.filters.push_back({ "Frame Windows (*.json, *.fwc)", { "*.json", "*.fwc" } });
        options.filters.push_back({ "NANDL JSON (*.json)", { "*.json" } });
        options.filters.push_back({ "Frame Window Counter (*.fwc)", { "*.fwc" } });

        geode::async::spawn(
            file::pick(file::PickMode::OpenFile, options),
            [](file::PickResult pickResult) {
                if (!pickResult.isOk()) return;
                auto optPath = pickResult.unwrap();
                if (!optPath.has_value()) return;

                auto path = optPath.value();
                auto pl = PlayLayer::get();
                if (!pl) return;

                auto fwcPl = static_cast<FWCPlayLayer*>(pl);
                if (fwcPl->loadMacroFile(path)) {
                    if (pl->m_level) {
                        int lvlId = static_cast<int>(pl->m_level->m_levelID);
                        auto dest = Mod::get()->getSaveDir() / "levels" / fmt::format("{}{}", lvlId, path.extension().string());
                        std::error_code ec;
                        std::filesystem::copy_file(path, dest, std::filesystem::copy_options::overwrite_existing, ec);
                    }
                    fwcPl->updateModeSubtitle();
                    fwcPl->resetLevel();
                    Notification::create(
                        fmt::format("Macro cargada: {} saltos", fwcPl->getMacroActionCount()),
                        NotificationIcon::Success,
                        2.0f
                    )->show();
                } else {
                    Notification::create(
                        "Error al cargar el archivo de macro",
                        NotificationIcon::Error,
                        2.0f
                    )->show();
                }
            }
        );
    }

    void onOpenLevelsFolder(CCObject*) {
        auto levelsDir = Mod::get()->getSaveDir() / "levels";
        std::error_code ec;
        std::filesystem::create_directories(levelsDir, ec);
        geode::utils::file::openFolder(levelsDir);
        Notification::create("Carpeta de niveles abierta", NotificationIcon::Info, 1.5f)->show();
    }
};
