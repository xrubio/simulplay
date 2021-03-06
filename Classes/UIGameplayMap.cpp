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
//  UIGameplayMap.cpp
//  simulplay
//
//  Created by Guillem Laborda on 13/08/14.
//
//

#include "UIGameplayMap.h"
#include "UIGoals.h"
#include "GameData.h"
#include "LocalizedString.h"
#include "UIGlobalPower.h"
#include "UIAreaPower.h"
#include "ResourcesGoal.h"
#include "DispersalGoal.h"
#include "Timing.h"
#include "LevelLoader.h"
#include "Message.h"
#include "Tutorial.h"
#include "UITransitionScene.h"
#include <audio/include/SimpleAudioEngine.h>

Scene* UIGameplayMap::createScene()
{
    auto scene = Scene::create();
    auto layer = UIGameplayMap::create();
    scene->addChild(layer);
    return scene;
}

UIGameplayMap::~UIGameplayMap()
{
    if(_tutorial)
    {
        delete _tutorial;
    }
}

bool UIGameplayMap::init()
{
    if (!Layer::init()) {
        return false;
    }

    setName("gameplayMap");
    _tutorial = nullptr;
    Director::getInstance()->setAnimationInterval(1.0 / 60);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Size contentSize;
    contentSize.width = GameData::getInstance()->getResourcesWidth();
    contentSize.height = GameData::getInstance()->getResourcesHeight();

    string map = "maps/" + GameLevel::getInstance()->getMapFilename();
    //fer DEFINES
    string ext = ".png";
    string background = "Background";
    string hotSpotsBase = "HotSpotsBase";
    string resources = "Resources";
    string forest = "Forest";

    //Set background gameplay map and all its functionalities
    gameplayMap = Sprite::create(map + background + ".jpg");
    gameplayMap->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    gameplayMap->setName("gameplayMap");
    this->addChild(gameplayMap, 0);
  
    //HOTSPOT
    gameplayMapHotSpot.initWithImageFile("mapsBase/"+GameLevel::getInstance()->getMapFilename()+hotSpotsBase+ext);
    int x = 3;
    if (gameplayMapHotSpot.hasAlpha()) {
        x = 4;
    }
    dataGameplayMapHotSpot = gameplayMapHotSpot.getData();

    //FINGER SPOT
    fingerSpot = MenuItemImage::create("gui/FingerSpot.png", "gui/FingerSpot.png", CC_CALLBACK_1(UIGameplayMap::removeFingerSpot, this));
    fingerSpot->setVisible(false);
    fingerSpot->setAnchorPoint(Vec2(0.5, 0));
    Menu* mFinger = Menu::createWithItem(fingerSpot);
    mFinger->setPosition(0, 0);
    mFinger->setName("finger");
    gameplayMap->addChild(mFinger, 3);

    //FRAMES
    auto topFrame = Sprite::create("gui/FrameTop.png");
    topFrame->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    topFrame->setPosition(visibleSize.width / 2, visibleSize.height - (topFrame->getBoundingBox().size.height / 2));
    topFrame->setName("topFrame");
    this->addChild(topFrame, 1);
    auto bottomFrame = Sprite::create("gui/FrameBottom.png");
    bottomFrame->setName("bottomFrame");
    bottomFrame->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    bottomFrame->setPosition(visibleSize.width / 2, bottomFrame->getBoundingBox().size.height / 2);
    this->addChild(bottomFrame, 1);

    string space = " ";
    string lvl = LocalizedString::create("LEVEL") + space + to_string(GameLevel::getInstance()->getNumLevel());
    auto levelLabel = Label::createWithTTF(lvl, "fonts/BebasNeue.otf", 90 * GameData::getInstance()->getRaConversion());
    levelLabel->cocos2d::Node::setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    levelLabel->setColor(Color3B(139, 146, 154));
    levelLabel->setAnchorPoint(Vec2(0, 0.5));
    levelLabel->setPosition(Vec2(12 * visibleSize.width / 204, 148 * visibleSize.height / 155));
    this->addChild(levelLabel, 5);
    
    if (GameData::getInstance()->getLevelsFailedForHint().at(GameLevel::getInstance()->getNumLevel()) > 2)
    {
        auto hintButton = MenuItemToggle::createWithCallback(CC_CALLBACK_1(UIGameplayMap::menuHintCallback, this), MenuItemImage::create("gui/HintButton.png", "gui/HintButton.png"), MenuItemImage::create("gui/HintButtonPressed.png", "gui/HintButtonPressed.png"), NULL);
        hintButton->setPosition(Vec2(0.25f*visibleSize.width, 0.94f*visibleSize.height));
        hintButton->setName("hintButton");
        auto menuHint = Menu::create(hintButton, NULL);
        menuHint->setName("menuHint");
        menuHint->setPosition(Vec2::ZERO);
        this->addChild(menuHint, 5);
        
        auto hintBackground = Sprite::create("gui/EmptyBackground.png");
        hintBackground->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.3f));
        hintBackground->setName("hintBackground");
        hintBackground->setVisible(false);
        this->addChild(hintBackground, 5);

        std::stringstream str;
        str << "LEVEL" << GameLevel::getInstance()->getNumLevel() << "HINT";
        auto hintLabel = Label::createWithTTF(LocalizedString::create(str.str().c_str(), "hints"), "fonts/arial.ttf", 40 * GameData::getInstance()->getRaConversion());
        hintLabel->cocos2d::Node::setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
        hintLabel->setColor(Color3B(230, 230, 230));
        hintLabel->setName("hintLabel");
        hintLabel->setMaxLineWidth(0.4f*visibleSize.width);
        hintLabel->setVisible(false);
        hintLabel->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.3f));
        
        float ratioWidth = 1.2f*hintLabel->getBoundingBox().size.width/hintBackground->getContentSize().width;
        float ratioHeight= 1.2f*hintLabel->getBoundingBox().size.height/hintBackground->getContentSize().height;

        hintBackground->setScale(ratioWidth, ratioHeight);
        this->addChild(hintLabel, 5);
    }
    
    auto currentGoalLabel = Label::createWithTTF(LocalizedString::create("CURRENT_GOAL")+":", "fonts/BebasNeue.otf", 45 * GameData::getInstance()->getRaConversion());
    currentGoalLabel->cocos2d::Node::setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    currentGoalLabel->setColor(Color3B(139, 146, 154));
    currentGoalLabel->setAnchorPoint(Vec2(0, 0.5));
    currentGoalLabel->setPosition(Vec2(12 * visibleSize.width / 204, 140 * visibleSize.height / 155));
    this->addChild(currentGoalLabel, 5);
    
    Director::getInstance()->getTextureCache()->addImage("gui/goals/ResourcesMineralGoal.png");
    Director::getInstance()->getTextureCache()->addImage("gui/goals/ResourcesWoodGoal.png");
    Director::getInstance()->getTextureCache()->addImage("gui/goals/DispersalGoal.png");
    Director::getInstance()->getTextureCache()->addImage("gui/goals/PopulationGoal.png");

    auto currentGoalImg = MenuItemImage::create(getGoalIcon(GameLevel::getInstance()->getGoals().at(0)), getGoalIcon(GameLevel::getInstance()->getGoals().at(0)));
    currentGoalImg->setScale(GameData::getInstance()->getRaWConversion()*0.6f, GameData::getInstance()->getRaHConversion()*0.6f);
    currentGoalImg->setPosition(Vec2(0.2f*visibleSize.width, 0.91f * visibleSize.height));
    currentGoalImg->setName("currentGoalImg");
    Menu* goalMenu = Menu::create(currentGoalImg, NULL);
    goalMenu->setName("currentGoalMenu");
    goalMenu->setPosition(Vec2(0, 0));
    
    auto goalsCompletedLabel = Label::createWithTTF("", "fonts/BebasNeue.otf", 100 * GameData::getInstance()->getRaConversion());
    goalsCompletedLabel->setPosition(Vec2(visibleSize.width / 2, 2 * visibleSize.height / 3));
    goalsCompletedLabel->setTextColor(Color4B(216, 229, 235, 200));
    goalsCompletedLabel->setOpacity(0);
    goalsCompletedLabel->setName("goalsCompletedLabel");
    this->addChild(goalsCompletedLabel, 5);
    
    auto goalCompletedSprite = Sprite::create("gui/goals/CompletedGoal.png");
    auto currentGoalProgress = ProgressTimer::create(goalCompletedSprite);
    currentGoalProgress->setPosition(Vec2(currentGoalImg->getContentSize().width / 2, currentGoalImg->getContentSize().height / 2));
    currentGoalProgress->setType(ProgressTimer::Type::BAR);
    currentGoalProgress->setMidpoint(Vec2(0, 0));
    currentGoalProgress->setBarChangeRate(Vec2(0, 1));
    currentGoalProgress->setName("currentGoalProgress");
    currentGoalProgress->setPercentage(0.0);
    currentGoalImg->addChild(currentGoalProgress, 5);
    this->addChild(goalMenu, 2);

    //QUIT / RETRY
    Vector<MenuItem*> quitRetryVec;
    MenuItem* quitButton = MenuItemImage::create("gui/Quit.png", "gui/QuitPressed.png", CC_CALLBACK_1(UIGameplayMap::quitCallback, this));
    quitButton->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    quitButton->setPosition(Vec2(quitButton->getBoundingBox().size.width / 2,
        visibleSize.height - (quitButton->getBoundingBox().size.height / 2)));
    quitButton->setName("quitButton");
    quitRetryVec.pushBack(quitButton);

    MenuItem* retryButton = MenuItemImage::create("gui/Repeat.png", "gui/RepeatPressed.png", CC_CALLBACK_1(UIGameplayMap::retryCallback, this));
    retryButton->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    retryButton->setPosition(Vec2(retryButton->getBoundingBox().size.width / 2,
        quitButton->getPositionY() - quitButton->getBoundingBox().size.height));
    retryButton->setName("retryButton");
    quitRetryVec.pushBack(retryButton);

    Menu* quitRetryMenu = Menu::createWithArray(quitRetryVec);
    quitRetryMenu->setPosition(0, 0);
    quitRetryMenu->setName("quitRetryMenu");
    this->addChild(quitRetryMenu, 10);

    //RESOURCES MAP (IF ANY)
    if (GameLevel::getInstance()->getNumLevel() > 10 and GameLevel::getInstance()->getNumLevel() < 20)
    {
        resourcesMap = true;
    }
    if (resourcesMap) {

        Image im;
        im.initWithImageFile(map + forest + ext);
        //4 = alpha
        unsigned char* data = im.getData();
        
        _exploitedMapTextureData.clear();
        _exploitedMapTextureData.resize(int(GameData::getInstance()->getResourcesWidth() * GameData::getInstance()->getResourcesHeight()));
        _exploitedMapTexture.initWithData(&(_exploitedMapTextureData.at(0)), GameData::getInstance()->getResourcesWidth() * GameData::getInstance()->getResourcesHeight(), Texture2D::PixelFormat::RGBA8888, GameData::getInstance()->getResourcesWidth(), GameData::getInstance()->getResourcesHeight(), contentSize);
                
        for (int i = 0; i < im.getWidth(); i++) {
            for (int j = 0; j < im.getHeight(); j++) {
                unsigned char* pixel = data + ((int)i + (int)j * im.getWidth()) * 4;
                _exploitedMapTextureData.at(i + (j * im.getWidth())) = Color4B(*(pixel), *(pixel + 1), *(pixel + 2), *(pixel + 3));
            }
        }
        if(_exploitedMapSprite)
        {
            delete _exploitedMapSprite;
        }
        _exploitedMapSprite = Sprite::createWithTexture(&_exploitedMapTexture);
        _exploitedMapSprite->setPosition(Vec2(gameplayMap->getBoundingBox().size.width / 2, gameplayMap->getBoundingBox().size.height / 2));
        gameplayMap->addChild(_exploitedMapSprite, 2);
        
        /********** FOR LEVEL CONFIGURATION ***********/
        //max resources
        int wood = 0;
        int mineral = 0;
        for(int x = 0; x < 480; x++)
        {
            for(int y = 1; y <= 320; y++)
            {
                int valueAtMap = getValueAtGameplayMap(2,x,y);
                if (valueAtMap == 0)
                {
                    wood++;
                }
                else if (valueAtMap == 1)
                {
                    mineral++;
                }
            }
        }
        CCLOG("MAX RESOURCES MAP: Wood %d, Mineral %d", wood, mineral);
        /********** FOR LEVEL CONFIGURATION ***********/

        string filename = "level" + to_string(GameLevel::getInstance()->getNumLevel());
        LevelLoader loader;
        loader.loadMaxResources(filename);
    }
    else
    {
        _exploitedMapTextureData.clear();
    }


    // information map
    _infoMap = Sprite::create("maps/info/"+GameLevel::getInstance()->getMapFilename()+".png");
    _infoMap->setPosition(Vec2(gameplayMap->getBoundingBox().size.width / 2, gameplayMap->getBoundingBox().size.height / 2));
    _infoMap->setName("infoMap");
    _infoMap->setCascadeOpacityEnabled(true);
    createLegendEntries();
    _infoMap->setVisible(false);
    gameplayMap->addChild(_infoMap, 2);

    // deployment map
    auto deploymentMap = Sprite::create("maps/deployment/"+GameLevel::getInstance()->getMapFilename()+".png");
    deploymentMap->setPosition(Vec2(gameplayMap->getBoundingBox().size.width / 2, gameplayMap->getBoundingBox().size.height / 2));
   
    deploymentMap->setName("deploymentMap");
    const Color3B & playerColor = GameData::getInstance()->getPlayerColor();
    deploymentMap->setColor(playerColor);
    deploymentMap->setCascadeOpacityEnabled(true);
    deploymentMap->setVisible(true);
    deploymentMap->runAction(RepeatForever::create(Sequence::create(DelayTime::create(1.0f), FadeTo::create(1, 120), FadeTo::create(1, 255), nullptr)));


    gameplayMap->addChild(deploymentMap, 3);

    GameLevel::getInstance()->setUIGameplayMap(this);

    //INITIALIZE AGENTS AND EXPLOITED MAP TEXTUREDATA
    Color4B white;
    white.r = 127;
    white.g = 127;
    white.b = 127;
    white.a = 0;
    _agentsTextureData.resize(int(GameData::getInstance()->getResourcesWidth() * GameData::getInstance()->getResourcesHeight()));
    for(size_t i=0; i<_agentsTextureData.size(); i++)
    {
        _agentsTextureData.at(i) = white; // i is an index running from 0 to w*h-1
    }

    _agentsTexture.initWithData(&(_agentsTextureData.at(0)), GameData::getInstance()->getResourcesWidth() * GameData::getInstance()->getResourcesHeight(), Texture2D::PixelFormat::RGBA8888,
        GameData::getInstance()->getResourcesWidth(), GameData::getInstance()->getResourcesHeight(), contentSize);
    if(_agentsSprite)
    {
        delete _agentsSprite;
    }
    _agentsSprite = Sprite::createWithTexture(&_agentsTexture);
    _agentsSprite->setPosition(Vec2(gameplayMap->getBoundingBox().size.width / 2, gameplayMap->getBoundingBox().size.height / 2));
    gameplayMap->addChild(_agentsSprite, 2);
    _agentsSprite->setCascadeOpacityEnabled(true);

    //EVOLUTION POINTS
    evolutionPointsIcon = Sprite::create("gui/EvolutionPoints.png");
    evolutionPointsIcon->setAnchorPoint(Vec2(0.5, 1));
    evolutionPointsIcon->setPosition(3.4 * bottomFrame->getContentSize().width / 13, 4.9 * bottomFrame->getContentSize().height / 6);
    evolutionPointsLabel = Label::createWithTTF(to_string(GameLevel::getInstance()->getEvolutionPoints()), "fonts/monofonto.ttf", 70 * GameData::getInstance()->getRaConversion());
    evolutionPointsLabel->setAlignment(TextHAlignment::CENTER);
    evolutionPointsLabel->setPosition(evolutionPointsIcon->getContentSize().width / 2, evolutionPointsIcon->getContentSize().height / 2);
    evolutionPointsLabel->setColor(Color3B(216, 229, 235));
    evolutionPointsIcon->addChild(evolutionPointsLabel, 2);
    restaEvolutionPointsLabel = Label::createWithTTF("- ", "fonts/monofonto.ttf", 60 * GameData::getInstance()->getRaConversion());
    restaEvolutionPointsLabel->setColor(Color3B(211, 197, 0));
    restaEvolutionPointsLabel->setOpacity(0);
    evolutionPointsIcon->addChild(restaEvolutionPointsLabel, 2);
    bottomFrame->addChild(evolutionPointsIcon, 1);

    //TIME BUTTONS
    Vector<MenuItem*> timeButtons;
    MenuItem* playButton = MenuItemImage::create("gui/PlayButton.png", "gui/PlayButtonPressed.png");
    playButton->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    playButton->setName("playButton");

    MenuItem* pauseButton = MenuItemImage::create("gui/PauseButton.png", "gui/PauseButton.png");
    pauseButton->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    pauseButton->setEnabled(false);
    pauseButton->setName("pauseButton");

    pauseDarkBackground = Sprite::create("gui/EndedGameBackground.png");
    pauseDarkBackground->setName("pauseDarkBackground");
    pauseDarkBackground->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    pauseDarkBackground->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    pauseDarkBackground->setOpacity(100);
    auto pauseDarkLabel = Label::createWithTTF(string(LocalizedString::create("SET_ATTRIBUTES")), "fonts/BebasNeue.otf", 100 * GameData::getInstance()->getRaConversion());
    pauseDarkLabel->setTextColor(Color4B(216, 229, 235, 60));
    pauseDarkLabel->setPosition(Vec2(pauseDarkBackground->getContentSize().width / 2, pauseDarkBackground->getContentSize().height / 2));
    pauseDarkLabel->setName("pauseDarkLabel");
    pauseDarkBackground->addChild(pauseDarkLabel);
    addChild(pauseDarkBackground, 0);

    MenuItemToggle *toggle = MenuItemToggle::createWithCallback(CC_CALLBACK_1(UIGameplayMap::togglePlay, this), playButton, pauseButton, NULL);
    toggle->setPosition(Vec2(184 * visibleSize.width / 204, 144 * visibleSize.height / 155));
    toggle->setName("playToggle");

    Menu* timeMenu = Menu::create(toggle, NULL);
    timeMenu->setName("timeMenu");
    timeMenu->setPosition(Vec2(0, 0));
    this->addChild(timeMenu, 2);

    //Powers
    vector<Power*> pws = GameLevel::getInstance()->getPowers();
    for (size_t i = 0; i < pws.size(); i++)
    {
        Vec2 pos;
        if (i == 0)
        {
            pos.x = 14 * visibleSize.width / 204;
            pos.y = 11 * visibleSize.height / 155;
        }
        else
        {
            pos.x = powerButtons.at(i - 1)->getIcon()->getPosition().x + (4 * visibleSize.width / 204) + powerButtons.at(i - 1)->getIcon()->getBoundingBox().size.width;
            pos.y = powerButtons.at(i - 1)->getIcon()->getPosition().y;
        }
        
        if(pws.at(i)->isGlobal())
        {
            powerButtons.push_back(new UIGlobalPower(pws.at(i)));
            powerButtons.at(i)->setPosition(pos.x, pos.y);
            powerButtons.at(i)->getIcon()->setName("power"+std::to_string(i));
            this->addChild(powerButtons.at(i)->getIcon(), 3);
        }
        else
        {
            powerButtons.push_back(new UIAreaPower(pws.at(i)));
            powerButtons.at(i)->setPosition(pos.x, pos.y);
            powerButtons.at(i)->getIcon()->setName("power"+std::to_string(i));
            this->addChild(powerButtons.at(i)->getIcon(), 3);
            gameplayMap->addChild(((UIAreaPower*)powerButtons.at(i))->getArea(), 3);
        }
    }

    //SET GOALS ON MAP
    int numResources = 1;
    for (size_t i = 0; i < GameLevel::getInstance()->getGoals().size(); i++)
    {
        Goal * goal = GameLevel::getInstance()->getGoals().at(i);
        //Set Checkpoint Area
        if(goal->getGoalType()!= Dispersal)
        {
            if(((ResourcesGoal*)GameLevel::getInstance()->getGoals().at(i))->getResourceType() == Wood and !_isWood)
            {
                _isWood = true;
                numResources++;
            }
            if(((ResourcesGoal*)GameLevel::getInstance()->getGoals().at(i))->getResourceType() == Mineral and !_isMineral)
            {
                _isMineral = true;
                numResources++;
            }
            //do nothing
            continue;
        }
        DispersalGoal * expansionGoal = (DispersalGoal*)(goal);
        
        //FIND AREA
        int minX = 479;
        int maxX = 0;
        int minY = 319;
        int maxY = 0;
        for(int x = 0; x < 480; x++)
        {
            for(int y = 1; y <= 320; y++)
            {
                int goalCode = expansionGoal->getColorZone();
                int valueAtMap = getValueAtGameplayMap(1,x,y);
                if(goalCode!=valueAtMap)
                {
                    continue;
                }

                // check limits
                if(minX>x)
                {
                    minX = x;
                }
                if(maxX<x)
                {
                    maxX = x;
                }
    
                if(minY>y)
                {
                    minY = y;
                }
                if(maxY<y)
                {
                    maxY = y;
                }
            }
        }
        int centerX = maxX - ((maxX - minX) / 2);
        int centerY = maxY - ((maxY - minY) / 2);
        expansionGoal->setCenterArea(centerX, centerY);
        
        int x = (int)(centerX * float(GameData::getInstance()->getResourcesWidth() / 480.0));
        int y = (int)(float((GameData::getInstance()->getResourcesHeight() - GameData::getInstance()->getResourcesMargin()) / 2.0) + (centerY * float(GameData::getInstance()->getResourcesMargin() / 320.0)));
        
        auto area = Sprite::create("gui/CheckpointArea.png");
        area->setColor(Color3B::RED);
        area->setOpacity(0);
        area->setPosition(x, y);
        area->setTag(400 + int(i));
        gameplayMap->addChild(area, 3);
    }

    //TIME PROGRESS BAR
    timeBorderBar = Sprite::create("gui/ProgressBarBorder.png");
    timeBorderBar->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    timeBorderBar->setPosition(6 * visibleSize.width / 21, 15 * visibleSize.height / 16);
    auto timeBorderSize = timeBorderBar->getContentSize();
    timeBorderBar->setAnchorPoint(Vec2(0, 0.5));
    this->addChild(timeBorderBar, 3);
    auto barContent = Sprite::create("gui/ProgressBarContent.png");
    timeBar = ProgressTimer::create(barContent);
    timeBar->setType(ProgressTimer::Type::BAR);
    timeBar->setAnchorPoint(Vec2(0, 0));
    timeBar->setMidpoint(Vec2(0, 0));
    timeBar->setBarChangeRate(Vec2(1, 0));
    timeBar->setPosition(0, 0);
    timeBorderBar->addChild(timeBar, 3);
    
    auto degradateTime = Sprite::create("gui/GoalDegradate.png");
    degradateTime->setName("degradateTime");
    degradateTime->setPositionY(timeBar->getContentSize().height / 2);
    degradateTime->setVisible(false);
    degradateTime->setAnchorPoint(Vec2(0, 0.5));
    timeBorderBar->addChild(degradateTime, 2);

    //SET GOALS ON TIME PROGRESS BAR
    //float pixelPerStep = barContent->getTexture()->getPixelsWide() / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime();
    for (size_t i = 0; i < GameLevel::getInstance()->getGoals().size(); i++)
    {
        float posXaverage = (float)GameLevel::getInstance()->getGoals().at(i)->getAverageTime() / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime() * timeBorderBar->getContentSize().width;
        float posXcentered = (float)(GameLevel::getInstance()->getGoals().at(i)->getMinTime() + ((GameLevel::getInstance()->getGoals().at(i)->getMaxTime() - GameLevel::getInstance()->getGoals().at(i)->getMinTime()) / 2)) / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime() * timeBorderBar->getContentSize().width;
        
        auto goalMark1star = Sprite::create("gui/GoalMark1.png");
        goalMark1star->setPosition(posXcentered, (timeBorderBar->getContentSize().height / 2));
        float pixelPerStepMark = goalMark1star->getTexture()->getPixelsWide() / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime();
        float sizeInPixelsMark = (GameLevel::getInstance()->getGoals().at(i)->getMaxTime() - GameLevel::getInstance()->getGoals().at(i)->getMinTime()) * pixelPerStepMark;
        float sizeRatio = timeBorderBar->getContentSize().width/goalMark1star->getTexture()->getPixelsWide();
        goalMark1star->setScaleX(sizeRatio *sizeInPixelsMark / goalMark1star->getTexture()->getPixelsWide());
        timeBorderBar->addChild(goalMark1star, 1);
        
        auto goalMark2star = Sprite::create("gui/GoalMark2.png");
        goalMark2star->setPosition(posXaverage, (timeBorderBar->getContentSize().height / 2));
        pixelPerStepMark = goalMark2star->getTexture()->getPixelsWide() / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime();
        sizeInPixelsMark = (GameLevel::getInstance()->getGoals().at(i)->getDesviation2Star() * 2) * pixelPerStepMark;
        sizeRatio = timeBorderBar->getContentSize().width/goalMark2star->getTexture()->getPixelsWide();
        goalMark2star->setScaleX(sizeRatio * sizeInPixelsMark / goalMark2star->getTexture()->getPixelsWide());
        timeBorderBar->addChild(goalMark2star, 1);
        
        auto goalMark3star = Sprite::create("gui/GoalMark3.png");
        goalMark3star->setPosition(posXaverage, (timeBorderBar->getContentSize().height / 2));
        pixelPerStepMark = goalMark3star->getTexture()->getPixelsWide() / (float)GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime();
        sizeInPixelsMark = (GameLevel::getInstance()->getGoals().at(i)->getDesviation3Star() * 2) * pixelPerStepMark;
        sizeRatio = timeBorderBar->getContentSize().width/goalMark3star->getTexture()->getPixelsWide();
        goalMark3star->setScaleX(sizeRatio * sizeInPixelsMark / goalMark3star->getTexture()->getPixelsWide());
        timeBorderBar->addChild(goalMark3star, 1);
        
        /*auto goalHex = DrawNode::create();
        float R = (GameLevel::getInstance()->getGoals().at(i)->getDesviation3Star() * 2) * pixelPerStep / 2;
        Vec2 v[6];
        v[0]= Vec2(posXaverage - R, timeBorderSize.height);
        v[1]= Vec2(posXaverage, timeBorderSize.height + timeBorderSize.height / 10);
        v[2]= Vec2(posXaverage + R, timeBorderSize.height);
        v[3]= Vec2(posXaverage + R, 0);
        v[4]= Vec2(posXaverage, - timeBorderSize.height / 10);
        v[5]= Vec2(posXaverage - R, 0);
        timeBorderBar->addChild(goalHex, 2);
        goalHex->drawPolygon(v, 6, Color4F(Color4B(108, 185, 94, 240)), 1, Color4F::WHITE);//(v, 6, true, Color4F::WHITE);*/

        //auto goalNum = Sprite::create("gui/GoalIcon.png");
        //goalNum->setPosition(posXaverage, timeBorderBar->getContentSize().height / 2);
        /*goalNum->setScaleX(GameLevel::getInstance()->getGoals()[i]->getDesviation3Star() * (goalNum->getTexture()->getPixelsWide()
                           / (float)GameLevel::getInstance()->getGoals()[GameLevel::getInstance()->getGoals().size() - 1]->getMaxTime() - 1));*/
        /*auto labelGoalNum = Label::createWithTTF(to_string(i + 1), "fonts/BebasNeue.otf", 30);
        labelGoalNum->setPosition(goalNum->getContentSize().width / 2, goalNum->getContentSize().height / 2);
        labelGoalNum->setColor(Color3B(216, 229, 235));
        goalNum->addChild(labelGoalNum, 2);*/
        //timeBorderBar->addChild(goalNum, 2);
    }

    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesBegan = CC_CALLBACK_2(UIGameplayMap::onTouchesBegan, this);
    listener->onTouchesMoved = CC_CALLBACK_2(UIGameplayMap::onTouchesMoved, this);
    listener->onTouchesEnded = CC_CALLBACK_2(UIGameplayMap::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // set listener for tutorial
    _listenerTutorial = EventListenerTouchOneByOne::create();
    _listenerTutorial->setSwallowTouches(true);
    _listenerTutorial->onTouchBegan = CC_CALLBACK_2(UIGameplayMap::onTouchBeganTutorial, this);
    _listenerTutorial->onTouchEnded = CC_CALLBACK_2(UIGameplayMap::onTouchEndedTutorial, this);
    _eventDispatcher->addEventListenerWithFixedPriority(_listenerTutorial, -1);

    if (GameData::getInstance()->getMusic() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/level.mp3", true);
    }
    
    if (GameData::getInstance()->getGameStarted() == false) {
        GameLevel::getInstance()->createLevel();
    }
    GameData::getInstance()->setGameStarted(true);

    initializeAgents();

    const GameLevel::Levels & modifAttr = GameLevel::getInstance()->getModifiableAttr();
    
    auto graphicBackground = MenuItemImage::create("gui/GraphicBackground.jpg", "gui/GraphicBackground.jpg", CC_CALLBACK_1(UIGameplayMap::changeGraphicCallback, this));
    graphicBackground->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    graphicBackground->setPosition(10.15 * visibleSize.width / 11, 0.55 * visibleSize.height / 7.5);
    graphicBackground->setName("graphicBackground");

    auto menuGraphic = Menu::create(graphicBackground, nullptr);
    menuGraphic->setPosition(Vec2::ZERO);
    menuGraphic->setName("menuGraphic");
    this->addChild(menuGraphic, 1);
    
    auto labelGraphic = Label::createWithTTF(LocalizedString::create("POPULATION"), "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
    labelGraphic->setPosition(graphicBackground->getPositionX(), graphicBackground->getPositionY() + (graphicBackground->getBoundingBox().size.height / 2) + (labelGraphic->getBoundingBox().size.height / 2));
    labelGraphic->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    labelGraphic->setName("graphicLabel");
    this->addChild(labelGraphic);
    
    auto labelCounterGraphic = Label::createWithTTF("( 1 / " + to_string(numResources) + " )", "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
    labelCounterGraphic->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    labelCounterGraphic->setPosition(graphicBackground->getPositionX(), (graphicBackground->getPositionY() - (graphicBackground->getBoundingBox().size.height / 2))/2);
    labelCounterGraphic->setName("graphicCounterLabel");
    this->addChild(labelCounterGraphic);
    
    ///////////////////////////////////////////////   WAVE NODE   //////////////////////////////////////////////////////
    auto populationNode = new WaveNode();
    populationNode->init();
    populationNode->glDrawMode = kDrawLines;
    populationNode->setReadyToDrawDynamicVerts(true);
    populationNode->setName("POPULATION");
    graphicBackground->addChild(populationNode, 1, 1);
    waveNodes.push_back(populationNode);
    
    if (_isWood)
    {
        auto woodNode = new WaveNode();
        woodNode->init();
        woodNode->glDrawMode = kDrawLines;
        woodNode->setReadyToDrawDynamicVerts(true);
        woodNode->setName("WOOD");
        waveNodes.push_back(woodNode);
    }
    
    if (_isMineral)
    {
        auto mineralNode = new WaveNode();
        mineralNode->init();
        mineralNode->glDrawMode = kDrawLines;
        mineralNode->setReadyToDrawDynamicVerts(true);
        mineralNode->setName("MINERAL");
        waveNodes.push_back(mineralNode);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto attBackgroundTitle = Label::createWithTTF(LocalizedString::create("EVOLUTION_POINTS") + " - ", "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
    attBackgroundTitle->setColor(Color3B(216, 229, 236));
    attBackgroundTitle->setAnchorPoint(Vec2(0, 0.5));
    attBackgroundTitle->setPosition(Vec2(4 * bottomFrame->getContentSize().width / 13,
                                         4 * bottomFrame->getContentSize().height / 6));
    bottomFrame->addChild(attBackgroundTitle);
    
    auto attBackgroundTitleCost = Label::createWithTTF(" [ " + LocalizedString::create("COST") + " ]", "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
    attBackgroundTitleCost->setColor(Color3B(211, 197, 0));
    attBackgroundTitleCost->setAnchorPoint(Vec2(0, 0.5));
    attBackgroundTitleCost->setPosition(Vec2(attBackgroundTitle->getPositionX() + attBackgroundTitle->getContentSize().width,
                                         attBackgroundTitle->getPositionY()));
    bottomFrame->addChild(attBackgroundTitleCost);

    Vector<MenuItem*> attributesButtons;

    // offset between the 3 panels of attributes
    int tag = 0;
    float offsetAttrs = 0.182f;
    float frameHeight = 0.46f*bottomFrame->getContentSize().height - attBackgroundTitle->getContentSize().height * 1.2;//0.0466f*visibleSize.height;
    for (size_t j = 0; j < modifAttr.size(); j++)
    {   
        if(modifAttr.at(j)==-1)
        {
            continue;
        }
        auto labelAttRight = Label::createWithTTF(string(LocalizedString::create(GameLevel::getInstance()->convertAttIntToString(modifAttr.at(j)).c_str())), "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
        labelAttRight->setColor(Color3B(216, 229, 236));
        labelAttRight->setPosition(Vec2((j*offsetAttrs + 0.378f)* bottomFrame->getContentSize().width, 0.46f*bottomFrame->getContentSize().height));
        bottomFrame->addChild(labelAttRight, 1, (int(j) + 1) * 1000);

        auto costBackground = Sprite::create("gui/EvolutionPointsCost.png");
        costBackground->setPosition(Vec2((j*offsetAttrs + 0.325f)* bottomFrame->getContentSize().width, frameHeight));
        bottomFrame->addChild(costBackground, 1, (int(j) + 1) * 1200);
         
        auto attNumLabel = Label::createWithTTF(to_string(GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), modifAttr.at(j))), "fonts/monofonto.ttf", 45 * GameData::getInstance()->getRaConversion());
        attNumLabel->setColor(Color3B::BLACK);
        attNumLabel->setPosition(Vec2((j*offsetAttrs + 0.325f)* bottomFrame->getContentSize().width, frameHeight));
        bottomFrame->addChild(attNumLabel, 1, (int(j) + 1) * 1100);        
        

        auto plusAttButton = MenuItemImage::create("gui/PlusButtonSmall.png", "gui/PlusButtonSmallPressed.png", "gui/PlusButtonSmallPressed.png", CC_CALLBACK_1(UIGameplayMap::plusAttCallback, this));
        CCLOG("j %d offset %f vis %f", j, offsetAttrs, visibleSize.width);
        plusAttButton->setPosition(Vec2((j*offsetAttrs + 0.442f)* bottomFrame->getContentSize().width, frameHeight));
        plusAttButton->setTag(int(j) + 50);
        plusAttButton->setEnabled(false);
        plusAttButton->setName("plus"+labelAttRight->getString());
        attributesButtons.pushBack(plusAttButton);
        
        
        // offset between bars
        float offsetBars = 0.0122f;
        for (int m = 0; m < 5; m++)
        {
            auto attrBar = Sprite::create("gui/BlankAttributePointButtonSmall.png");
            if(m<GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), modifAttr.at(j)))
            {
               attrBar = Sprite::create("gui/FilledAttributePointButtonSmall.png");
            }
            float xPos = (0.354f + j*offsetAttrs + m*offsetBars)*bottomFrame->getContentSize().width;
            attrBar->setPosition(Vec2(xPos, frameHeight));
            bottomFrame->addChild(attrBar, 1, tag);
            tag++;
        }
    }

    Menu* attrMenu = cocos2d::Menu::createWithArray(attributesButtons);
    attrMenu->setPosition(Vec2(0, 0));
    attrMenu->setName("attrMenu");
    bottomFrame->addChild(attrMenu, 1, 100000);

    //CHECK IF THERE IS ANY IN-GAME ACHIEVEMENT (achievement num 3 of the corresponding level)
    if (GameData::getInstance()->getAchievements(GameLevel::getInstance()->getNumLevel()).size() > 2 and GameData::getInstance()->getAchievements(GameLevel::getInstance()->getNumLevel()).at(2)->getGoalType() == "Discovery" and GameData::getInstance()->getAchievements(GameLevel::getInstance()->getNumLevel()).at(2)->getCompleted() == false)
    {
        GameLevel::getInstance()->setInGameAchievement(GameData::getInstance()->getAchievements(GameLevel::getInstance()->getNumLevel()).at(2));
    }
    
    //SET FIRST GOAL ON MAP
    auto goal = DrawNode::create();
    goal->setName("goal");
    graphicBackground->addChild(goal);
    auto firstGoal = GameLevel::getInstance()->getGoals().at(0);
    
    if(firstGoal->getGoalType() == Resources)
    {
        moveGoalPopup(-1);
    }
    else if (firstGoal->getGoalType() == Dispersal)
    {
        auto area = (Sprite*)gameplayMap->getChildByTag(400);
        area->setOpacity(255);
        auto blink = Blink::create(2, 2);
        auto repeatBlink = RepeatForever::create(blink);
        area->setColor(Color3B::WHITE);
        area->runAction(repeatBlink);
    }
    
    createTutorialGUI();
    _message = 0;

    createTimingThread();
    createNewLevelThread();
    
    gameplayMap->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());

	auto mouseListener  = EventListenerMouse::create();
	mouseListener->onMouseScroll = CC_CALLBACK_1(UIGameplayMap::onMouseScroll, this);
	this->getEventDispatcher()->addEventListenerWithFixedPriority(mouseListener, 1);
        
    updateAttributesButtons();
    
    
    ///TEMP
    /*
    GameLevel::getInstance()->setFinishedGame(Success);
    GameData::getInstance()->setFirstTimeLevelCompleted(GameLevel::getInstance()->getNumLevel());
    */
    ////

    
    this->scheduleUpdate();
    return true;
}

bool UIGameplayMap::checkPowersClicked(void)
{
    for (size_t i = 0; i < powerButtons.size(); i++)
    {
        if (powerButtons.at(i)->getClicked() == true) {
            return true;
        }
    }
    return false;
}

bool UIGameplayMap::checkButtonPressed(Touch* touch)
{
    auto menu = (Menu*)this->getChildByName("bottomFrame")->getChildByName("attrMenu");
    Point touchLocation = menu->convertToNodeSpace(touch->getLocation());
    
    Vector<Node*> menuChildren = menu->getChildren();
    for (int i = 0; i < menuChildren.size(); i++)
    {
        Node *button = menuChildren.at(i);
        if(button->getBoundingBox().containsPoint(touchLocation))
        {
            return true;
        }
    }
    
    touchLocation = touch->getLocation();
    for (size_t i = 0; i < powerButtons.size(); i++)
    {
        if(powerButtons.at(i)->getIcon()->getBoundingBox().containsPoint(touchLocation))
        {
            return true;
        }
    }

    return false;
}

void UIGameplayMap::onTouchesBegan(const vector<Touch*>& touches, Event* event)
{
    if (endGameWindowPainted || checkPowersClicked())
    {
        return;
    }
    
    for (auto touch : touches)
    {
        _touches.pushBack(touch);
    }
    if (touches.size() == 1 and checkButtonPressed(touches.at(0)) == false)
    {
        if ((((clock() - float(timeFingerSpot)) / CLOCKS_PER_SEC) < 0.3) and (abs(touches[0]->getLocation().distance(firstTouchLocation)) < 40))
        {
            firstTouchLocation = touches.at(0)->getLocation();
            changeSpotPosition();
        }
    }

    for (auto touch : touches)
    {
        Point touchLocation = this->convertTouchToNodeSpace(touch);
        moveBackground = false;
        for (size_t i = 0; i < powerButtons.size(); i++)
        {
            powerButtons.at(i)->onTouchesBegan(touchLocation);
        }
        if (checkPowersClicked() == false and selectSpriteForTouch(gameplayMap, touchLocation))
        {
            moveBackground = true;
        }
    }
}

void UIGameplayMap::changeSpotPosition()
{
    //Size visibleSize = Director::getInstance()->getVisibleSize();
    float verticalMargin = gameplayMap->getContentSize().width / 1.5;
    if (verticalMargin > gameplayMap->getContentSize().height)
    {
        verticalMargin = gameplayMap->getContentSize().height;
    }
    
    Point nextDirection(gameplayMap->convertToNodeSpace(firstTouchLocation).x / float(gameplayMap->getContentSize().width / 480.0), (gameplayMap->convertToNodeSpace(firstTouchLocation).y - ((gameplayMap->getContentSize().height - verticalMargin) / 2)) / float(verticalMargin / 320.0));
    GameLevel::getInstance()->setAgentDirection(0, nextDirection);
        
    auto fadeFinger = FadeIn::create(1);
    fadeFinger->setTag(1);
        
    if (GameData::getInstance()->getSFX() == true)
    {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
        
    fingerSpot->setPosition(Vec2(gameplayMap->convertToNodeSpace(firstTouchLocation)));
    fingerSpot->setVisible(true);
    if(_tutorial && _tutorial->getCurrentMessage() && _tutorial->getCurrentMessage()->getPostCondition()=="spot")
    {
        _tutorial->getCurrentMessage()->postConditionAchieved();
    }
}

void UIGameplayMap::onTouchesMoved(const vector<Touch*>& touches, Event* event)
{
    if (endGameWindowPainted)
    {
        return;
    }
    // ZOOM
    if (_touches.size() >= 2)
    {
        if (checkPowersClicked() == false)
        {
            // Get the two first touches
            Touch *touch1 = (Touch*)_touches.at(0);
            Touch *touch2 = (Touch*)_touches.at(1);
            
            // Get current and previous positions of the touches
            Point curPosTouch1 = Director::getInstance()->convertToGL(touch1->getLocationInView());
            Point curPosTouch2 = Director::getInstance()->convertToGL(touch2->getLocationInView());
            Point prevPosTouch1 = Director::getInstance()->convertToGL(touch1->getPreviousLocationInView());
            Point prevPosTouch2 = Director::getInstance()->convertToGL(touch2->getPreviousLocationInView());
            
            // Calculate current and previous positions of the layer relative the anchor point
            Point curPosLayer = curPosTouch1.getMidpoint(curPosTouch2);
            Point prevPosLayer = prevPosTouch1.getMidpoint(prevPosTouch2);
            
            // Calculate new scale
            float prevScaleX = gameplayMap->getScaleX();
            float curScaleX = gameplayMap->getScaleX() * curPosTouch1.getDistance(curPosTouch2) / prevPosTouch1.getDistance(prevPosTouch2) ;
            
            float prevScaleY = gameplayMap->getScaleY();
            float curScaleY = gameplayMap->getScaleY() * curPosTouch1.getDistance(curPosTouch2) / prevPosTouch1.getDistance(prevPosTouch2) ;
            
            float xScale = MIN( MAX( curScaleX, GameData::getInstance()->getRaWConversion()), 3.0);
            float yScale = MIN( MAX( curScaleY, GameData::getInstance()->getRaHConversion()), 3.0 );
            
            //si no canvia x i y (aixi seviten deformacions)
            if (xScale != gameplayMap->getScaleX() and yScale != gameplayMap->getScaleY())
            {
                gameplayMap->setScale(xScale, yScale);
            }
            
            if( this->getScaleX() != prevScaleX )
            {
                Point realCurPosLayer = gameplayMap->convertToNodeSpaceAR(curPosLayer);
                float deltaX = (realCurPosLayer.x) * (gameplayMap->getScaleX() - prevScaleX);
                float deltaY = (realCurPosLayer.y) * (gameplayMap->getScaleY() - prevScaleY);
                
                gameplayMap->setPosition(Vec2(gameplayMap->getPosition().x - deltaX, gameplayMap->getPosition().y - deltaY));
            }
            
            // If current and previous position of the multitouch's center aren't equal -> change position of the layer
            if (!prevPosLayer.equals(curPosLayer))
            {
                gameplayMap->setPosition(Vec2(gameplayMap->getPosition().x + curPosLayer.x - prevPosLayer.x,
                                      gameplayMap->getPosition().y + curPosLayer.y - prevPosLayer.y));
            }
            
            Point reLocate = gameplayMap->getPosition();

            checkBackgroundLimitsInTheScreen(reLocate);
            while (!moveBackgroundLeft) {
                reLocate.x -= 2.0;
                checkBackgroundLimitsInTheScreen(reLocate);
            }
            while (!moveBackgroundRight) {
                reLocate.x += 2.0;
                checkBackgroundLimitsInTheScreen(reLocate);
            }
            while (!moveBackgroundUp) {
                reLocate.y += 2.0;
                checkBackgroundLimitsInTheScreen(reLocate);
            }
            while (!moveBackgroundDown) {
                reLocate.y -= 2.0;
                checkBackgroundLimitsInTheScreen(reLocate);
            }
            gameplayMap->setPosition(reLocate);
        }
    }
    // PAN
    else if (touches.size() == 1)
    {
        for (size_t i = 0; i < powerButtons.size(); i++)
        {
            powerButtons.at(i)->onTouchesMoved(touches.at(0));
        }
        if (moveBackground) {
            Point touchLocation = this->convertTouchToNodeSpace(touches.at(0));

            Point oldTouchLocation = touches.at(0)->getPreviousLocationInView();
            oldTouchLocation = Director::getInstance()->convertToGL(oldTouchLocation);
            oldTouchLocation = convertToNodeSpace(oldTouchLocation);

            Point translation = touchLocation - oldTouchLocation;
            Point newPos = gameplayMap->getPosition() + translation;

            checkBackgroundLimitsInTheScreen(newPos);

            Point destPos = gameplayMap->getPosition();
            if (moveBackgroundLeft and translation.x > 0) {
                destPos.x = newPos.x;
            }
            if (moveBackgroundRight and translation.x < 0) {
                destPos.x = newPos.x;
            }
            if (moveBackgroundUp and translation.y < 0) {
                destPos.y = newPos.y;
            }
            if (moveBackgroundDown and translation.y > 0) {
                destPos.y = newPos.y;
            }
            gameplayMap->setPosition(destPos);
        }
    }
}

bool UIGameplayMap::onTouchBeganTutorial(Touch * touch, Event* event)
{
    if(!_tutorial)
    {
        return false;
    }
    if (!_tutorial->getCurrentMessage())
    {
        return true;
    }
    
    Node * parent = this;
    Point touchLocation = parent->convertToNodeSpace(touch->getLocation());
    
    //SKIP
    auto tutorialWindow = parent->getChildByName("menuTutorialWindow")->getChildByName("tutorialWindow");
    auto skip = parent->getChildByName("menuTutorialWindow")->getChildByName("skipWindow");
    if(tutorialWindow->getBoundingBox().containsPoint(touchLocation) or skip->getBoundingBox().containsPoint(touchLocation))
    {
        return false;
    }
    
    //QUIT OR RETRY
    Node * quitRetry = parent->getChildByName("quitRetryMenu");
    if(quitRetry->getChildByName("quitButton")->getBoundingBox().containsPoint(touchLocation) or
       quitRetry->getChildByName("retryButton")->getBoundingBox().containsPoint(touchLocation))
    {
        return false;
    }
    
    string qrButton = "";
    if (quitRetry->getChildByName("quitButton")->getChildByName("confirmBackground") != nullptr)
    {
        Node * n = quitRetry->getChildByName("quitButton")->getChildByName("confirmBackground")->getChildByName("confirmMenu");
        
        //both buttons have same size
        Size buttonSize = n->getChildByName("confirmNo")->getContentSize();
        
        Vec2 posRNO = n->convertToWorldSpace(n->getChildByName("confirmNo")->getPosition());
        posRNO.x -= buttonSize.width / 2;
        posRNO.y -= buttonSize.height / 2;
        Rect rNo(posRNO, buttonSize);
        
        Vec2 posROK = n->convertToWorldSpace(n->getChildByName("confirmOk")->getPosition());
        posROK.x -= buttonSize.width / 2;
        posROK.y -= buttonSize.height / 2;
        Rect rOk(posROK, buttonSize);
        
        if (rNo.containsPoint(touchLocation) or rOk.containsPoint(touchLocation))
        {
            return false;
        }
    }
    if (quitRetry->getChildByName("retryButton")->getChildByName("confirmBackground") != nullptr)
    {
        Node * n = quitRetry->getChildByName("retryButton")->getChildByName("confirmBackground")->getChildByName("confirmMenu");
        
        //both buttons have same size
        Size buttonSize = n->getChildByName("confirmNo")->getContentSize();
        
        Vec2 posRNO = n->convertToWorldSpace(n->getChildByName("confirmNo")->getPosition());
        posRNO.x -= buttonSize.width / 2;
        posRNO.y -= buttonSize.height / 2;
        Rect rNo(posRNO, buttonSize);
        
        Vec2 posROK = n->convertToWorldSpace(n->getChildByName("confirmOk")->getPosition());
        posROK.x -= buttonSize.width / 2;
        posROK.y -= buttonSize.height / 2;
        Rect rOk(posROK, buttonSize);
        
        if (rNo.containsPoint(touchLocation) or rOk.containsPoint(touchLocation))
        {
            return false;
        }
        
    }
    
    if(_tutorial->getCurrentMessage()->getPostCondition() == "tap")
    {
        _tutorial->getCurrentMessage()->postConditionAchieved();
    }

    return true;
}

void UIGameplayMap::onTouchEndedTutorial(Touch * touch, Event* event)
{
    if(!_tutorial)
    {
        return;
    }
    if (!_tutorial->getCurrentMessage())
    {
        return;
    }
    
    Node * parent = this;
    Point touchLocation = parent->convertToNodeSpace(touch->getLocation());
    
    if(_tutorial->getCurrentMessage()->getPostCondition() == "spot")
    {
        if (((clock() - float(timeFingerSpot)) / CLOCKS_PER_SEC) < 0.3)
        {
            firstTouchLocation = touchLocation;
            changeSpotPosition();
        }
        timeFingerSpot = clock();
        return;
    }
    
    if (_tutorial->getCurrentMessage()->getPostCondition() != "tapButton")
    {
        return;
    }
    
    std::string buttonName = _tutorial->getCurrentMessage()->getPostConditionButtonTap();
    parent = this;
    std::size_t pos;
    std::string delimiter = "/";
    std::string token;
    while ((pos = buttonName.find(delimiter)) != std::string::npos)
    {
        token = buttonName.substr(0, pos);
        parent = parent->getChildByName(token);
        buttonName.erase(0, pos + delimiter.length());
    }
    
    token = buttonName.substr(0, pos);
    std::string buttonPressed = token;
    
    //attribute translation
    if (token.substr(0, 4) == "plus")
    {
        buttonPressed = "plus";
        token = "plus" + LocalizedString::create(token.substr(4).c_str());
    }
    
    Node * button = parent->getChildByName(token);
    touchLocation = parent->convertToNodeSpace(touch->getLocation());
    if(button->getBoundingBox().containsPoint(touchLocation))
    {
        if (buttonPressed == "playToggle")
        {
            togglePlay(button);
        }
        else if (buttonPressed == "plus")
        {
            plusAttCallback(button);
        }
        else if (buttonPressed == "power0")
        {
            powerButtons.at(0)->onTouchesBegan(touchLocation);
            bool actioned = powerButtons.at(0)->onTouchesEnded(touchLocation);
            //ANIMACIO RESTA PUNTS
            if (actioned == true)
            {
                restaEvolutionPointsLabel->setPosition(evolutionPointsIcon->getContentSize().width / 2, evolutionPointsIcon->getContentSize().height / 2);
                restaEvolutionPointsLabel->setString("-" + to_string(int(powerButtons.at(0)->getPower()->getCost())));
                auto mov = MoveTo::create(1.5, Vec2(evolutionPointsIcon->getContentSize().width / 2, - evolutionPointsIcon->getContentSize().height / 2));
                restaEvolutionPointsLabel->runAction(Spawn::create(mov, Sequence::create(FadeIn::create(0.5), FadeOut::create(1.0), NULL), NULL));
            }
            else
            {
                powerButtons.at(0)->disabled = true;
            }
        }
        _tutorial->getCurrentMessage()->postConditionAchieved();
    }
}

void UIGameplayMap::onTouchesEnded(const vector<Touch*>& touches, Event* event)
{
    if (endGameWindowPainted)
    {
        return;
    }
    Point touchLocation = this->convertTouchToNodeSpace(touches.at(0));
    for(size_t i = 0; i < powerButtons.size(); i++)
    {
        bool actioned = powerButtons.at(i)->onTouchesEnded(touchLocation);
        //ANIMACIO RESTA PUNTS
        if (actioned == true)
        {
            restaEvolutionPointsLabel->setPosition(evolutionPointsIcon->getContentSize().width / 2, evolutionPointsIcon->getContentSize().height / 2);
            restaEvolutionPointsLabel->setString("-" + to_string(int(powerButtons.at(i)->getPower()->getCost())));
            auto mov = MoveTo::create(1.5, Vec2(evolutionPointsIcon->getContentSize().width / 2, - evolutionPointsIcon->getContentSize().height / 2));
            restaEvolutionPointsLabel->runAction(Spawn::create(mov, Sequence::create(FadeIn::create(0.5), FadeOut::create(1.0), NULL), NULL));
        }
        else
        {
            powerButtons.at(i)->disabled = true;
        }
    }
    moveBackground = false;
    _touches.clear();

    firstTouchLocation = touches.at(0)->getLocation();
    timeFingerSpot = clock();
}

void UIGameplayMap::onMouseScroll(Event* event)
{
    if (endGameWindowPainted)
    {
        return;
    }

    // ZOOM
    EventMouse* e = (EventMouse*)event;
    if(checkPowersClicked() == false and (!_tutorial or _tutorial->isFinished()))
    {
        // Get current and previous positions of the touches
        Point curPosTouch1 = Director::getInstance()->convertToGL(e->getLocationInView());
        Point prevPosTouch1 = Director::getInstance()->convertToGL(e->getPreviousLocationInView());
        
        // Calculate new scale
        float prevScaleX = gameplayMap->getScaleX();
        float curScaleX = gameplayMap->getScaleX() - (e->getScrollY() / 10) * GameData::getInstance()->getRaWConversion();
        
        float prevScaleY = gameplayMap->getScaleY();
        float curScaleY = gameplayMap->getScaleY() - (e->getScrollY() / 10) * GameData::getInstance()->getRaHConversion();
        
        gameplayMap->setScale(MIN( MAX( curScaleX, GameData::getInstance()->getRaWConversion()),
                                  3.0 * GameData::getInstance()->getRaWConversion()),
                              MIN( MAX( curScaleY, GameData::getInstance()->getRaHConversion()),
                                  3.0 * GameData::getInstance()->getRaHConversion()));
        
        if( this->getScaleX() != prevScaleX )
        {
            Point realCurPosLayer = gameplayMap->convertToNodeSpaceAR(curPosTouch1);
            float deltaX = (realCurPosLayer.x) * (gameplayMap->getScaleX() - prevScaleX);
            float deltaY = (realCurPosLayer.y) * (gameplayMap->getScaleY() - prevScaleY);
            
            gameplayMap->setPosition(Vec2(gameplayMap->getPosition().x - deltaX, gameplayMap->getPosition().y + deltaY));
        }
        
        Point reLocate = gameplayMap->getPosition();

        checkBackgroundLimitsInTheScreen(reLocate);
        while (!moveBackgroundLeft) {
            reLocate.x -= 2.0;
            checkBackgroundLimitsInTheScreen(reLocate);
        }
        while (!moveBackgroundRight) {
            reLocate.x += 2.0;
            checkBackgroundLimitsInTheScreen(reLocate);
        }
        while (!moveBackgroundUp) {
            reLocate.y += 2.0;
            checkBackgroundLimitsInTheScreen(reLocate);
        }
        while (!moveBackgroundDown) {
            reLocate.y -= 2.0;
            checkBackgroundLimitsInTheScreen(reLocate);
        }

        gameplayMap->setPosition(reLocate);
    }
}

void UIGameplayMap::menuBackCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click.mp3");
    }
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    _eventDispatcher->removeEventListener(_listenerTutorial);
    GameLevel::getInstance()->setFinishedGame(UserCancel);
    while (GameLevel::getInstance()->ended == false)
        ;
    //pthread_cancel(gameLevelThread);
    //pthread_cancel(timingThread);
    pthread_join(gameLevelThread, nullptr);
    pthread_join(timingThread, nullptr);
    GameLevel::getInstance()->play(false);
    GameData::getInstance()->setGameStarted(false);
    
    if (GameData::getInstance()->getFirstTimeLevelCompleted() == GameLevel::getInstance()->getNumLevel() or GameData::getInstance()->getFirstTimeLevelCompleted() == 21)
    {
        auto scene = UITransitionScene::createScene();
        auto transition = TransitionFade::create(0.2f, scene);
        Director::getInstance()->replaceScene(transition);
    }
    else
    {
        auto scene = UIProgressMap::createScene();
        auto transition = TransitionFade::create(1.0f, scene);
        Director::getInstance()->replaceScene(transition);
    }
}

