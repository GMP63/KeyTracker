#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * @file Message.h
 * @brief Message interface.
 * @author Guillermo M. Paris
 * @date 2019-12-15
 */

#include <string>

class Message
{
public:
    enum class Command : uint8_t
    {
        notDefined, stop, addKey, getKeyRanking, isHotKey, setRankingLength
    };

    Message() : cmd(Command::notDefined), nbyte(0), nshort(0), number(0) {}

    Message(Command c, uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : cmd(c), nbyte(u8), nshort(u16), number(u32) {}

    Message(Command c, const std::string& s1, const std::string& s2,
            uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : cmd(c), str1(s1), str2(s2), nbyte(u8), nshort(u16), number(u32) {}

    Message(Command c, const std::string&& s1, const std::string&& s2,
            uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : cmd(c), str1(s1), str2(s2), nbyte(u8), nshort(u16), number(u32) {}

    Message(const Message& m) // push = delete;
        : cmd(m.cmd), str1(m.str1), str2(m.str2), nbyte(m.nbyte), nshort(m.nshort), number(m.number) {}

    Message(const Message&& m)
        : cmd(m.cmd), str1(std::move(m.str1)), str2(std::move(m.str2)), nbyte(m.nbyte), nshort(m.nshort), number(m.number) {}

    const Message& operator = (const Message& m) // pop = delete;
          { cmd = m.cmd; str1 = m.str1; str2 = m.str2;
            nbyte = m.nbyte; nshort = m.nshort; number = m.number; return m; }

    const Message& operator = (Message&& m)
          { cmd = m.cmd; str1 = std::move(m.str1); str2 = std::move(m.str2);
            nbyte = m.nbyte; nshort = m.nshort; number = m.number; return *this; }

    std::string   getString1() { return str1; }
    std::string   getString2() { return str2; } // sometimes a copy is necessary :)
    void          getString1(std::string& s1) { s1 = str1; }
    void          getString2(std::string& s2) { s2 = str1; }
    std::string&  getStringRef1() { return str1; }
    std::string&  getStringRef2() { return str2; }
    Command       getCommand() { return cmd; }
    uint8_t       getByte()    { return nbyte; }
    uint16_t      getShort()   { return nshort; }
    uint32_t      getNumber()  { return number; }

private:
    std::string  str1;
    std::string  str2;
    Command      cmd;
    uint8_t      nbyte;
    uint16_t     nshort;
    uint32_t     number;
};

#endif // MESSAGE_H
