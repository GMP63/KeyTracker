#ifndef IMESSAGESERVER_H
#define IMESSAGESERVER_H

#include <string>

class IMessageServer
{
public:
    virtual void keySent(std::string& key, std::string& url, unsigned int port) = 0;
    virtual void getTopHotkeys() = 0;
    virtual void getTopHotkeysReply() = 0;
    virtual void isHotKey() = 0;
    virtual void isHotKeyReply() = 0;
    virtual void setTopNumber(unsigned int n) = 0;
};


#endif // IMESSAGESERVER_H