void UIGameplayMap::pauseGame()
{
    ((MenuItemToggle*)this->getChildByName("timeMenu")->getChildByName("playToggle"))->setSelectedIndex(0);
    pauseDarkBackground->setVisible(true);
    GameLevel::getInstance()->play(false);
}

void UIGameplayMap::togglePlay(Ref* pSender)
{
    if (endGameWindowPainted)
    {
        return;
    }
    
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
    if (firstPlayFF == true)
    {
        setAttributesToInitialAgents();
        pauseDarkBackground->setOpacity(150);
        auto l = ((Label*)pauseDarkBackground->getChildByName("pauseDarkLabel"));
        l->setTTFConfig(_ttfConfig("fonts/BebasNeue.otf", 150 * GameData::getInstance()->getRaConversion()));
        l->setString(string(LocalizedString::create("PAUSE")));
        timeBorderBar->getChildByName("degradateTime")->setVisible(true);
        auto deploymentMap = gameplayMap->getChildByName("deploymentMap");
        deploymentMap->setVisible(false);
        firstPlayFF = false;
    }

    if(!GameLevel::getInstance()->isPlaying())
    {
        pauseDarkBackground->setVisible(false);
        GameLevel::getInstance()->play(true);
        if (GameData::getInstance()->getLevelsFailedForHint().at(GameLevel::getInstance()->getNumLevel()) > 2)
        {
            this->getChildByName("hintBackground")->setVisible(false);
            this->getChildByName("hintLabel")->setVisible(false);
            ((MenuItemToggle*)this->getChildByName("menuHint")->getChildByName("hintButton"))->setSelectedIndex(0);
        }

    }
    else
    {
        pauseGame();
    }
}

