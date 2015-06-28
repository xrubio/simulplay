
#include "Tutorial.h"
#include "GameLevel.h"
#include "../libs/pugixml/pugixml.hpp"
#include <string>
#include "Message.h"
#include "LocalizedString.h"

Tutorial::Tutorial() : _current(0)
{
}

Tutorial::~Tutorial()
{
    if(_current)
    {
        delete _current;
    }
    MessageList::iterator it = _messages.begin();
    while(it!=_messages.end())
    {
        Message * message = *it;
        it = _messages.erase(it);
        delete message;
    }
}

void Tutorial::loadMessagesForLevel(const pugi::xml_node & node)
{
    int numMessages = node.attribute("numMessages").as_int();

    for(int i=0; i<numMessages; i++)
    {
        pugi::xml_node message = node.child("message");
        while(message!= nullptr)
        {
            int id = message.attribute("id").as_int();
            if(id!=i)
            {
                message = message.next_sibling("message");
                continue;
            }
            std::string trigger = message.attribute("trigger").value();
            float xPos = message.attribute("x").as_float();
            float yPos = message.attribute("y").as_float();
            float lineBreak = message.attribute("lineWidth").as_float();
            std::string text = LocalizedString::create(message.attribute("text").value());
            if(trigger=="next")
            { 
                _messages.push_back(new MessageNext(text, xPos, yPos, lineBreak));
            }
            else if(trigger=="time")
            {
                int step = message.attribute("step").as_int();
                _messages.push_back(new MessageTime(text, xPos, yPos, lineBreak, step));
            }
            message = message.next_sibling("message");
        }
    }
}

bool Tutorial::loadTutorial()
{ 
    string dir = "data/";

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    dir = "";
#endif

    string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(dir + "tutorial.xml");
    pugi::xml_document doc;
    doc.load_file((fullPath).c_str());

    int numLevel = GameLevel::getInstance()->getNumLevel();
    pugi::xml_node level = doc.child("level");
    CCLOG("checking level %i", numLevel);
    while(level!= nullptr)
    {
        int num = stoi(level.attribute("num").value());
        if(numLevel==num)
        {
            CCLOG("level: %i found!", numLevel);
            loadMessagesForLevel(level);
            return true;
        }
        level = level.next_sibling("level");
    }
    // not available tutorial for level
    CCLOG("level %i not found!", numLevel);
    return false;
}

bool Tutorial::checkNextMessage()
{
    if(_current || _messages.size()==0)
    {
        return false;
    }
    Message * nextMessage = *(_messages.begin());
    if(nextMessage->meetConditions())
    {
        _current = nextMessage;
        _messages.pop_front();
        return true;
    }
    return false;
}

void Tutorial::removeCurrentMessage()
{
    if(!_current)
    {
        return;
    }
    delete _current;
    _current = 0;
}

