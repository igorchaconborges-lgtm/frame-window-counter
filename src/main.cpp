#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <vector>
#include <string>

using namespace geode::prelude;

// Configuracion de los rangos de tolerancia por color
struct FrameTier {
    const char* label;
    int minF;
    int maxF;
    ccColor3B color;
};

static const std::vector<FrameTier> FRAME_TIERS = {
    { "1 F",    0,  1, {255, 55,  55}  }, // Rojo / Frame Perfect
    { "2 F",    2,  2, {255, 140, 30}  }, // Naranja
    { "3 F",    3,  3, {255, 225, 40}  }, // Amarillo
    { "4 F",    4,  4, {175, 255, 45}  }, // Lima
    { "5-6 F",  5,  6, {60,  220, 80}  }, // Verde
    { "7-8 F",  7,  8, {70,  200, 255} }, // Azul claro
    { "9-12 F", 9, 12, {60,  130, 255} }, // Azul
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
    // 1. Intentar cargar desde la carpeta resources del mod
    auto spr = CCSprite::create("fwc_toggle_btn.png");
    if (!spr) {
        spr = CCSprite::create("user.frame_window_counter/fwc_toggle_btn.png");
    }
    if (spr) return spr;

    // 2. Fallback de decodificacion directa en memoria usando CCImage y CCTexture2D
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

    // 3. Fallback de respaldo nativo de Geode/GD
    return CCSprite::createWithSpriteFrameName("GJ_eyeBtn_001.png");
}

// Hook de PlayLayer para el HUD del contador
class $modify(FWCPlayLayer, PlayLayer) {
    struct Fields {
        int m_counts[7] = {0};
        int m_lastInputFrame = -100;
        int m_currentPhysicsFrame = 0;
        CCNode* m_hudNode = nullptr;
        std::vector<CCLabelBMFont*> m_valueLabels;
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

        // Fondo semitransparente oscuro
        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setColor({0, 0, 0});
        bg->setOpacity(115);
        bg->setContentSize({95.f, 116.f});
        bg->setAnchorPoint({0.f, 1.f});
        bg->setPosition({-5.f, 5.f});
        hud->addChild(bg);

        // Titulo del HUD
        auto title = CCLabelBMFont::create("FRAME WINDOWS", "goldFont.fnt");
        title->setScale(0.35f);
        title->setAnchorPoint({0.f, 1.f});
        title->setPosition({0.f, 0.f});
        hud->addChild(title);

        m_fields->m_valueLabels.clear();
        float startY = -16.f;
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

        // Montar directamente sobre m_uiLayer para garantizar renderizado HUD
        if (m_uiLayer) {
            m_uiLayer->addChild(hud);
        } else {
            this->addChild(hud);
        }
        m_fields->m_hudNode = hud;

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        // Reinicio de todos los contadores
        for (size_t i = 0; i < 7; ++i) {
            m_fields->m_counts[i] = 0;
            if (i < m_fields->m_valueLabels.size() && m_fields->m_valueLabels[i]) {
                m_fields->m_valueLabels[i]->setString("0");
            }
        }
        m_fields->m_currentPhysicsFrame = 0;
        m_fields->m_lastInputFrame = -100;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        m_fields->m_currentPhysicsFrame++;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        PlayLayer::handleButton(down, button, isPlayer1);

        if (down && isPlayer1 && !m_isDead) {
            int currentF = m_fields->m_currentPhysicsFrame;
            int delta = currentF - m_fields->m_lastInputFrame;
            m_fields->m_lastInputFrame = currentF;

            // Clasificar en el rango correspondiente
            for (size_t i = 0; i < FRAME_TIERS.size(); ++i) {
                const auto& tier = FRAME_TIERS[i];
                if (delta >= tier.minF && delta <= tier.maxF) {
                    m_fields->m_counts[i]++;
                    if (i < m_fields->m_valueLabels.size() && m_fields->m_valueLabels[i]) {
                        m_fields->m_valueLabels[i]->setString(std::to_string(m_fields->m_counts[i]).c_str());
                    }
                    break;
                }
            }
        }
    }
};

// Hook de PauseLayer para el boton Toggle
class $modify(FWCPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Crear boton con el sprite personalizado
        auto spr = createToggleSprite();
        spr->setScale(0.70f);

        bool currentVisible = Mod::get()->getSavedValue<bool>("show-frame-window-hud", true);
        if (!currentVisible) {
            spr->setColor({120, 120, 120});
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(FWCPauseLayer::onToggleFrameWindowHUD)
        );
        btn->setID("toggle-fwc-btn"_spr);

        // Ubicar en el menu lateral o de botones de pausa
        auto menu = this->getChildByID("right-button-menu");
        if (!menu) {
            menu = this->getChildByID("bottom-button-menu");
        }
        if (!menu) {
            menu = CCMenu::create();
            menu->setPosition({winSize.width - 35.f, winSize.height - 35.f});
            this->addChild(menu);
        }

        menu->addChild(btn);
        menu->updateLayout();
    }

    void onToggleFrameWindowHUD(CCObject* sender) {
        bool current = Mod::get()->getSavedValue<bool>("show-frame-window-hud", true);
        bool newState = !current;
        Mod::get()->setSavedValue("show-frame-window-hud", newState);

        // Feedback visual en el boton
        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
            if (auto spr = typeinfo_cast<CCSprite*>(btn->getNormalImage())) {
                spr->setColor(newState ? ccColor3B{255, 255, 255} : ccColor3B{120, 120, 120});
            }
        }

        // Alternar visibilidad en PlayLayer de forma inmediata
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
};