void UIGameplayMap::quitCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click.mp3");
    }
    MenuItem* p = (MenuItem*)pSender;
    p->setEnabled(false);
    auto confirmBackground = Sprite::create("gui/ConfirmBackground.png");
    confirmBackground->setName("confirmBackground");
    Vector<MenuItem*> confirmVec;
    MenuItem* confirmOk = MenuItemImage::create("gui/ConfirmOk.png", "gui/ConfirmOkPressed.png", CC_CALLBACK_1(UIGameplayMap::menuBackCallback, this));
    confirmOk->setName("confirmOk");
    MenuItem* confirmNo = MenuItemImage::create("gui/ConfirmNo.png", "gui/ConfirmNoPressed.png", CC_CALLBACK_1(UIGameplayMap::NoCallback, this));
    confirmNo->setName("confirmNo");
    auto separator = Sprite::create("gui/ConfirmSeparator.png");
    separator->setPosition(Vec2(3 * confirmBackground->getContentSize().width / 4, confirmBackground->getContentSize().height / 2));
    confirmOk->setPosition(Vec2(5 * confirmBackground->getContentSize().width / 8, confirmBackground->getContentSize().height / 2));
    confirmNo->setPosition(Vec2(7 * confirmBackground->getContentSize().width / 8, confirmBackground->getContentSize().height / 2));
    confirmVec.pushBack(confirmOk);
    confirmVec.pushBack(confirmNo);
    Menu* confirmMenu = Menu::createWithArray(confirmVec);
    confirmMenu->setName("confirmMenu");
    confirmMenu->setPosition(0, 0);
    auto labelConfirm = Label::createWithTTF(string(LocalizedString::create("QUIT_QUESTION")), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    labelConfirm->setPosition(Vec2(confirmBackground->getContentSize().width / 4, confirmBackground->getContentSize().height / 2));
    confirmBackground->addChild(separator);
    confirmBackground->addChild(confirmMenu, 10);
    confirmBackground->addChild(labelConfirm);
    confirmBackground->setAnchorPoint(Vec2(0, 0.5));
    confirmBackground->setPosition(Vec2(p->getContentSize().width - confirmBackground->getContentSize().width, p->getContentSize().height / 2));
    p->addChild(confirmBackground, -1);
    auto moveConfirm = MoveTo::create(0.4, Vec2(p->getContentSize().width, p->getContentSize().height / 2));
    auto ease = EaseBackOut::create(moveConfirm);
    confirmBackground->runAction(ease);
}

