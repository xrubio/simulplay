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
//  AreaPower.h
//  simulplay
//
//  Created by Guillem Laborda on 18/09/14.
//
//

#ifndef __simulplay__AreaPower__
#define __simulplay__AreaPower__

#include "Power.h"
#include "Position.h"
#include <cocos2d.h>

using cocos2d::Vec2;

class AreaPower : public Power {

public:
    AreaPower( const std::string & name, const PowerType & type, float c, float dur, float cost);
    bool isInRadius( const Position & position ) const;
    void setArea( const Vec2 & center, float radius );

    bool isGlobal() const { return false; }

private:
    float _radius;
    Vec2 _center;
};

#endif /* defined(__simulplay__AreaPower__) */

