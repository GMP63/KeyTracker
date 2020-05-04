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

    Message() : m_cmd(Command::notDefined), m_nbyte(0), m_nshort(0), m_number(0) {}

    Message(Command c, uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : m_cmd(c), m_nbyte(u8), m_nshort(u16), m_number(u32) {}

    Message(Command c, const std::string& s1, const std::string& s2,
            uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : m_str1(s1), m_str2(s2), m_cmd(c), m_nbyte(u8), m_nshort(u16), m_number(u32) {}

    Message(Command c, const std::string&& s1, const std::string&& s2,
            uint8_t u8 = 0, uint16_t u16 = 0, uint32_t u32 = 0)
        : m_str1(s1), m_str2(s2), m_cmd(c), m_nbyte(u8), m_nshort(u16), m_number(u32) {}

    Message(const Message& m)
        : m_str1(m.m_str1), m_str2(m.m_str2), m_cmd(m.m_cmd)
        , m_nbyte(m.m_nbyte), m_nshort(m.m_nshort), m_number(m.m_number) {}

    Message(const Message&& m)
        : m_str1(std::move(m.m_str1)), m_str2(std::move(m.m_str2)), m_cmd(m.m_cmd)
        , m_nbyte(m.m_nbyte), m_nshort(m.m_nshort), m_number(m.m_number) {}

//  ~Message() {m_str1.clear(); m_str2.clear();}

    const Message& operator = (const Message& m) // pop = delete;
          { m_cmd = m.m_cmd; m_str1 = m.m_str1; m_str2 = m.m_str2;
            m_nbyte = m.m_nbyte; m_nshort = m.m_nshort; m_number = m.m_number; return m; }

    const Message& operator = (Message&& m)
          { m_cmd = m.m_cmd; m_str1 = std::move(m.m_str1); m_str2 = std::move(m.m_str2);
            m_nbyte = m.m_nbyte; m_nshort = m.m_nshort; m_number = m.m_number; return *this; }

    std::string   getString1() { return m_str1; }
    std::string   getString2() { return m_str2; } // sometimes a copy is necessary :)
    void          getString1(std::string& s1) { s1 = m_str1; }
    void          getString2(std::string& s2) { s2 = m_str1; }
    std::string&  getStringRef1() { return m_str1; }
    std::string&  getStringRef2() { return m_str2; }
    Command       getCommand() { return m_cmd; }
    uint8_t       getByte()    { return m_nbyte; }
    uint16_t      getShort()   { return m_nshort; }
    uint32_t      getNumber()  { return m_number; }

private:
    std::string  m_str1;
    std::string  m_str2;
    Command      m_cmd;
    uint8_t      m_nbyte;
    uint16_t     m_nshort;
    uint32_t     m_number;
};

#endif // MESSAGE_H
