#include "../DevTools.hpp"
#include "DragButton.hpp"

DragButton *DragButton::m_instance = nullptr;

bool DragButton::init() {
    if (!CCMenu::init())
        return false;

    auto winSize = CCDirector::get()->getWinSize();
    
    m_sprite = CircleButtonSprite::createWithSprite("devtools.png"_spr, 1, 
        CircleBaseColor::Green, CircleBaseSize::MediumAlt);
    m_sprite->setScale(.8f);
    m_sprite->setID("sprite");
    addChild(m_sprite);
    setContentSize(m_sprite->getScaledContentSize());
    m_sprite->setPosition(getContentSize() / 2);

    CCScene::get()->addChild(this);
    SceneManager::get()->keepAcrossScenes(this);
    scheduleUpdate();

    setZOrder(70000);
    
    auto settings = DevTools::get()->getSettings();
    setOpacity(settings.buttonOpacity);
    setScale(settings.buttonScale);
    settings.buttonPos.x = std::clamp(settings.buttonPos.x, -getContentWidth() / 2, winSize.width - getContentWidth() / 2);
    settings.buttonPos.y = std::clamp(settings.buttonPos.y, -getContentHeight() / 2, winSize.height - getContentHeight() / 2);
    setPosition(settings.buttonPos);

    setID("drag-button"_spr);

    return true;
};

DragButton *DragButton::get() {
    if (m_instance)
        return m_instance;
    m_instance = new DragButton();
    if (m_instance && m_instance->init()) {
        m_instance->autorelease();
        return m_instance;
    } else {
        delete m_instance;
        return nullptr;
    }
}

void DragButton::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -512, true);
}

bool DragButton::ccTouchBegan(CCTouch *touch, CCEvent *evt) {
    if (!m_handleTouch || !m_bVisible)
        return false;
    if (getScaledContentSize().width / 2 <
        ccpDistance(m_sprite->getPosition(), convertToNodeSpace(touch->getLocation()))) {
        return false;
    }

    m_diff = getPosition() - touch->getLocation();
    m_startPos = touch->getLocation();

    m_moving = false;

    m_sprite->stopAllActions();

    // For some reason I could not get a recreation of CCEaseSineOut working on ios.
    #ifdef GEODE_IS_IOS
    m_sprite->runAction(CCEaseOut::create(CCScaleTo::create(0.3f, .8 * m_scale * m_multiplier), 1.6f));
    #else
    m_sprite->runAction(CCEaseSineOut::create(CCScaleTo::create(0.3f, .8 * m_scale * m_multiplier)));
    #endif
    return true;
}

void DragButton::ccTouchCancelled(CCTouch *touch, CCEvent *event) {
    ccTouchEnded(touch, event);
}

void DragButton::ccTouchEnded(CCTouch *touch, CCEvent *evt) {
    m_sprite->stopAllActions();

    // For some reason I could not get a recreation of CCEaseSineOut working on ios.
    #ifdef GEODE_IS_IOS
    m_sprite->runAction(CCEaseOut::create(CCScaleTo::create(0.3f, .8 * m_scale), 1.6f));
    #else
    m_sprite->runAction(CCEaseSineOut::create(CCScaleTo::create(0.3f, .8 * m_scale)));
    #endif
    if (m_moving) {
      DevTools::get()->getSettings().buttonPos = getPosition();
      return;
    }
    activate();
}

void DragButton::ccTouchMoved(CCTouch *touch, CCEvent *evt) {
    if (!m_moving)
        if (ccpDistance(m_startPos, touch->getLocation()) > 3)
        m_moving = true;
    if (m_moving) {
        auto pos = touch->getLocation() + m_diff;
        pos.x = std::clamp(pos.x, -getContentWidth() / 2, CCDirector::get()->getWinSize().width - getContentWidth() / 2);
        pos.y = std::clamp(pos.y, -getContentHeight() / 2, CCDirector::get()->getWinSize().height - getContentHeight() / 2);
        setPosition(pos);
    }
}

void DragButton::update(float delta) {
    static auto devtools = DevTools::get();
    bool shouldRender = true;
    if (auto pl = PlayLayer::get(); pl && !pl->m_isPaused) {
        shouldRender = devtools->getSettings().buttonInGameplay;
    } else if(auto el = LevelEditorLayer::get(); el && !el->getChildByType<EditorPauseLayer *>(0)) {
        if (devtools->getSettings().buttonInEditor) {
            shouldRender = el->m_playbackMode != PlaybackMode::Playing || devtools->getSettings().buttonInGameplay;
        } else {
            shouldRender = false;
        }
    }
    setVisible(shouldRender && m_render);
}

bool DragButton::isRendered() {
    return m_render;
}

void DragButton::setRendered(bool render) {
    m_render = render;
}

bool DragButton::isHandlingTouch() {
    return m_render && m_handleTouch;
}

void DragButton::setHandlingTouch(bool handle) { m_handleTouch = handle; }

void DragButton::activate() {
  DevTools::get()->toggle();
}

// Only make it show if on mobile
#ifdef GEODE_IS_MOBILE
#include <Geode/modify/CCScene.hpp>

class $modify(CCScene) {
    int getHighestChildZ() {
        int btnZ;
        auto btn = DragButton::get();
        if (btn) {
            btnZ = btn->getZOrder();
            btn->setZOrder(-1);
        }
        auto highest = CCScene::getHighestChildZ();
        if (btn) {
            btn->setZOrder(btnZ);
        }
        return highest;
    }
};

#include <Geode/modify/MenuLayer.hpp>

class $modify(MenuLayer) {
  bool init() {
    if (!MenuLayer::init()) return false;

    DragButton::get();
    return true;
  }
};
#endif
