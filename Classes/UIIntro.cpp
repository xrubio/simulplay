/*
 * Copyright (c) 2015
 * MURPHY'S TOAST GAMES
 *
 * This file is part of Evolving Planet.
 * Evolving Planet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Evolving Planet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Evolving Planet.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//  UIIntro.cpp
//  simulplay
//
//  Created by Guillem Laborda on 3/12/15.
//
//

#include "UIIntro.h"
#include "UIMainMenu.h"
#include "GameData.h"
#include <extensions/cocos-ext.h>
#include <ui/CocosGUI.h>
#include <audio/include/SimpleAudioEngine.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#include <ui/UIVideoPlayer.h>
using namespace cocos2d::ui;
using namespace cocos2d::experimental;
using namespace cocos2d::experimental::ui;

#endif

Scene* UIIntro::createScene()
{
    auto scene = Scene::create();
    auto layer = UIIntro::create();
    scene->addChild(layer);
    return scene;
}

bool UIIntro::init()
{
    if (!Layer::init()) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    /*auto background = Sprite::create("gui/MainMenuBackground.png");
    background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    background->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    background->setOpacity(0);
    this->addChild(background, 2);*/
    
    this->setColor(Color3B(211, 210, 210));
  
    /*if (GameData::getInstance()->getMusic() == true and CocosDenshion::SimpleAudioEngine::getInstance()->isBackgroundMusicPlaying() == false) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/main.mp3", true);
    }*/

// video player only for ios/android    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

    VideoPlayer *p = VideoPlayer::create();
    p->setFileName("audio/logo_02.mp4");
    p->setName("video");
    p->setContentSize(Size(visibleSize.width, visibleSize.height + (3 * visibleSize.height / 10)));
    p->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    //p->setKeepAspectRatioEnabled(true);
    p->play();
    this->addChild(p, 1);

// at least show the logo    
#else
    auto logo = Sprite::create("misc/murphy.png");
    logo->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));   
    logo->setName("logo");
    logo->setOpacity(0);
    logo->runAction(Sequence::create(FadeIn::create(1.5f), FadeOut::create(1.0f), nullptr));
    this->addChild(logo);
#endif
    
    _listener = EventListenerTouchOneByOne::create();
    _listener->setSwallowTouches(true);
    _listener->onTouchBegan = CC_CALLBACK_2(UIIntro::onTouchesBegan, this);
    _eventDispatcher->addEventListenerWithFixedPriority(_listener, -1);

    this->scheduleUpdate();
    
    return true;
}

bool UIIntro::onTouchesBegan(Touch* touch, Event* event)
{
    
// video player only for ios/android    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

    if (((VideoPlayer*)this->getChildByName("video")) != nullptr)
    {
        CCLOG("TAP");
        return true;
    }
# endif

    return true;
}

void UIIntro::update(float delta)
{
    
// video player only for ios/android    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if (((VideoPlayer*)this->getChildByName("video")) != nullptr and((VideoPlayer*)this->getChildByName("video"))->isPlaying() == false)
    {
        //this->removeChild(((VideoPlayer*)this->getChildByName("video")));
        _eventDispatcher->removeEventListener(_listener);
        auto scene = UIMainMenu::createScene();
        Director::getInstance()->replaceScene(scene);
        
    }
# else
    Node * logo = this->getChildByName("logo");
    if(logo->getNumberOfRunningActions()==0)
    {
        _eventDispatcher->removeEventListener(_listener);
        auto scene = UIMainMenu::createScene();
        Director::getInstance()->replaceScene(scene);
    }
#endif
}
