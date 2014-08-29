//
//  GameLevel.h
//  simulplay
//
//  Created by Guillem Laborda on 18/08/14.
//
//

#ifndef __simulplay__GameLevel__
#define __simulplay__GameLevel__

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "Agent.h"

class UIGameplayMap;

using namespace std;

class GameLevel {
public:
    static GameLevel* getInstance();

    UIGameplayMap* getUIGameplayMap(void);
    void setUIGameplayMap(UIGameplayMap* gmplmap);
    double getCurrentTime(void);
    void setCurrentTime(double time);
    int getNumLevel(void);
    void setNumLevel(int lvl);
    int getAgentAttribute(string key);
    void setAgentAttributes(string key, int value);
    vector<Agent*> getAgents(void);
    void setAgents(vector<Agent*> ags);
    int getAddedAgents(void);
    void setAddedAgents(int i);
    vector<int> getDeletedAgents(void);
    void setDeletedAgents(vector<int> v);
    int getFinishedGame(void);
    void setFinishedGame(int f);
    int getTimeSteps(void);
    void setTimeSteps(int steps);
    float getTimeSpeed(void);
    void setTimeSpeed(float speed);
    float getTimeSpeedBeforePause(void);
    void setTimeSpeedBeforePause(float speed);
    int getCooldownPower1(void);
    void setCooldownPower1(int cd);
    int getCooldownPower2(void);
    void setCooldownPower2(int cd);
    int getPower1Active(void);
    void setPower1Active(int cd);
    int getPower2Active(void);
    void setPower2Active(int cd);

    void createLevel(int lvl);
    void playLevel(void);
    void resetLevel(void);

    bool paint = false;

private:
    static GameLevel* gamelevelInstance;
    UIGameplayMap* gameplayMap;

    clock_t currentTime = 0;
    int numLevel = 0;
    map<string, int> agentAttributes;
    map<string, int> powers;
    vector<Agent*> agents;
    int addedAgents = 0;
    vector<int> deletedAgents;
    int idCounter = 0;

    int timeSteps = 0;
    float timeSpeed = 2.5;
    float timeSpeedBeforePause = 2.5;

    int power1Active = 0;
    int power2Active = 0;

    int cooldownPower1 = 0;
    int cooldownPower2 = 0;

    //0 = notFinished, 1 = finishedCompleted, 2 = finishedTimeOut, 3 = finished0Agents,
    //4 = finishedBack
    int finishedGame = 0;

    GameLevel() {};
    GameLevel(GameLevel const&) {};
    GameLevel& operator=(GameLevel const&);

    void generateInitialAgents(void);
    void reproduce(void);
    void die(void);
    void dieAndReproduce(void);
    bool validatePosition(int posx, int posy);
};

#endif /* defined(__simulplay__GameLevel__) */
