/*
 * Copyright (c) 2015
 * MURPHY'S TOAST STUDIOS
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

#include "AppDelegate.h"
#include "UIMainMenu.h"
#include "GameData.h"
#include "LocalizedString.h"
#include <string>

#include <audio/include/SimpleAudioEngine.h>

USING_NS_CC;

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview) {
        glview = GLViewImpl::createWithFullScreen("simulplay");
        director->setOpenGLView(glview);
        //glview->setDesignResolutionSize(2048, 1536, ResolutionPolicy::NO_BORDER);
    }

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    // multi-device resolution assets
    auto screenSize = glview->getFrameSize();
    auto fileUtils = FileUtils::getInstance();
    std::vector<std::string> resDirOrders;

    //resDirOrders.push_back("ipadhd");
    glview->setDesignResolutionSize(screenSize.width, screenSize.height, ResolutionPolicy::NO_BORDER);

    /* if (2048 == screenSize.width or 2048 == screenSize.height) {
        resDirOrders.push_back("ipadhd");
        resDirOrders.push_back("ipad");
        resDirOrders.push_back("iphonehd5");
        resDirOrders.push_back("iphonehd");
        resDirOrders.push_back("iphone");

        glview->setDesignResolutionSize(2048, 1536, ResolutionPolicy::NO_BORDER);

    } else if (1024 == screenSize.width or 1024 == screenSize.height) {
        resDirOrders.push_back("ipadhd");

        resDirOrders.push_back("ipad");
        resDirOrders.push_back("iphonehd5");
        resDirOrders.push_back("iphonehd");
        resDirOrders.push_back("iphone");

        glview->setDesignResolutionSize(1024, 768, ResolutionPolicy::NO_BORDER);

    } else if (1136 == screenSize.width or 1136 == screenSize.height) {
        resDirOrders.push_back("ipadhd");

        resDirOrders.push_back("iphonehd5");
        resDirOrders.push_back("iphonehd");
        resDirOrders.push_back("iphone");

        glview->setDesignResolutionSize(1136, 640, ResolutionPolicy::FIXED_HEIGHT);

    } else if (960 == screenSize.width or 960 == screenSize.height) {
        resDirOrders.push_back("ipadhd");

        resDirOrders.push_back("iphonehd");
        resDirOrders.push_back("iphone");

        glview->setDesignResolutionSize(960, 640, ResolutionPolicy::NO_BORDER);

    } else {
        if (screenSize.width > 1080) {
            resDirOrders.push_back("ipadhd");

            resDirOrders.push_back("iphonehd");
            resDirOrders.push_back("iphone");

            glview->setDesignResolutionSize(960, 640, ResolutionPolicy::NO_BORDER);

        } else {
            resDirOrders.push_back("ipadhd");

            resDirOrders.push_back("iphone");

            glview->setDesignResolutionSize(480, 320, ResolutionPolicy::NO_BORDER);
        }
    }
*/

    fileUtils->setSearchPaths(resDirOrders);

    //Primer cop language = '', agafar del dispositiu per defecte
    string lang = UserDefault::getInstance()->getStringForKey("language");
    if (lang.size() == 0) {
        GameData::getInstance()->setLanguage(LocalizedString::getSystemLang());
        GameData::getInstance()->setMusic(true);
        GameData::getInstance()->setSFX(true);
        UserDefault::getInstance()->setBoolForKey("firsttimeplaying", true);
    }
    else {
        GameData::getInstance()->setLanguage(lang);
        GameData::getInstance()->setMusic(UserDefault::getInstance()->getBoolForKey("music"));
        GameData::getInstance()->setSFX(UserDefault::getInstance()->getBoolForKey("sfx"));
        UserDefault::getInstance()->setBoolForKey("firsttimeplaying", false);
    }

    //RA CONVERSION - PRECALCS
    Size visibleSize = Director::getInstance()->getVisibleSize();
    GameData::getInstance()->setRaConversion((2048.0 / 1536.0) / (visibleSize.width / visibleSize.height));
    GameData::getInstance()->setRaWConversion(visibleSize.width / 2048.0);
    GameData::getInstance()->setRaHConversion(visibleSize.height / 1536.0);
    /*GameData::getInstance()->setHeightProportionalResolution((Director::getInstance()->getVisibleSize().width / 480.0) * 320.0);
    GameData::getInstance()->setRowDrawAgentPrecalc(Director::getInstance()->getVisibleSize().width / 480.0);
    GameData::getInstance()->setColumnOffsetDrawAgentPrecalc((Director::getInstance()->getVisibleSize().height -
                                                              GameData::getInstance()->getHeightProportionalResolution()) / 2.0);
    GameData::getInstance()->setColumnDrawAgentPrecalc(GameData::getInstance()->getHeightProportionalResolution() / 320.0);*/
    GameData::getInstance()->setHeightProportionalResolution((2048.0 / 480.0) * 320.0);
    GameData::getInstance()->setRowDrawAgentPrecalc(2048.0 / 480.0);
    GameData::getInstance()->setColumnOffsetDrawAgentPrecalc((1536.0 - 1365.0) / 2.0);
    GameData::getInstance()->setColumnDrawAgentPrecalc(1365.0 / 320.0);

    //LOAD LEVELS
    vector<int> levelsCompleted;
    for (int i = 0; i < UserDefault::getInstance()->getIntegerForKey("maxLevel"); i++) {
        levelsCompleted.push_back(UserDefault::getInstance()->getIntegerForKey(to_string(i).c_str()));
    }
    GameData::getInstance()->setLevelsCompleted(levelsCompleted);

    CCLOG("visible size width: %i", int(Director::getInstance()->getVisibleSize().width));

    // create a scene. it's an autorelease object
    auto scene = UIMainMenu::createScene();
    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone
// call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();

    UserDefault::getInstance()->setStringForKey("language", GameData::getInstance()->getLanguage());
    UserDefault::getInstance()->setBoolForKey("music", GameData::getInstance()->getMusic());
    UserDefault::getInstance()->setBoolForKey("sfx", GameData::getInstance()->getSFX());
    UserDefault::getInstance()->flush();

    // if you use SimpleAudioEngine, it must be pause
    CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
