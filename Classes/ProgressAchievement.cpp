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
//  ProgressAchievement.cpp
//  simulplay
//
//  Created by Guillem Laborda on 5/3/15.
//
//

#include "ProgressAchievement.h"
#include <cocos2d.h>

ProgressAchievement::ProgressAchievement(const std::string & icon, const std::string & resource, const std::string & goalType, int level, bool completed, bool occult) : Achievement(icon, resource, goalType, level, completed, occult)
{
}

bool ProgressAchievement::checkAchievement(const std::string & typeAch, int level)
{
    CCLOG("ProgressAchievement::checkAchievement not implemented yet");
    return false;
}

bool ProgressAchievement::checkInGameAchievement(const std::string & typeAch, int level, int agentColorCode)
{
    CCLOG("ProgressAchievement::checkInGameAchievement not implemented yet");
    return false;
}