void UIGameplayMap::retryCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click.mp3");
    }
    MenuItem* p = (MenuItem*)pSender;
    p->setEnabled(false);
    auto confirmBackground = Sprite::create("gui/ConfirmBackground.png");
    confirmBackground->setName("confirmBackground");
    Vector<MenuItem*> confirmVec;
    auto confirmOk = MenuItemImage::create("gui/ConfirmOk.png", "gui/ConfirmOkPressed.png", CC_CALLBACK_1(UIGameplayMap::retryOkCallback, this));
    confirmOk->setName("confirmOk");
    auto confirmNo = MenuItemImage::create("gui/ConfirmNo.png", "gui/ConfirmNoPressed.png", CC_CALLBACK_1(UIGameplayMap::NoCallback, this));
    confirmNo->setName("confirmNo");
    auto separator = Sprite::create("gui/ConfirmSeparator.png");
    separator->setPosition(Vec2(3 * confirmBackground->getContentSize().width / 4, confirmBackground->getContentSize().height / 2));
    confirmOk->setPosition(Vec2(5 * confirmBackground->getContentSize().width / 8, confirmBackground->getContentSize().height / 2));
    confirmNo->setPosition(Vec2(7 * confirmBackground->getContentSize().width / 8, confirmBackground->getContentSize().height / 2));
    confirmVec.pushBack(confirmOk);
    confirmVec.pushBack(confirmNo);
    Menu* confirmMenu = Menu::createWithArray(confirmVec);
    confirmMenu->setName("confirmMenu");
    confirmMenu->setPosition(0, 0);
    auto labelConfirm = Label::createWithTTF(string(LocalizedString::create("RETRY_QUESTION")), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    labelConfirm->setPosition(Vec2(confirmBackground->getContentSize().width / 4, confirmBackground->getContentSize().height / 2));
    confirmBackground->addChild(separator);
    confirmBackground->addChild(confirmMenu);
    confirmBackground->addChild(labelConfirm);
    confirmBackground->setAnchorPoint(Vec2(0, 0.5));
    confirmBackground->setPosition(Vec2(p->getContentSize().width - confirmBackground->getContentSize().width, p->getContentSize().height / 2));
    p->addChild(confirmBackground, -1);
    auto moveConfirm = MoveTo::create(0.4, Vec2(p->getContentSize().width, p->getContentSize().height / 2));
    auto ease = EaseBackOut::create(moveConfirm);
    confirmBackground->runAction(ease);
}

