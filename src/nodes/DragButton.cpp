#include "DragButton.hpp"
#include "../DevTools.hpp"

using namespace geode::prelude;

bool DragButton::init(CCNode* node, std::function<void()> onPress) {
	if (!CCLayer::init()) return false;
    	this->setTouchEnabled(true);
	this->setAnchorPoint({.5f, .5f});
    	this->ignoreAnchorPointForPosition(false);
	this->scheduleUpdate();
	if (node) {
		this->setContentSize(node->getScaledContentSize());
		this->addChildAtPosition(node, Anchor::Center, CCPoint{0, 0});
	}
	this->m_onPress = onPress;
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
    auto winSize = CCDirector::get()->getWinSize();
    pos.x = std::clamp(pos.x, 0.f, winSize.width);
    pos.y = std::clamp(pos.y, 0.f, winSize.height);
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

void DragButton::setCallback(std::function<void()> onPress) {
	this->m_onPress = onPress;
}

std::function<void()> DragButton::getCallback() {
	return this->m_onPress;
}

void DragButton::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -512, true);
}

DragButton* DragButton::create(CCNode* node, std::function<void ()> onPress) {
	auto ret = new DragButton; 
	if (ret->init(node, onPress)) {
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

DragButton* DragButton::get() {
    static DragButton* instance;
    if (!instance) {
        auto spr = CircleButtonSprite::createWithSprite("devtools.png"_spr, 1, CircleBaseColor::Green, CircleBaseSize::MediumAlt);
        spr->setScale(.8f);
        instance = DragButton::create(spr, [](){
            DevTools::get()->toggle();
        });
        instance->setPosition(DevTools::get()->getSettings().buttonPos);
        instance->setZOrder(10000);
        instance->setID("devtools-button"_spr);
        CCScene::get()->addChild(instance);
        SceneManager::get()->keepAcrossScenes(instance);
    }
    return instance;
}
