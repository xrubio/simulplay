//
//  Deplete.h
//  simulplay
//
//  Created by Guillem Laborda on 23/10/14.
//
//

#ifndef __simulplay__Deplete__
#define __simulplay__Deplete__

#include "Act.h"

class Deplete : public Act {

public:
    Deplete() {};
    Deplete(Deplete const&) {};
    Deplete& operator=(Deplete const&);

    bool execute(int type, int indexAgent);
};

#endif /* defined(__simulplay__Deplete__) */