void UIGameplayMap::retryOkCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click.mp3");
    }
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    GameLevel::getInstance()->setFinishedGame(UserCancel);

    //pthread_cancel(gameLevelThread);
    //pthread_cancel(timingThread);
    pthread_join(gameLevelThread, nullptr);
    pthread_join(timingThread, nullptr);
    GameLevel::getInstance()->play(false);
    GameData::getInstance()->setGameStarted(false);
    string filename = "level" + to_string(GameLevel::getInstance()->getNumLevel());
    GameLevel::getInstance()->resetLevel();
    LevelLoader loader;
    loader.loadXmlFile(filename);
    
    _eventDispatcher->removeEventListener(_listenerTutorial);
    GameLevel::getInstance()->setAgentAttributesInitToCurrent();
    auto scene = UIGameplayMap::createScene();
    auto transition = TransitionFade::create(1.0f, scene);
    Director::getInstance()->replaceScene(transition);
}

void UIGameplayMap::NoCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
    MenuItem* p = (MenuItem*)((MenuItem*)pSender)->getParent()->getParent();
    MenuItem* p2 = (MenuItem*)p->getParent();
    p2->setEnabled(true);
    auto moveConfirm = MoveTo::create(0.8, Vec2(p2->getContentSize().width - p->getContentSize().width,
                                               p2->getContentSize().height / 2));
    auto ease = EaseBackOut::create(moveConfirm);
    p->runAction(ease);
    p2->removeChild(p);
}

void UIGameplayMap::plusAttCallback(Ref* pSender)
{
    if (endGameWindowPainted)
    {
        return;
    }
    
    MenuItem* pMenuItem = (MenuItem*)(pSender);
    int tag = pMenuItem->getTag();
    int i = tag - 50;
    Sprite* layout = (Sprite*)(pMenuItem->getParent()->getParent());

    // XRC TODO fix this
    if (GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) >= 5 or GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) > GameLevel::getInstance()->getEvolutionPoints())
    {
        return;
    }
    
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }

    GameLevel::getInstance()->setAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr()[i], GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) + 1);
    GameLevel::getInstance()->setEvolutionPoints(GameLevel::getInstance()->getEvolutionPoints() - GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)));
    int oldCost = GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i));
    GameLevel::getInstance()->setAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i), GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) + 1);
    Label* l = (Label*)layout->getChildByTag((i + 1) * 1100);
//    l->setAnchorPoint(Vec2(-2.5, 0.5));
    if(oldCost<5)
    {
        l->setString(to_string(GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i))));
    }
    // top level
    else
    {
        Sprite * costBackground = (Sprite*)(layout->getChildByTag((i+1)*1200));
        costBackground->setTexture("gui/EvolutionPointsCostDisabled.png");
        l->setString("");
    }

    auto blankAttribute = layout->getChildByTag((GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) - 1) + (i * 5));
    auto filledAttribute = Sprite::create("gui/FilledAttributePointButtonSmall.png");
    filledAttribute->setPosition(blankAttribute->getPosition());
    layout->removeChildByTag((GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) - 1) + (i * 5));
    layout->addChild(filledAttribute, 1, (GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) - 1) + (i * 5));
    
    //ANIMATION
    restaEvolutionPointsLabel->setPosition(evolutionPointsIcon->getContentSize().width / 2, evolutionPointsIcon->getContentSize().height / 2);
    restaEvolutionPointsLabel->setString("-" + to_string(oldCost));
    auto mov = MoveTo::create(1.5, Vec2(evolutionPointsIcon->getContentSize().width / 2, - evolutionPointsIcon->getContentSize().height / 2));
    restaEvolutionPointsLabel->runAction(Spawn::create(mov, Sequence::create(FadeIn::create(0.5), FadeOut::create(1.0), NULL), NULL));
}

void UIGameplayMap::hideAchievementWindowCallback(Ref* pSender)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto window = (MenuItemImage*)pSender;
    window->runAction(EaseBackIn::create(MoveTo::create(0.4, Vec2(visibleSize.width / 2, visibleSize.height + window->getContentSize().height))));
}

void UIGameplayMap::removeFingerSpot(Ref* pSender)
{
    if (endGameWindowPainted)
    {
        return;
    }
    
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
    GameLevel::getInstance()->setAgentDirection(0, Point(-1, -1));
    fingerSpot->setVisible(false);
}

void UIGameplayMap::changeGraphicCallback(Ref* pSender)
{    
    auto graphicBackground = (MenuItem*) pSender;
    auto graphicLabel = (Label*)this->getChildByName("graphicLabel");
    auto graphicCounterLabel = (Label*)this->getChildByName("graphicCounterLabel");
    string name;
    for (size_t i = 0; i < graphicBackground->getChildren().size(); i++)
    {
        if ((name = graphicBackground->getChildren().at(i)->getName()) != "" and name != "goal")
        {
            i = graphicBackground->getChildren().size();
        }
    }
    
    int i = 0;
    while (i < waveNodes.size() and waveNodes.at(i)->getName() != name)
    {
        i++;
    }
    
    graphicBackground->removeChild(waveNodes.at(i));
    if (i == waveNodes.size() - 1)
    {
        i = 0;
    }
    else
    {
        i++;
    }
    string currentName = waveNodes.at(i)->getName();
    graphicBackground->addChild(waveNodes.at(i), 1, 1);
    graphicLabel->setString(LocalizedString::create(currentName.c_str()));
    
    int numResources = 1;
    if (_isWood)
    {
        numResources++;
    }
    if (_isMineral)
    {
        numResources++;
    }
    graphicCounterLabel->setString("( " + to_string(i+1) + " / " + to_string(numResources) + " )");
    
    auto goal = (DrawNode*) graphicBackground->getChildByName("goal");
    goal->clear();
    
    i = 0;
    while (i < GameLevel::getInstance()->getGoals().size() and GameLevel::getInstance()->getGoals().at(i)->getCompleted() == true)
    {
        i++;
    }
    if (GameLevel::getInstance()->getGoals().at(i)->getGoalType() == Resources)
    {
        if (GameData::getInstance()->getSFX() == true) {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
        }
        auto goalCollection = (ResourcesGoal*)GameLevel::getInstance()->getGoals().at(i);
        int graphResource = PopulationRes;
        Color4B colorGoal(GameData::getInstance()->getPlayerColor());
        if (currentName == "WOOD")
        {
            graphResource = Wood;
            colorGoal = Color4B(103, 222, 31, 255);
        }
        else if (currentName == "MINERAL")
        {
            graphResource = Mineral;
            colorGoal = Color4B(241, 200, 10, 255);
        }
        if (goalCollection->getResourceType() == graphResource)
        {
            float height;
            if (graphResource == PopulationRes)
            {
                height = float(goalCollection->getGoalAmount())/float(GameLevel::getInstance()->getMaxAgents().at(0)) * graphicBackground->getContentSize().height;
            }
            else
            {
                height = float(goalCollection->getGoalAmount())/float(Agent::_resourcesPoolMax.at(goalCollection->getResourceType())) * graphicBackground->getContentSize().height;
            }
        
            // Space the verticies out evenly across the screen for the wave.
            float vertexHorizontalSpacing = graphicBackground->getContentSize().width / float(GameLevel::getInstance()->getGoals().back()->getMaxTime());
        
            goal->drawSegment(Vec2(vertexHorizontalSpacing, height), Vec2(vertexHorizontalSpacing * GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime(), height), 1, Color4F(Color4B(115, 148, 155, 200)));
        
            goal->drawSegment(Vec2(vertexHorizontalSpacing * goalCollection->getMinTime(), height), Vec2(vertexHorizontalSpacing * goalCollection->getMaxTime(), height), 3, Color4F(colorGoal));
        }
    }
}

void UIGameplayMap::skipTutorial(Ref* pSender)
{
    auto window = (MenuItemToggle*)pSender;
    auto skipWindow = (MenuItem*)window->getParent()->getChildByName("skipWindow");
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
    if (skipWindow->isVisible())
    {
        skipWindow->runAction(FadeOut::create(0.3));
        skipWindow->setVisible(false);
        window->setSelectedIndex(0);
    }
    else
    {
        skipWindow->setVisible(true);
        skipWindow->runAction(FadeIn::create(0.2));
        window->setSelectedIndex(1);
    }
}

void UIGameplayMap::skipTutorialConfirm(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }
    _tutorial->removeCurrentMessage();
    this->getChildByName("tutorial")->setVisible(false);
    this->getChildByName("tutorialNext")->setVisible(false);
    this->getChildByName("tutorialBorder")->setVisible(false);
    this->getChildByName("tutorialImage")->stopAllActions();
    this->getChildByName("tutorialImage")->setVisible(false);
    _tutorial->removeAllMessages();
    _eventDispatcher->removeEventListener(_listenerTutorial);
    disableTutorialGUI();
    pauseGame();
}

void UIGameplayMap::menuHintCallback(Ref* pSender)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
    }

    auto hintButton = (MenuItemToggle*) pSender;
    if (hintButton->getSelectedIndex() == 0)
    {
        this->getChildByName("hintBackground")->setVisible(false);
        this->getChildByName("hintLabel")->setVisible(false);
        //RESUME STATE
        if (_playingBeforeHintPressed == true)
        {
            //play
            ((MenuItemToggle*)this->getChildByName("timeMenu")->getChildByName("playToggle"))->setSelectedIndex(1);
            togglePlay(this->getChildByName("timeMenu")->getChildByName("playToggle"));
        }
    }
    else if (hintButton->getSelectedIndex() == 1)
    {
        this->getChildByName("hintBackground")->setVisible(true);
        this->getChildByName("hintLabel")->setVisible(true);
        //SAVE STATE
        _playingBeforeHintPressed = GameLevel::getInstance()->isPlaying();
        if(GameLevel::getInstance()->isPlaying() == true)
        {
            pauseGame();
        }
    }
    
    hintButton->setSelectedIndex(hintButton->getSelectedIndex());
}

void UIGameplayMap::createTimingThread(void)
{
    pthread_create(&timingThread, NULL, &UIGameplayMap::createTiming, this);
}

void* UIGameplayMap::createTiming(void* arg)
{
    UIGameplayMap* game = (UIGameplayMap*)arg;
    game->startTiming();
    //delete game;
    return nullptr;
}

void UIGameplayMap::startTiming(void)
{
    Timing::getInstance()->start();
}

void UIGameplayMap::createNewLevelThread(void)
{
    pthread_create(&gameLevelThread, NULL, &UIGameplayMap::createLevel, this);
}

void* UIGameplayMap::createLevel(void* arg)
{
    UIGameplayMap* game = (UIGameplayMap*)arg;
    game->playLevel();
    //delete game;
    return nullptr;
}

void UIGameplayMap::enableTutorialGUI()
{
    this->getChildByName("menuTutorialWindow")->setVisible(true);
}

void UIGameplayMap::disableTutorialGUI()
{
    this->getChildByName("menuTutorialWindow")->setVisible(false);
}


void UIGameplayMap::playLevel(void)
{
    bool launchTutorial = GameData::getInstance()->launchTutorial(GameLevel::getInstance()->getNumLevel());
    delete _tutorial;
    _tutorial = nullptr;
    disableTutorialGUI();

    if(launchTutorial)
    {
        _tutorial = new Tutorial();
        /** delete tutorial if level doesn't have messages **/
        if(!_tutorial->loadTutorial())
        {
            delete _tutorial;
            _tutorial = nullptr;
        }
        else
        {
            enableTutorialGUI();
        }
    }
    GameLevel::getInstance()->playLevel();
}

bool UIGameplayMap::selectSpriteForTouch(Node* sprite, Point touchLocation)
{
    return sprite->getBoundingBox().containsPoint(touchLocation);
}

std::string UIGameplayMap::getGoalIcon( const Goal * goal ) const
{
    string currGoal = "Dispersal";
    if (goal->getGoalType() == Resources)
    {
        auto goalResources = (ResourcesGoal*)goal;
        currGoal = "Population";
        if(goalResources->getResourceType()==Wood)
        {
            currGoal = "ResourcesWood";
        }
        // mineral
        else if(goalResources->getResourceType()==Mineral)
        {
            currGoal = "ResourcesMineral";
        }
    }
    return "gui/goals/"+currGoal+"Goal.png";
}

void UIGameplayMap::moveGoalPopup(int index)
{
    Goal * goal;
    if (index >= 0)
    {
        if (GameData::getInstance()->getSFX() == true) {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/goal.mp3");
        }
        goal = GameLevel::getInstance()->getGoals().at(index);
        // erase completed goal
        if(goal->getGoalType() == Resources)
        {
            auto graphicBackground = (MenuItem*)(this->getChildByName("menuGraphic")->getChildByName("graphicBackground"));
            auto goal = (DrawNode*) graphicBackground->getChildByName("goal");
            
            goal->clear();
        }
        else
        {
            auto area = gameplayMap->getChildByTag(400+index);
            area->stopAllActions();
            area->setVisible(true);
            area->setColor(Color3B(223, 211, 39));
            auto fadeOut = FadeOut::create(2.5);
            area->runAction(fadeOut);
        }
        
        auto goalsCompletedLabel = (Label*)this->getChildByName("goalsCompletedLabel");
        goalsCompletedLabel->setString(to_string(index+1) + " " + LocalizedString::create("OF") + " " + to_string(GameLevel::getInstance()->getGoals().size()) + " " + LocalizedString::create("GOALS_COMPLETED"));
        goalsCompletedLabel->setOpacity(0);
        goalsCompletedLabel->runAction(Sequence::create(FadeTo::create(1.0, 200), DelayTime::create(0.8), FadeTo::create(1.0, 0), NULL));
    }
    
    // if it's not the last goal, highlight the next one
    if(index != (GameLevel::getInstance()->getGoals().size()-1))
    {
        goal = GameLevel::getInstance()->getGoals().at(index+1);
        if(goal->getGoalType() == Resources)
        {
            auto graphicBackground = (MenuItem*)(this->getChildByName("menuGraphic")->getChildByName("graphicBackground"));
            auto graphicLabel = (Label*)this->getChildByName("graphicLabel");
            auto graphicCounterLabel = (Label*)this->getChildByName("graphicCounterLabel");
            auto collectionGoal = (ResourcesGoal*)goal;
            auto goal = (DrawNode*) graphicBackground->getChildByName("goal");
            string name;
            for (size_t i = 0; i < graphicBackground->getChildren().size(); i++)
            {
                if ((name = graphicBackground->getChildren().at(i)->getName()) != "" and name != "goal")
                {
                    i = graphicBackground->getChildren().size();
                }
            }
            
            int i = 0;
            while (i < waveNodes.size() and waveNodes.at(i)->getName() != name)
            {
                i++;
            }
            
            graphicBackground->removeChild(waveNodes.at(i));
            
            //set new graphic corresponding next goal
            
            name = "POPULATION";
            if (collectionGoal->getResourceType() == Wood)
            {
                name = "WOOD";
            }
            else if (collectionGoal->getResourceType() == Mineral)
            {
                name = "MINERAL";
            }
            
            i = 0;
            while(waveNodes.at(i)->getName() != name)
            {
                i++;
            }
            
            graphicBackground->addChild(waveNodes.at(i), 1, 1);
            graphicLabel->setString(LocalizedString::create(name.c_str()));
            
            int numResources = 1;
            if (_isWood)
            {
                numResources++;
            }
            if (_isMineral)
            {
                numResources++;
            }
            graphicCounterLabel->setString("( " + to_string(i+1) + " / " + to_string(numResources) + " )");
            
            
            i = 0;
            while (i < GameLevel::getInstance()->getGoals().size() and GameLevel::getInstance()->getGoals().at(i)->getCompleted() == true)
            {
                i++;
            }
            if (GameLevel::getInstance()->getGoals().at(i)->getGoalType() == Resources)
            {
                auto goalCollection = (ResourcesGoal*)GameLevel::getInstance()->getGoals().at(i);
                int graphResource = PopulationRes;
                Color4B colorGoal(GameData::getInstance()->getPlayerColor());
                if (name == "WOOD")
                {
                    graphResource = Wood;
                    colorGoal = Color4B(103, 222, 31, 255);
                }
                else if (name == "MINERAL")
                {
                    graphResource = Mineral;
                    colorGoal = Color4B(241, 200, 10, 255);
                }
                if (goalCollection->getResourceType() == graphResource)
                {
                    float height;
                    if (graphResource == PopulationRes)
                    {
                        height = float(goalCollection->getGoalAmount())/float(GameLevel::getInstance()->getMaxAgents().at(0)) * graphicBackground->getContentSize().height;
                    }
                    else
                    {
                        height = float(goalCollection->getGoalAmount())/float(Agent::_resourcesPoolMax.at(goalCollection->getResourceType())) * graphicBackground->getContentSize().height;
                    }
                    
                    // Space the verticies out evenly across the screen for the wave.
                    float vertexHorizontalSpacing = graphicBackground->getContentSize().width / float(GameLevel::getInstance()->getGoals().back()->getMaxTime());
                    
                    goal->drawSegment(Vec2(vertexHorizontalSpacing, height), Vec2(vertexHorizontalSpacing * GameLevel::getInstance()->getGoals().at(GameLevel::getInstance()->getGoals().size() - 1)->getMaxTime(), height), 1, Color4F(Color4B(115, 148, 155, 200)));
                    
                    goal->drawSegment(Vec2(vertexHorizontalSpacing * goalCollection->getMinTime(), height), Vec2(vertexHorizontalSpacing * goalCollection->getMaxTime(), height), 3, Color4F(colorGoal));
                }
            }
        }
        else
        {
            auto nextArea = gameplayMap->getChildByTag(400+index+1);
            auto blink = Blink::create(2, 2);
            auto repeatBlink = RepeatForever::create(blink);
            nextArea->setColor(Color3B::WHITE);
            nextArea->runAction(repeatBlink);
            nextArea->setOpacity(255);
        }
        
        // X / Y GOALS COMPLETED
        
        
        auto currentGoal = (MenuItemImage*)(getChildByName("currentGoalMenu")->getChildByName("currentGoalImg"));
        currentGoal->setNormalImage(Sprite::create(getGoalIcon(goal)));
        ((ProgressTimer*)currentGoal->getChildByName("currentGoalProgress"))->setPercentage(0.0);
    }
}

