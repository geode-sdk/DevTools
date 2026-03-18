#include "DragButton.hpp"
#include "../DevTools.hpp"

#include <Geode/Geode.hpp>
#include <utility>

using namespace geode::prelude;

bool DragButton::init(CCNode* node, Function<void()> onPress) {
    if (!CCLayer::init()) return false;
    this->setTouchEnabled(true);
    this->setAnchorPoint({.5f, .5f});
    this->ignoreAnchorPointForPosition(false);
    this->scheduleUpdate();
    if (node) {
        this->setContentSize(node->getScaledContentSize());
        this->addChildAtPosition(node, Anchor::Center, CCPoint{0, 0});
    }
    this->m_onPress = std::move(onPress);
    return true;
}

bool DragButton::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!this->m_bVisible) return false;
    CCPoint local = convertToNodeSpace(touch->getLocation());
    if (this->getContentWidth() > local.x && local.x > 0 && this->getContentHeight() > local.y && local.y > 0) {
        this->stopActionByTag(123);
        auto action = CCEaseSineOut::create(CCScaleTo::create(.3f, .8f));
        action->setTag(123);
        this->runAction(action);

        m_diff = this->getParent()->convertToNodeSpace(touch->getLocation()) - this->getPosition();
        return true;
    }
    return false;
}

void DragButton::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    auto pos = this->getParent()->convertToNodeSpace(touch->getLocation()) - m_diff;
    this->setPosition(pos);
}

void DragButton::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    this->stopActionByTag(123);
    auto action = CCEaseSineOut::create(CCScaleTo::create(.3f, 1.f));
    action->setTag(123);
    this->runAction(action);

    if ((touch->getLocation() - touch->getStartLocation()).getLength() > 3.f) return;
    if (this->m_onPress) this->m_onPress();
}

void DragButton::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    this->ccTouchEnded(touch, event);
}

void DragButton::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -512, true);
}

DragButton* DragButton::create(CCNode* node, geode::Function<void ()> onPress) {
    auto ret = new DragButton;
    if (ret->init(node, std::move(onPress))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void DragButton::update(float dt) {
    const Settings& settings = DevTools::get()->getSettings();
    if (PlayLayer::get() && !CCScene::get()->getChildByType<PauseLayer>(0) && !settings.buttonInGame) {
        setVisible(false);
    } else if (auto lel = LevelEditorLayer::get(); lel && ((lel->m_playbackMode == PlaybackMode::Playing && !settings.buttonInGame) || !settings.buttonInEditor)) {
        setVisible(false);
    } else {
        setVisible(true);
    }
}

void DragButton::setPosition(cocos2d::CCPoint const& pos_) {
    auto winSize = CCDirector::get()->getWinSize();
    float pad = 5.f;
    auto pos = pos_;
    pos.x = std::clamp(pos.x, pad, winSize.width - pad);
    pos.y = std::clamp(pos.y, pad, winSize.height - pad);
    DevTools::get()->setBallPosition(pos);
    CCNode::setPosition(pos);
}