#pragma once

#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/cocos/touch_dispatcher/CCTouchDelegateProtocol.h"

class DragButton : public cocos2d::CCLayer {
	protected:
		std::function<void()> m_onPress;
		cocos2d::CCPoint m_diff = cocos2d::CCPoint{0, 0};

		bool init(cocos2d::CCNode* node, std::function<void()>);
		bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
		void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
		void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
		void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

	        void registerWithTouchDispatcher() override;

		void update(float dt) override;
	public:
		void setCallback(std::function<void()> onPress);
		std::function<void()> getCallback();

		static DragButton* create(cocos2d::CCNode* node, std::function<void()> onPress);
        static DragButton* get();
};