float UIGameplayMap::sqrOfDistanceBetweenPoints(Point p1, Point p2)
{
    Point diff = p1 - p2;
    return diff.x * diff.x + diff.y * diff.y;
}

void UIGameplayMap::checkBackgroundLimitsInTheScreen(Point destPoint)
{
    Size winSize = Director::getInstance()->getVisibleSize();
    float gameplayMapBoundingBoxWidth = gameplayMap->getBoundingBox().size.width;
    float gameplayMapBoundingBoxHeight = gameplayMap->getBoundingBox().size.height;

    // LEFT-RIGHT
    if ((destPoint.x - gameplayMapBoundingBoxWidth / 2) > 0) {
        moveBackgroundLeft = false;
    }
    else {
        moveBackgroundLeft = true;
    }
    if ((destPoint.x + gameplayMapBoundingBoxWidth / 2) < winSize.width) {
        moveBackgroundRight = false;
    }
    else {
        moveBackgroundRight = true;
    }
    // UP-DOWN
    if ((destPoint.y - gameplayMapBoundingBoxHeight / 2) > 0) {
        moveBackgroundDown = false;
    }
    else {
        moveBackgroundDown = true;
    }
    if ((destPoint.y + gameplayMapBoundingBoxHeight / 2) < winSize.height) {
        moveBackgroundUp = false;
    }
    else {
        moveBackgroundUp = true;
    }
}

int UIGameplayMap::getValueAtGameplayMap(int rgb, int posx, int posy)
{
    Point loc(Point(posx, posy));
    loc.y = 320 - loc.y;
    return getValueAtGameplayMap(rgb, loc);
}

int UIGameplayMap::getValueAtGameplayMap(int rgb, Point pt)
{
    unsigned char* pixel;
    int x = 3;

    if (gameplayMapHotSpot.hasAlpha()) {
        x = 4;
    }
    pixel = dataGameplayMapHotSpot + ((int)pt.x + (int)pt.y * gameplayMapHotSpot.getWidth()) * x;

    switch (rgb) {
    case 0: {
        unsigned char r = *pixel;
        return (int)r;
        break;
    }
    case 1: {
        unsigned char g = *(pixel + 1);
        return (int)g;
        break;
    }
    case 2: {
        unsigned char b = *(pixel + 2);
        return (int)b;
        break;
    }
    case 3: {
        unsigned char a = *(pixel + 3);
        return (int)a;
        break;
    }
    default:
        break;
    }
    return 255;
}

void UIGameplayMap::initializeAgents(void)
{
    vector<list<Agent*> > agentsDomain = GameLevel::getInstance()->getAgents();
    for (size_t i = 0; i < agentsDomain.size(); i++)
    {
        std::list<Agent *> & aList = agentsDomain.at(i);
        aList.sort([](const Agent * a, const Agent * b) { return a->getLife() > b->getLife(); });
        
        for (list<Agent*>::iterator it = agentsDomain.at(i).begin(); it != agentsDomain.at(i).end(); it++)
        {
            Color3B c = GameData::getInstance()->getPlayerColor();
            Color4B color = Color4B(c.r, c.g, c.b, (*it)->getLife() * (255 / 175));
            drawAgentWithShadow(Point((*it)->getPosition().getX(), (*it)->getPosition().getY()), color);
        }
    }
    _agentsTexture.updateWithData(&(_agentsTextureData.at(0)), 0, 0, GameData::getInstance()->getResourcesWidth(), GameData::getInstance()->getResourcesHeight());
}

void UIGameplayMap::setAttributesToInitialAgents(void)
{
    GameLevel::getInstance()->setAttributesToInitialAgents();
}

