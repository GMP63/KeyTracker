

#ifndef VERBOSITY_H
#define VERBOSITY_H

#include <cstdint>

class Verbosity
{
public:
    enum level {
        error = 0, info = 1, trace = 2, debug = 3
    };

    Verbosity() : m_e(error)        {}
    Verbosity(int i)              { fromInt(i);  }
    Verbosity(unsigned int u)     { fromUint(u); }
    Verbosity(const Verbosity& v) { m_e = v.m_e; }

    int operator = (int i) { fromInt(i); return i; }
    unsigned int operator = (unsigned int u) { fromUint(u); return u; }

    bool operator <  (level l) const { return m_e < l;  }
    bool operator <= (level l) const { return m_e <= l; }
    bool operator == (level l) const { return m_e == l; }
    bool operator >= (level l) const { return m_e >= l; }
    bool operator >  (level l) const { return m_e > l;  }

    operator int()          { return int(m_e); }
    int          toInt()    { return int(m_e); }
    unsigned int toUint()   { return (unsigned int)(m_e); }
    enum level   toLevel()  { return m_e; }
    operator unsigned int() { return (unsigned int)(m_e); }

private:

    void fromInt(int i)
    {
        if (i <= 0)  m_e = error; // warnings & errors
        else if (i >= int(debug)) m_e = debug;
        else switch(i)
             {
                case 1:
                    m_e = info;
                    break;
                case 2:
                    m_e = trace;
                    break;
             }
    }

    void fromUint(unsigned int u)
    {
        if (u >= (unsigned int)(debug)) m_e = debug;
        else switch(u)
            {
                case 0:
                    m_e = error; // warnings & errors
                    break;
                case 1:
                    m_e = info;
                    break;
                case 2:
                    m_e = trace;
                    break;
            }
    }

    enum level m_e;
};

#endif // VERBOSITY_H
