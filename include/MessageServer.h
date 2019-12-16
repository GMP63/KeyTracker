#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include "IMessageServer.h"

class MessageServer : public IMessageServer
{
public:
    virtual void keySent(std::string& key, std::string& url, unsigned int port){}
    virtual void getTopHotkeys(){}
    virtual void isHotKey(){}
    virtual void setTopNumber(unsigned int n){}
};


#endif // MESSAGESERVER_H