void UIGameplayMap::createEndGameWindow(const LevelState & mode)
{
    _infoMap->setVisible(false);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto background = Sprite::create("gui/EndedGameBackground.png");
    background->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(background, 9);

    auto window = Sprite::create("gui/EndedGameWindow.png");
    window->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    window->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    string title;
    string text;

    if (mode == Success) {
        //success
        title = LocalizedString::create("LEVEL_COMPLETED");
        text = LocalizedString::create("CONGRATULATIONS");
        int starCount = 0;
        int score = GameData::getInstance()->getLevelScore(GameLevel::getInstance()->getNumLevel());
        while (starCount < 3) {
            if (starCount < score) {
                auto starFull = Sprite::create("gui/StarFull.png");
                starFull->setPosition(((starCount * 3) + 10) * window->getContentSize().width / 18,
                    5 * window->getContentSize().height / 10);
                window->addChild(starFull);
            }
            else {
                auto starEmpty = Sprite::create("gui/StarEmpty.png");
                starEmpty->setPosition(((starCount * 3) + 10) * window->getContentSize().width / 18,
                    5 * window->getContentSize().height / 10);
                window->addChild(starEmpty);
            }
            starCount++;
        }
        auto titleLabel = Label::createWithTTF(title, "fonts/BebasNeue.otf", 120 * GameData::getInstance()->getRaConversion());
        titleLabel->setColor(Color3B(255, 255, 255));
        titleLabel->setPosition(5 * window->getContentSize().width / 18, 5 * window->getContentSize().height / 10);
        window->addChild(titleLabel);
        // turn off tutorial for successful levels
        GameData::getInstance()->setTutorial(GameLevel::getInstance()->getNumLevel(), false);
    }
    else {
        //game over
        title = LocalizedString::create("GAME_OVER");

        if (mode == GoalFailBefore) {
            text = LocalizedString::create("GOAL_FAIL_BEFORE");
        }
        else if (mode == GoalFailAfter) {
            text = LocalizedString::create("GOAL_FAIL_AFTER");
        }
        else if (mode == NoAgentsLeft) {
            text = LocalizedString::create("ALL_BOTS_DIED");
        }

        auto titleLabel = Label::createWithTTF(title, "fonts/BebasNeue.otf", 100 * GameData::getInstance()->getRaConversion());
        titleLabel->setColor(Color3B(255, 255, 255));
        titleLabel->setPosition(9 * window->getContentSize().width / 18, 4 * window->getContentSize().height / 10);
        window->addChild(titleLabel);

        auto textLabel = Label::createWithTTF(text, "fonts/arial.ttf", 40 * GameData::getInstance()->getRaConversion());
        textLabel->setPosition(9 * window->getContentSize().width / 18, 6 * window->getContentSize().height / 10);
        textLabel->setAlignment(TextHAlignment::CENTER);
        window->addChild(textLabel);
        
        //failed vector
        GameData::getInstance()->setLevelFailedForHint(GameLevel::getInstance()->getNumLevel());
    }

    string space = " ";
    string lvl = LocalizedString::create("LEVEL") + space + to_string(GameLevel::getInstance()->getNumLevel());
    auto levelLabel = Label::createWithTTF(lvl, "fonts/BebasNeue.otf", 120 * GameData::getInstance()->getRaConversion());
    levelLabel->setColor(Color3B(85, 108, 117));
    levelLabel->setPosition(Vec2(4 * window->getContentSize().width / 18, 8.5 * window->getContentSize().height / 10));
    window->addChild(levelLabel);
    
    auto continueButton = MenuItemImage::create("gui/ProgressMapPlayButton.png", "gui/ProgressMapPlayButtonPressed.png", CC_CALLBACK_1(UIGameplayMap::menuBackCallback, this));
    continueButton->setPosition(14 * window->getContentSize().width / 18, 1.5 * window->getContentSize().height / 10);
    auto continueLabel = Label::createWithTTF(LocalizedString::create("CONTINUE"), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    continueLabel->setColor(Color3B(205, 202, 207));
    continueLabel->setPosition(1.1 * continueButton->getContentSize().width / 2, continueButton->getContentSize().height / 2);
    continueButton->addChild(continueLabel);
    auto continueMenu = Menu::createWithItem(continueButton);
    continueMenu->setPosition(0, 0);
    window->addChild(continueMenu);

    auto retryButton = MenuItemImage::create("gui/ProgressMapBackButton.png", "gui/ProgressMapBackButtonPressed.png", CC_CALLBACK_1(UIGameplayMap::retryOkCallback, this));
    retryButton->setPosition(4 * window->getContentSize().width / 18, 1.5 * window->getContentSize().height / 10);
    auto retryLabel = Label::createWithTTF(LocalizedString::create("RETRY"), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    retryLabel->setColor(Color3B(205, 202, 207));
    retryLabel->setPosition(retryButton->getContentSize().width / 2, retryButton->getContentSize().height / 2);
    retryButton->addChild(retryLabel);
    auto retryMenu = Menu::createWithItem(retryButton);
    retryMenu->setPosition(0, 0);
    window->addChild(retryMenu);
    
    if (GameData::getInstance()->getFirstTimeLevelCompleted() == GameLevel::getInstance()->getNumLevel())
    {
        retryButton->setVisible(false);
        continueButton->setPositionX(window->getContentSize().width / 2);
    }
    
    //delete wave nodes
    this->getChildByName("menuGraphic")->getChildByName("graphicBackground")->removeAllChildren();
    ((MenuItem*)this->getChildByName("timeMenu")->getChildByName("playToggle"))->setEnabled(false);
    
    this->addChild(window, 10);
}

void UIGameplayMap::createEndGameWindowLevel20(const LevelState & mode)
{
    _infoMap->setVisible(false);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto background = Sprite::create("gui/EndedGameBackground.png");
    background->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(background, 9);
    
    auto window = Sprite::create("gui/EndedGameWindow.png");
    window->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    window->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    string title;
    string text;
    
    if (mode == Success or mode == NoAgentsLeft) {
        //success
        title = LocalizedString::create("LEVEL_COMPLETED");
        int score = GameData::getInstance()->getLevelScore(GameLevel::getInstance()->getNumLevel());

        if (mode == NoAgentsLeft)
        {
            score = GameData::getInstance()->getLevelScore(GameLevel::getInstance()->getNumLevel() + 1);
            //set level 20 score to 1 if all bots died
            if (score == 0)
            {
                GameData::getInstance()->setLevelScore(GameLevel::getInstance()->getNumLevel() + 1, 1);
                score = 1;
            }
            text = LocalizedString::create("ALL_BOTS_LEVEL_20_DIED");
        }
        else
        {
            text = LocalizedString::create("CONGRATULATIONS");
        }
        int starCount = 0;
        while (starCount < 3) {
            if (starCount < score) {
                auto starFull = Sprite::create("gui/StarFull.png");
                starFull->setPosition(((starCount * 3) + 10) * window->getContentSize().width / 18,
                                      5 * window->getContentSize().height / 10);
                window->addChild(starFull);
            }
            else {
                auto starEmpty = Sprite::create("gui/StarEmpty.png");
                starEmpty->setPosition(((starCount * 3) + 10) * window->getContentSize().width / 18,
                                       5 * window->getContentSize().height / 10);
                window->addChild(starEmpty);
            }
            starCount++;
        }
        auto titleLabel = Label::createWithTTF(title, "fonts/BebasNeue.otf", 120 * GameData::getInstance()->getRaConversion());
        titleLabel->setColor(Color3B(255, 255, 255));
        titleLabel->setPosition(5 * window->getContentSize().width / 18, 5 * window->getContentSize().height / 10);
        window->addChild(titleLabel);
        // turn off tutorial for successful levels
        GameData::getInstance()->setTutorial(GameLevel::getInstance()->getNumLevel(), false);
    }
    else {
        //game over
        title = LocalizedString::create("GAME_OVER");
        
        if (mode == GoalFailBefore) {
            text = LocalizedString::create("GOAL_FAIL_BEFORE");
        }
        else if (mode == GoalFailAfter) {
            text = LocalizedString::create("GOAL_FAIL_AFTER");
        }
        
        auto titleLabel = Label::createWithTTF(title, "fonts/BebasNeue.otf", 100 * GameData::getInstance()->getRaConversion());
        titleLabel->setColor(Color3B(255, 255, 255));
        titleLabel->setPosition(9 * window->getContentSize().width / 18, 4 * window->getContentSize().height / 10);
        window->addChild(titleLabel);
        
        auto textLabel = Label::createWithTTF(text, "fonts/arial.ttf", 40 * GameData::getInstance()->getRaConversion());
        textLabel->setPosition(9 * window->getContentSize().width / 18, 6 * window->getContentSize().height / 10);
        textLabel->setAlignment(TextHAlignment::CENTER);
        window->addChild(textLabel);
        
        //failed vector
        GameData::getInstance()->setLevelFailedForHint(GameLevel::getInstance()->getNumLevel());
    }
    
    string space = " ";
    string lvl = LocalizedString::create("LEVEL") + space + to_string(GameLevel::getInstance()->getNumLevel());
    auto levelLabel = Label::createWithTTF(lvl, "fonts/BebasNeue.otf", 120 * GameData::getInstance()->getRaConversion());
    levelLabel->setColor(Color3B(85, 108, 117));
    levelLabel->setPosition(Vec2(4 * window->getContentSize().width / 18, 8.5 * window->getContentSize().height / 10));
    window->addChild(levelLabel);
    
    auto continueButton = MenuItemImage::create("gui/ProgressMapPlayButton.png", "gui/ProgressMapPlayButtonPressed.png", CC_CALLBACK_1(UIGameplayMap::menuBackCallback, this));
    continueButton->setPosition(14 * window->getContentSize().width / 18, 1.5 * window->getContentSize().height / 10);
    auto continueLabel = Label::createWithTTF(LocalizedString::create("CONTINUE"), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    continueLabel->setColor(Color3B(205, 202, 207));
    continueLabel->setPosition(1.1*continueButton->getContentSize().width / 2, continueButton->getContentSize().height / 2);
    continueButton->addChild(continueLabel);
    auto continueMenu = Menu::createWithItem(continueButton);
    continueMenu->setPosition(0, 0);
    window->addChild(continueMenu);
    
    auto retryButton = MenuItemImage::create("gui/ProgressMapBackButton.png", "gui/ProgressMapBackButtonPressed.png", CC_CALLBACK_1(UIGameplayMap::retryOkCallback, this));
    retryButton->setPosition(4 * window->getContentSize().width / 18, 1.5 * window->getContentSize().height / 10);
    auto retryLabel = Label::createWithTTF(LocalizedString::create("RETRY"), "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
    retryLabel->setColor(Color3B(205, 202, 207));
    retryLabel->setPosition(retryButton->getContentSize().width / 2, retryButton->getContentSize().height / 2);
    retryButton->addChild(retryLabel);
    auto retryMenu = Menu::createWithItem(retryButton);
    retryMenu->setPosition(0, 0);
    window->addChild(retryMenu);
    
    if (GameData::getInstance()->getFirstTimeLevelCompleted() == GameLevel::getInstance()->getNumLevel() or GameData::getInstance()->getFirstTimeLevelCompleted() == 21)
    {
        retryButton->setVisible(false);
        continueButton->setPositionX(window->getContentSize().width / 2);
    }
    
    //delete wave nodes
    this->getChildByName("menuGraphic")->getChildByName("graphicBackground")->removeAllChildren();
    ((MenuItem*)this->getChildByName("timeMenu")->getChildByName("playToggle"))->setEnabled(false);
    
    this->addChild(window, 10);

}


void UIGameplayMap::createAchievementWindow(void)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/achievement.mp3");
    }
    Size visibleSize = Director::getInstance()->getVisibleSize();
    int numCompletedAchievements = (int)GameLevel::getInstance()->getCompletedAchievements().size();
    auto window = MenuItemImage::create("gui/AchievementsBackground.png", "gui/AchievementsBackground.png",
                                        CC_CALLBACK_1(UIGameplayMap::hideAchievementWindowCallback, this));
    window->setPosition(Vec2(visibleSize.width /2, visibleSize.height + window->getContentSize().height));
    window->setScale(GameData::getInstance()->getRaWConversion() * 0.6,
                     GameData::getInstance()->getRaHConversion() * (0.3 + (0.1 * numCompletedAchievements)));
    window->setName("achievementWindow");
    
    std::stringstream ach;
    ach << LocalizedString::create("YOU_UNLOCKED") + " " + to_string(numCompletedAchievements) + " ";
    if (numCompletedAchievements > 1)
    {
        ach << LocalizedString::create("ACHS_UNLOCK");
    }
    else
    {
        ach << LocalizedString::create("ACH_UNLOCK");
    }

    auto titleLabel = Label::createWithTTF(ach.str(), "fonts/BebasNeue.otf", 100 * GameData::getInstance()->getRaConversion());
    titleLabel->setColor(Color3B(255, 255, 255));
    titleLabel->setAnchorPoint(Vec2(0.5, 0));
    titleLabel->setPosition(Vec2(window->getContentSize().width / 2, window->getContentSize().height / 2 - titleLabel->getContentSize().height * 1.1));
    window->addChild(titleLabel);
    
    for (int i = 0; i < numCompletedAchievements; i++)
    {
        string titleAch = LocalizedString::create(("TITLE_LVL" + GameLevel::getInstance()->getCompletedAchievements().at(i)).c_str(), "achievements");
        string descrAch = LocalizedString::create(("DESCR_LVL" + GameLevel::getInstance()->getCompletedAchievements().at(i)).c_str(), "achievements");
        auto labelAch = Label::createWithTTF(titleAch + ": " + descrAch, "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
        labelAch->setColor(Color3B(85, 108, 117));
        labelAch->setPosition(Vec2(window->getContentSize().width / 2,
                                   (titleLabel->getPositionY() - titleLabel->getContentSize().height) - (labelAch->getContentSize().height * 1.2 * i)));
        window->addChild(labelAch);
    }
    
    auto menuWindow = Menu::create(window, NULL);
    menuWindow->setPosition(Vec2(0, 0));
    this->addChild(menuWindow, 10);
    window->runAction(EaseBackOut::create(MoveTo::create(1, Vec2(visibleSize.width /2, visibleSize.height))));
}

void UIGameplayMap::createInGameAchievementWindow(Achievement * ach)
{
    if (GameData::getInstance()->getSFX() == true) {
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/achievement.mp3");
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto window = MenuItemImage::create("gui/AchievementsBackground.png", "gui/AchievementsBackground.png",
                                        CC_CALLBACK_1(UIGameplayMap::hideAchievementWindowCallback, this));
    window->setPosition(Vec2(visibleSize.width /2, visibleSize.height + window->getContentSize().height));
    window->setScale(GameData::getInstance()->getRaWConversion() * 0.6,
                     GameData::getInstance()->getRaHConversion() * 0.4);
    window->setName("achievementWindow");
    
    auto titleLabel = Label::createWithTTF(LocalizedString::create("YOU_UNLOCKED") + " 1 " + LocalizedString::create("ACH_UNLOCK"), "fonts/BebasNeue.otf",
                                           100 * GameData::getInstance()->getRaConversion());
    titleLabel->setColor(Color3B(255, 255, 255));
    titleLabel->setAnchorPoint(Vec2(0.5, 0));
    titleLabel->setPosition(Vec2(window->getContentSize().width / 2, window->getContentSize().height / 2 - titleLabel->getContentSize().height * 1.1));
    window->addChild(titleLabel);
    
    string key = to_string(ach->getLevel())+"_"+ach->getGoalType();
    
        string titleAch = LocalizedString::create(("TITLE_LVL" + key).c_str(), "achievements");
        string descrAch = LocalizedString::create(("DESCR_LVL" + key).c_str(), "achievements");
        auto labelAch = Label::createWithTTF(titleAch + ": " + descrAch, "fonts/BebasNeue.otf", 60 * GameData::getInstance()->getRaConversion());
        labelAch->setColor(Color3B(85, 108, 117));
        labelAch->setPosition(Vec2(window->getContentSize().width / 2,
                                   (titleLabel->getPositionY() - titleLabel->getContentSize().height) - (labelAch->getContentSize().height * 1.2)));
        window->addChild(labelAch);
    
    auto menuWindow = Menu::create(window, NULL);
    menuWindow->setPosition(Vec2(0, 0));
    this->addChild(menuWindow, 10);
    window->runAction(EaseBackOut::create(MoveTo::create(1, Vec2(visibleSize.width /2, visibleSize.height))));
}

void UIGameplayMap::updateAgents(void)
{

    pthread_mutex_lock(&gameLevelMutex);
    vector<list<Agent*> > agentsDomain = GameLevel::getInstance()->getAgents();

    Color4B transparent = Color4B::WHITE;
    transparent.a = 0;
    for (size_t i = 0; i < GameLevel::getInstance()->getDeletedAgents().size(); i++)
    {
        drawAgentWithShadow(GameLevel::getInstance()->getDeletedAgents().at(i), transparent, transparent);
    }

    Color3B agentColorPlayer = GameData::getInstance()->getPlayerColor();
    for (int i = int(agentsDomain.size()) - 1; i >= 0 ; i--)
    {
        std::list<Agent *> & aList = agentsDomain.at(i);
        aList.sort([](const Agent * a, const Agent * b) { return a->getLife() < b->getLife(); });
 
        for (list<Agent*>::iterator it = agentsDomain.at(i).begin(); it != agentsDomain.at(i).end(); ++it)
        {
            Color4B color;
            switch ((*it)->getType())
            {
                case 1:
                    color = Color4B(255, 0, 0, (*it)->getLife() * (255 / 175));
                    break;
                case 2:
                    color = Color4B(255, 252, 0, (*it)->getLife() * (255 / 175));
                    break;
                case 3:
                    color = Color4B(19, 1, 253, (*it)->getLife() * (255 / 175));
                    break;
                default:
                    color = Color4B(agentColorPlayer.r, agentColorPlayer.g, agentColorPlayer.b, (*it)->getLife() * (255 / 175));
            }
            drawAgentWithShadow(Point((*it)->getPosition().getX(), (*it)->getPosition().getY()), color);
        }
    }

    _agentsTexture.updateWithData(&(_agentsTextureData.at(0)), 0, 0, GameData::getInstance()->getResourcesWidth(), GameData::getInstance()->getResourcesHeight());
    if (resourcesMap) {
        //restore map
        for (int i = 0; i < GameLevel::getInstance()->getRestored().size(); i++)
        {
            drawExploitedMap(GameLevel::getInstance()->getRestored().at(i), Color4B(255,255,255,255));
        }
        GameLevel::getInstance()->clearRestored();
       
        for (int i = 0; i < GameLevel::getInstance()->getTerraformedVector().size(); i++)
        {
            drawExploitedMap(GameLevel::getInstance()->getTerraformedVector().at(i), Color4B(0, 0, 0, 0));
        
        }
        GameLevel::getInstance()->clearTerraformedVector();
        
        for (int i = 0; i < GameLevel::getInstance()->getNewDepletedVector().size(); i++)
        {
            drawExploitedMap(GameLevel::getInstance()->getNewDepletedVector().at(i), Color4B(0,0,0,0));
        }
        GameLevel::getInstance()->clearNewDepletedVector();

        _exploitedMapTexture.updateWithData(&(_exploitedMapTextureData.at(0)), 0, 0, GameData::getInstance()->getResourcesWidth(), GameData::getInstance()->getResourcesHeight());
    }
}


void UIGameplayMap::drawAgentWithShadow(const Point & pos, const Color4B & colour, const Color4B & shadow)
{
    int x = (int)(pos.x * GameData::getInstance()->getRowDrawAgentPrecalc());
    int y = (int)(GameData::getInstance()->getColumnOffsetDrawAgentPrecalc() + ((pos.y) * GameData::getInstance()->getColumnDrawAgentPrecalc()));
    int position = x + ((GameData::getInstance()->getResourcesHeight() - y) * GameData::getInstance()->getResourcesWidth());

    int xShadow = x - int(GameLevel::getInstance()->getAgentPixelSize()*2.0f/3.0f);
    int yShadow = y - int(GameLevel::getInstance()->getAgentPixelSize()*2.0f/3.0f);
    if(xShadow>0 && yShadow>0)
    {
        int positionShadow = xShadow + ((GameData::getInstance()->getResourcesHeight() - yShadow) * GameData::getInstance()->getResourcesWidth());
        drawAgent(positionShadow, Color4B(shadow.r, shadow.g, shadow.b, colour.a));
    }
    drawAgent(position, colour);
}

void UIGameplayMap::drawAgent(int position, const Color4B & colour)
{
    /*
    int x = (int)(pos.x * GameData::getInstance()->getRowDrawAgentPrecalc());
    int y = (int)(GameData::getInstance()->getColumnOffsetDrawAgentPrecalc() + ((pos.y) * GameData::getInstance()->getColumnDrawAgentPrecalc()));
    int position = x + ((GameData::getInstance()->getResourcesHeight() - y) * GameData::getInstance()->getResourcesWidth());
    */

    int k = -GameData::getInstance()->getResourcesWidth() * GameLevel::getInstance()->getAgentPixelSize();
    while (k <= GameData::getInstance()->getResourcesWidth() * GameLevel::getInstance()->getAgentPixelSize())
    {
        for (int j = -GameLevel::getInstance()->getAgentPixelSize(); j <= GameLevel::getInstance()->getAgentPixelSize(); j++)
        {
            if (k == -(GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize())) and (j == - GameLevel::getInstance()->getAgentPixelSize() or j == GameLevel::getInstance()->getAgentPixelSize()))
            {
                continue;
            }
            else if (k == GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize()) and (j == - GameLevel::getInstance()->getAgentPixelSize() or j == GameLevel::getInstance()->getAgentPixelSize()))
            {
                continue;
            }
            _agentsTextureData.at(position + j + k) = colour;
        }
        k += GameData::getInstance()->getResourcesWidth();
    }
}

void UIGameplayMap::drawExploitedMap(const Point & pos, const Color4B & colour, int geometry)
{
    if(!resourcesMap)
    {
        return;
    }
    /*int x = (int)(pos.x * float(GameData::getInstance()->getResourcesWidth() / 480.0));
    int y = (int)(float((GameData::getInstance()->getResourcesHeight() - GameData::getInstance()->getResourcesMargin()) / 2.0) + ((pos.y) * float(GameData::getInstance()->getResourcesMargin() / 320.0)));
    int position = x + ((GameData::getInstance()->getResourcesHeight() - y) * GameData::getInstance()->getResourcesWidth());*/
    int x = (int)(pos.x * GameData::getInstance()->getRowDrawAgentPrecalc());
    int y = (int)(GameData::getInstance()->getColumnOffsetDrawAgentPrecalc() + ((pos.y) * GameData::getInstance()->getColumnDrawAgentPrecalc()));
    int position = x + ((GameData::getInstance()->getResourcesHeight() - y) * GameData::getInstance()->getResourcesWidth());    
    switch (geometry) {
    default:
        int k = -(GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize()+2));
            
        while (k <= GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize()+2)) {
            for (int j = - GameLevel::getInstance()->getAgentPixelSize()-2; j < GameLevel::getInstance()->getAgentPixelSize() + 3; j++) {
                //circle
                if (k == -(GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize()+2)) and (j == - GameLevel::getInstance()->getAgentPixelSize()-2 or j == GameLevel::getInstance()->getAgentPixelSize() + 2))
                {
                        continue;
                }
                else if (k == GameData::getInstance()->getResourcesWidth()* (GameLevel::getInstance()->getAgentPixelSize()+2) and (j == - GameLevel::getInstance()->getAgentPixelSize()-2 or j == GameLevel::getInstance()->getAgentPixelSize() + 2))
                {
                        continue;
                }
                else
                {
                if (colour.r == 0)
                {
                    if(_exploitedMapTextureData.at(position + j + k).a == 255)
                    {
                        _exploitedMapTextureData.at(position + j + k).a = 1;
                    }
                }
                else
                {
                    if(_exploitedMapTextureData.at(position + j + k).a == 1)
                    {
                        _exploitedMapTextureData.at(position + j + k).a = 255;
                    }
                }
                }
            }
            k += GameData::getInstance()->getResourcesWidth();
        }
        break;
    }
}

void UIGameplayMap::createTutorialGUI()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    // TUTORIAL MESSAGES
    auto messageLabel = Label::createWithTTF("no message", "fonts/arial.ttf", 40 * GameData::getInstance()->getRaConversion());
    messageLabel->setName("tutorial");
    messageLabel->setColor(Color3B(230, 230, 230));
    messageLabel->enableShadow();
    messageLabel->setMaxLineWidth(300);
    messageLabel->setVisible(false);
    messageLabel->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    
    auto messageNextLabel = Label::createWithTTF("", "fonts/arial.ttf", 40 * GameData::getInstance()->getRaConversion());
    messageNextLabel->setName("tutorialNext");
    messageNextLabel->setColor(Color3B(210, 210, 210));
    messageNextLabel->setMaxLineWidth(300);
    messageNextLabel->setVisible(false);
    messageNextLabel->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());

    auto labelBorder = DrawNode::create();
    labelBorder->setName("tutorialBorder");
    labelBorder->setVisible(false);

    auto tutorialWindow = MenuItemToggle::createWithCallback(CC_CALLBACK_1(UIGameplayMap::skipTutorial, this), MenuItemImage::create("gui/TutorialWindow.png", "gui/TutorialWindow.png"), MenuItemImage::create("gui/TutorialWindowPressed.png", "gui/TutorialWindowPressed.png"), nullptr);
    tutorialWindow->setAnchorPoint(Vec2(0, 0.5));
    tutorialWindow->setPosition(Vec2(5 * visibleSize.width / 204, 125 * visibleSize.height / 155));
    tutorialWindow->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    tutorialWindow->setName("tutorialWindow");
    
    auto skipWindow = MenuItemImage::create("gui/TutorialWindowSkip.png", "gui/TutorialWindowSkip.png", CC_CALLBACK_1(UIGameplayMap::skipTutorialConfirm, this));
    skipWindow->setName("skipWindow");
    skipWindow->setAnchorPoint(Vec2(0, 0.5));
    skipWindow->setPosition(Vec2(tutorialWindow->getPositionX() + tutorialWindow->getBoundingBox().size.width / 1.05, tutorialWindow->getPositionY() + skipWindow->getBoundingBox().size.height / 1.5));
    skipWindow->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    skipWindow->setOpacity(0);
    skipWindow->setVisible(false);
    
    auto skipLabel = Label::createWithTTF(LocalizedString::create("SKIP"), "fonts/BebasNeue.otf", 40 * GameData::getInstance()->getRaConversion());
    skipLabel->setPosition(Vec2(skipWindow->getContentSize().width / 2, skipWindow->getContentSize().height / 2));
    skipLabel->setColor(Color3B(139, 146, 154));
    skipWindow->addChild(skipLabel);
    
    auto menuTutorialWindow = Menu::create(tutorialWindow, skipWindow, nullptr);
    menuTutorialWindow->setName("menuTutorialWindow");
    menuTutorialWindow->setPosition(Vec2::ZERO);
    menuTutorialWindow->setVisible(false);
    
    auto tutorialTitle = Label::createWithTTF("TUTORIAL", "fonts/BebasNeue.otf", 90 * GameData::getInstance()->getRaConversion());
    tutorialTitle->setName("tutorialTitle");
    tutorialTitle->setColor(Color3B(139, 146, 154));
    tutorialTitle->setPosition(Vec2(3.3 * tutorialWindow->getContentSize().width / 7, 2.3 * tutorialWindow->getContentSize().height / 5));
    tutorialWindow->addChild(tutorialTitle, 5);
    
    auto tutorialImage = Sprite::create();
    tutorialImage->setName("tutorialImage");
    tutorialImage->setVisible(false);
    tutorialImage->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
    
    // add first the border to draw it first
    this->addChild(labelBorder);
    this->addChild(messageLabel);
    this->addChild(messageNextLabel);
    this->addChild(menuTutorialWindow);
    this->addChild(tutorialImage, 100);
}

