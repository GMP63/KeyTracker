/**
 * @file test-macros.inl
 * @brief Implementation file defining template functions.
 * @author Guillermo M. Paris
 * @date 2019-10-27
 */

namespace gp
{
namespace test
{

template<typename V, typename E>
void TestByMacros::complainAboutValue(V value, E expected, const char* reason,
                                      const char* file, int line, const char* function)
{
    std::cout << sFailed << sTested << value << ' ' << reason << ' ' << expected
              << "\nFile: " << file << " , Line: " << line
              << "\nFunction: " << function << std::endl;
}

 //-------------------------------------
 // Template functions implementations |
 //-------------------------------------

template<typename V>
bool TestByMacros::isZero(V v, const char* file, int line, const char* function)
{
    if (v == 0 ) { ++goodCount; return true; }
    else { ++badCount; complainAboutValue(v, 0, sNotEqual, file, line, function); return false; };
}

template<typename V>
bool TestByMacros::isNotZero(V v, const char* file, int line, const char* function)
{
    if (v != 0 ) { ++goodCount; return true; }
    else { ++badCount; complainAboutValue(v, 0, sEqual, file, line, function); return false; }
}

template<typename V>
bool TestByMacros::isNull(V v, const char* file, int line, const char* function)
{
    if (v == nullptr ) { ++goodCount; return true; }
    else { ++badCount; complainAboutValue(v, 0, sNotNull, file, line, function); return false; }
}

template<typename V>
bool TestByMacros::isNotNull(V v, const char* file, int line, const char* function)
{
    if (v != nullptr ) { ++goodCount; return true; }
    else { ++badCount; complainAboutValue(v, 0, sNull, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::areEquals(A a, B b, const char* file, int line, const char* function)
{
    if (a == b) { ++goodCount; return true; }
    else { ++badCount; complainAboutValue(a, b, sNotEqual, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::areNotEquals(A a, B b, const char* file, int line, const char* function)
{
    if (a != b) { ++goodCount; return true; }
    else { ++badCount;complainAboutValue(a, b, sEqual, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::isGreaterOrEqual(A a, B b, const char* file, int line, const char* function)
{
    if (a >= b) { ++goodCount; return true; }
    else { ++badCount;complainAboutValue(a, b, sNotGreaterOrEqual, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::isLowerOrEqual(A a, B b, const char* file, int line, const char* function)
{
    if (a <= b) { ++goodCount; return true; }
    else { ++badCount;complainAboutValue(a, b, sNotLowerOrEqual, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::isGreater(A a, B b, const char* file, int line, const char* function)
{
    if (a > b) { ++goodCount; return true; }
    else { ++badCount;complainAboutValue(a, b, sNotGreater, file, line, function); return false; }
}

template<typename A, typename B>
bool TestByMacros::isLower(A a, B b, const char* file, int line, const char* function)
{
    if (a < b) { ++goodCount; return true; }
    else { ++badCount;complainAboutValue(a, b, sNotLower, file, line, function); return false; }
}

} // namespace test
} // namespace gp
