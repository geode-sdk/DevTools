#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class DragButton : public cocos2d::CCMenu {
protected:
    static DragButton *m_instance;

    bool m_handleTouch = true;
    bool m_render = true;
    bool m_moving = false;

    cocos2d::CCPoint m_startPos;
    cocos2d::CCPoint m_diff;
    cocos2d::CCSprite *m_sprite;
    
    float m_scale = 1.0f;
    float m_multiplier = 0.8f;

    bool init() override;
    void update(float delta) override;
    bool ccTouchBegan(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void ccTouchEnded(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void ccTouchMoved(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void ccTouchCancelled(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) override;
    void registerWithTouchDispatcher() override;
public:
    static DragButton *get();
    void activate();
    bool isRendered();
    void setRendered(bool render);
    bool isHandlingTouch();
    void setHandlingTouch(bool handle);
};