void UIGameplayMap::update(float delta)
{
    if (GameLevel::getInstance()->getFinishedGame() == Running) {
        if (GameLevel::getInstance()->paint == true and GameLevel::getInstance()->ended == false)
        {
            if(_tutorial)
            {
                if(_tutorial->checkNextMessage())
                {
                    setMessage(_tutorial->getCurrentMessage());
                }
                else if(_tutorial->getCurrentMessage() && _tutorial->getCurrentMessage()->meetsPostCondition())
                {
                    _tutorial->removeCurrentMessage();
                    this->getChildByName("tutorial")->setVisible(false);
                    this->getChildByName("tutorialNext")->setVisible(false);
                    this->getChildByName("tutorialBorder")->setVisible(false);
                    this->getChildByName("tutorialImage")->stopAllActions();
                    this->getChildByName("tutorialImage")->setVisible(false);

                    if(_tutorial->isFinished())
                    {
                        _eventDispatcher->removeEventListener(_listenerTutorial);
                        disableTutorialGUI();
                        pauseGame();
                    }
                }
            }
       
            updateAgents();

            MenuItemImage * currentGoal = (MenuItemImage*)(getChildByName("currentGoalMenu")->getChildByName("currentGoalImg"));
            updateLegend(currentGoal->isSelected());

            // TODO everything stopped if _message?
            updateWave(0, int(GameLevel::getInstance()->getAgents().at(0).size()), GameLevel::getInstance()->getMaxAgents().at(0), Color4B(179, 205, 221, 255));
            
            if (_isWood)
            {
                updateWave(1, Agent::_resourcesPool.at(0).at(Wood), Agent::_resourcesPoolMax.at(Wood), Color4B(103, 222, 31, 255));
            }
            if (_isMineral)
            {
                // 2 resources
                if(_isWood)
                {
                    updateWave(2, Agent::_resourcesPool.at(0).at(Mineral), Agent::_resourcesPoolMax.at(Mineral), Color4B(241, 200, 10, 255));
                }
                // just mineral
                else
                {
                    updateWave(1, Agent::_resourcesPool.at(0).at(Mineral), Agent::_resourcesPoolMax.at(Mineral), Color4B(241, 200, 10, 255));
                }
            }
            
            //UPDATE PROGRESS RESOURCE GOAL
            int i = 0;
            while (i < GameLevel::getInstance()->getGoals().size() and GameLevel::getInstance()->getGoals().at(i)->getCompleted() == true)
            {
                i++;
            }
            if (GameLevel::getInstance()->getGoals().at(i)->getGoalType() == Resources)
            {
                auto resGoal = ((ResourcesGoal*)GameLevel::getInstance()->getGoals().at(i));
                MenuItemImage * currentGoal = (MenuItemImage*)(getChildByName("currentGoalMenu")->getChildByName("currentGoalImg"));
                if (resGoal->getResourceType() == PopulationRes)
                {
                    ((ProgressTimer*)currentGoal->getChildByName("currentGoalProgress"))->setPercentage(float(GameLevel::getInstance()->getAgents().at(0).size()) / float(resGoal->getGoalAmount()) * 100.0);
                }
                else
                {
                    ((ProgressTimer*)currentGoal->getChildByName("currentGoalProgress"))->setPercentage(float(Agent::_resourcesPool.at(0).at(resGoal->getResourceType())) / float(resGoal->getGoalAmount()) * 100.0);
                }
            }
            
            pthread_mutex_unlock(&gameLevelMutex);
        }
        if (GameLevel::getInstance()->getGoals().empty() == false)
        {
            float value = std::min(100.0f, Timing::getInstance()->getTimeStep()/float(GameLevel::getInstance()->getGoals().back()->getMaxTime())*100.0f);
            // min with 100 and percentage because Timing thread is faster than gamelevel thread (checking goals) and this thread (painting). it will add some diff to _timeStep before checking for fail
            timeBar->setPercentage(value);
            if (value > 99.4)
            {
                timeBorderBar->getChildByName("degradateTime")->setVisible(false);
            }

            timeBorderBar->getChildByName("degradateTime")->setPositionX(timeBorderBar->getContentSize().width * (value/100.0f));
        }

        for (size_t i = 0; i < powerButtons.size(); i++) {
            powerButtons.at(i)->update(delta);
        }

        if (std::atoi(evolutionPointsLabel->getString().c_str()) != GameLevel::getInstance()->getEvolutionPoints())
        {
            evolutionPointsLabel->setString(to_string(GameLevel::getInstance()->getEvolutionPoints()));
            updateAttributesButtons();
        }
        if (drawInGameAchievementWindow)
        {
            createInGameAchievementWindow(GameLevel::getInstance()->getInGameAchievement());
            drawInGameAchievementWindow = false;
            GameLevel::getInstance()->setInGameAchievement(nullptr);
        }
    }
    else if (GameLevel::getInstance()->getFinishedGame() != Running and endGameWindowPainted == false and GameLevel::getInstance()->ended == true)
    {
        updateAgents();
        if (GameLevel::getInstance()->getGoals().empty() == false)
        {
            float value = std::min(100.0f, Timing::getInstance()->getTimeStep()/float(GameLevel::getInstance()->getGoals().back()->getMaxTime())*100.0f);
            timeBar->setPercentage(value);
        }

        for (size_t i = 0; i < powerButtons.size(); i++) {
            powerButtons.at(i)->update(delta);
        }
        pthread_mutex_unlock(&gameLevelMutex);


        evolutionPointsLabel->setString(to_string(GameLevel::getInstance()->getEvolutionPoints()));

        if (GameLevel::getInstance()->getNumLevel() == 20)
        {
            createEndGameWindowLevel20(GameLevel::getInstance()->getFinishedGame());

        }
        else
        {
            createEndGameWindow(GameLevel::getInstance()->getFinishedGame());
        }
        
        //HAS COMPLETED ANY ACHIEVEMENT
        if (GameLevel::getInstance()->getCompletedAchievements().size() > 0)
        {
            createAchievementWindow();
        }
        
        endGameWindowPainted = true;
    }
}

void UIGameplayMap::setMessage( const Message * message )
{
    restoreGameplayMap();
    pauseGame();

    _message = message;

    Label * label = (Label*)(this->getChildByName("tutorial"));
    Label * nextLabel = (Label*)(this->getChildByName("tutorialNext"));
    Sprite* image = (Sprite*)(this->getChildByName("tutorialImage"));

    label->setString(_message->text());
    Size visibleSize = Director::getInstance()->getVisibleSize();
    label->setMaxLineWidth(_message->lineWidth()*visibleSize.width);
    nextLabel->setMaxLineWidth(_message->lineWidth()*visibleSize.width);
    Vec2 position = Vec2(visibleSize.width*_message->pos().x, visibleSize.height*_message->pos().y);
    label->setPosition(position);

    auto labelBorder = (DrawNode*)(this->getChildByName("tutorialBorder"));
    labelBorder->clear();
    if(_message->text()!="")
    {
        // condition to close the message
        if(message->getPostCondition()=="tapButton")
        {
            nextLabel->setString(LocalizedString::create("NEXT_MESSAGE_BUTTON", "tutorial"));
        }
        else if(message->getPostCondition()=="spot")
        {
            nextLabel->setString(LocalizedString::create("NEXT_MESSAGE_SPOT", "tutorial"));
        }
        else
        {
            nextLabel->setString(LocalizedString::create("NEXT_MESSAGE_TAP", "tutorial"));
        }

        // if message shows an image in a particular spot
        if(message->getSpot()!=nullptr)
        {     
            const Spot * spot = message->getSpot();
            image->setTexture(spot->_image+".png");
            image->setScale(GameData::getInstance()->getRaWConversion(), GameData::getInstance()->getRaHConversion());
            image->setVisible(true);
            image->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));

            // locate the spot in the exact position of the button
            if(message->getPostCondition()=="tapButton")
            { 
                std::string buttonName = _tutorial->getCurrentMessage()->getPostConditionButtonTap();
                Node * parent = this;
                std::size_t pos;
                std::string delimiter = "/";
                std::string token;
                while ((pos = buttonName.find(delimiter)) != std::string::npos)
                {
                    token = buttonName.substr(0, pos);
                    parent = parent->getChildByName(token);
                    buttonName.erase(0, pos + delimiter.length());
                }
                
                token = buttonName.substr(0, pos);
                std::string buttonPressed = token;
                
                //attribute translation
                if (token.substr(0, 4) == "plus")
                {
                    buttonPressed = "plus";
                    token = "plus" + LocalizedString::create(token.substr(4).c_str());
                }
                Node * button = parent->getChildByName(token);
                if(button)
                {
                    auto screenPosition = parent->convertToWorldSpace(button->getPosition());
                    image->runAction(MoveTo::create(1.0f, Vec2(screenPosition.x, screenPosition.y)));
                }

            }
            else
            {
                image->runAction(MoveTo::create(1.0f, Vec2(visibleSize.width*spot->_centerX, visibleSize.height*spot->_centerY)));
            }
            image->runAction(RepeatForever::create(Sequence::create(DelayTime::create(1.0f), FadeTo::create(1, 120), FadeTo::create(1, 255), nullptr)));
        }
        
        const Rect & contents = label->getBoundingBox();
        const Rect & ownContents = nextLabel->getBoundingBox();
        nextLabel->setPosition(Vec2(contents.getMaxX()-(ownContents.size.width/2), contents.getMinY()-0.02f*label->getContentSize().height-(ownContents.size.height/2)));
        
        label->setVisible(true);    
        labelBorder->setVisible(true);   
        nextLabel->setVisible(true);

    }

    float marginWidth = 0.02f*label->getContentSize().width;
    float marginHeight = 0.02f*label->getContentSize().height;
    Vec2 margin(marginWidth, marginHeight);

    Vec2 origin(label->getBoundingBox().origin - margin);
    Vec2 end(label->getBoundingBox().origin + label->getBoundingBox().size + margin);
    labelBorder->drawSolidRect(origin, end, Color4F(0.77f, 0.6f, 0.95f, 0.3f));
    labelBorder->drawRect(origin, end, Color4F(0.66f, 0.43f, 0.97f, 1.0f));

    auto pauseDarkBackground = this->getChildByName("pauseDarkBackground");
    pauseDarkBackground->setVisible(false);
}

void UIGameplayMap::updateWave(int index, int variable, int maxVariable, Color4B color)
{
    auto graphicBackground = (MenuItem*)this->getChildByName("menuGraphic")->getChildByName("graphicBackground");
    
    float height = float(variable)/float(maxVariable) * graphicBackground->getContentSize().height * GameData::getInstance()->getRaHConversion();

    // Space the verticies out evenly across the screen for the wave.
    float vertexHorizontalSpacing = graphicBackground->getContentSize().width * GameData::getInstance()->getRaWConversion()/ float(GameLevel::getInstance()->getGoals().back()->getMaxTime());
    
    // Used to increment to the next vertexX position.
    float currentWaveVertX = this->convertToWorldSpace(graphicBackground->getPosition()).x - (graphicBackground->getContentSize().width * GameData::getInstance()->getRaWConversion() / 2);

    WavePoint w;
    w.x = currentWaveVertX + vertexHorizontalSpacing * Timing::getInstance()->getTimeStep();
    w.y = height + this->convertToWorldSpace(graphicBackground->getPosition()).y - (graphicBackground->getContentSize().height * GameData::getInstance()->getRaHConversion() / 2);
    w.z = 0;
    waveNodes.at(index)->addToDynamicVerts3D(w, color);
}

void UIGameplayMap::restoreGameplayMap(void)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    gameplayMap->runAction(Spawn::create(ScaleTo::create(0.3, GameData::getInstance()->getRaWConversion(),
                                                         GameData::getInstance()->getRaHConversion()),
                                         MoveTo::create(0.3, Vec2(visibleSize.width / 2, visibleSize.height / 2)),
                                         NULL));
}

void UIGameplayMap::updateAttributesButtons(void)
{
    Menu* attributesMenu = (Menu*)this->getChildByName("bottomFrame")->getChildByTag(100000);
    for (size_t i = 0; i < GameLevel::getInstance()->getModifiableAttr().size(); i++)
    {
        if(GameLevel::getInstance()->getModifiableAttr().at(i)==-1)
        {
            continue;
        }
        MenuItem* plus = (MenuItem*) attributesMenu->getChildByTag(int(i) + 50);
        if (GameLevel::getInstance()->getAttributeCost(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) > GameLevel::getInstance()->getEvolutionPoints())
        {
            plus->setEnabled(false);
        }
        else
        {
            plus->setEnabled(true);
        }
        if (GameLevel::getInstance()->getAgentAttribute(GameLevel::getInstance()->getCurrentAgentType(), GameLevel::getInstance()->getModifiableAttr().at(i)) >= 5)
        {
            plus->setEnabled(false);
        }
    }
}

void UIGameplayMap::updateLegend(bool visible)
{
    if(!visible && _infoMap->isVisible())
    {
        _infoMap->setVisible(visible);
        getChildByName("legend")->setVisible(visible);
        return;
    }
    
    // update position
    Size visibleSize = Director::getInstance()->getVisibleSize();
    getChildByName("legend")->setPosition(0.05f*visibleSize.width, 0.25f*visibleSize.height);

    // if visible==true and not currently visible
    if(!_infoMap->isVisible())
    {
        if (visible == true and GameData::getInstance()->getSFX() == true) {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("audio/click2.mp3");
        }
        _infoMap->setVisible(visible);
        getChildByName("legend")->setVisible(visible);
    }
}
void UIGameplayMap::createLegendEntries()
{
    auto node = Node::create();
    node->setName("legend");

    int heightOffset = 0.0f;
    for(size_t i=0;i <GameLevel::getInstance()->getLegendSize(); i++)
    {
        auto newEntry = Label::createWithTTF(LocalizedString::create(GameLevel::getInstance()->getLegendName(int(i)).c_str()), "fonts/arial.ttf", 45 * GameData::getInstance()->getRaConversion());
        newEntry->setAnchorPoint(Vec2(0.0, 0.0));
        newEntry->setTextColor(GameLevel::getInstance()->getLegendColor(int(i)));
        newEntry->setPosition(0.0f, heightOffset);
        newEntry->enableOutline(Color4B(50,50,50,255), 2);

        heightOffset += newEntry->getContentSize().height;
        node->addChild(newEntry);
    }
    this->addChild(node);
    updateLegend(false);
}

