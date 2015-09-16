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
//  Kill.cpp
//  simulplay
//
//  Created by Guillem Laborda on 10/11/14.
//
//

#include "Kill.h"
#include "UIGameplayMap.h"

list<Agent*>::reverse_iterator Kill::execute(int type, Agent* agent)
{
    //Agent* agent = GameLevel::getInstance()->getAgents().at(type).at(indexAgent);
    float tech =  GameLevel::getInstance()->getAttributesValues("TECHNOLOGY", max(1, agent->getValOfAttribute("TECHNOLOGY")));
    int probKill = agent->getValOfAttribute("HOSTILITY") * tech;
    int mobility = (agent->getValOfAttribute("MOBILITY") * 4) + 1 * tech;
    if ((rand() % 100) < probKill * 10) {
        int maxIterations = 100;
        bool kill = false;
        int posx = rand() % (2 * mobility) + (agent->getPosition()->getX() - mobility);
        int posy = rand() % (2 * mobility) + (agent->getPosition()->getY() - mobility);
        while (maxIterations > 0 and kill == false) {
            if (posx >= 0 and posx < 480 and posy >= 0 and posy < 320) {
                if (GameLevel::getInstance()->getAgentAtMap(posx, posy) != nullptr and GameLevel::getInstance()->getAgentAtMap(posx, posy)->getType() != type) {
                    GameLevel::getInstance()->getAgentAtMap(posx, posy)->setLife(0);
                    kill = true;
                }
            }
            posx = rand() % (2 * mobility) + (agent->getPosition()->getX() - mobility);
            posy = rand() % (2 * mobility) + (agent->getPosition()->getY() - mobility);
            maxIterations--;
        }
    }
}