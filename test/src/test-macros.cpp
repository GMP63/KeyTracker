/**
 * @file test-macros.cpp
 * @brief Implementation file defining static strings and private members.
 * @author Guillermo M. Paris
 * @date 2019-10-27
 */

#include "test-macros.h"

namespace gp
{
namespace test
{

const char* TestByMacros::sFailed = "TEST FAILED\n";
const char* TestByMacros::sTested = "Current tested value ";
const char* TestByMacros::sNotExpected = "is not the expected one.";
const char* TestByMacros::sEqual = "is equal to";
const char* TestByMacros::sNotEqual = "is not equal to";
const char* TestByMacros::sNull = "is NULL";
const char* TestByMacros::sNotNull = "is not NULL";

const char* TestByMacros::sNotGreaterOrEqual = "is not greater than or equal to";
const char* TestByMacros::sNotLowerOrEqual = "is not lower than or equal to";
const char* TestByMacros::sNotGreater = "is not greater than";
const char* TestByMacros::sNotLower = "is not lower than";

const char* TestByMacros::sGoodMsg = "ALL TEST PASSED !";
const char* TestByMacros::sBadMsg = "There are ERRORS in test(s) detailed above by \"TEST FAILED\".";

TestByMacros::TestByMacros()
    : goodCount(0)
    , badCount(0)
{
}

TestByMacros::TestByMacros(int good, int bad)
    : goodCount(good)
    , badCount(bad)
{
}

int TestByMacros::getCounters(int& good, int& bad) const
{
    good = goodCount;
    bad = badCount;
    return good + bad;
}

void TestByMacros::printCounters(std::ostream& stream) const
{
    int total = goodCount + badCount;

    stream << "\nTotal tests: " << total
           << "\nTest passed: " << goodCount
           << "\nTest failed: " << badCount
           << "\n\n" << (total > 0 ? (badCount > 0 ? sBadMsg : sGoodMsg) : "") << std::endl; 
}

void TestByMacros::complainAboutValue(bool value, const char* file,
                                      int line, const char* function)
{
    const char* pValue = (value ? "true" : "false");
    std::cout << sFailed << sTested << pValue << ' ' << sNotExpected
              << "\nFile: " << file << " , Line: " << line
              << "\nFunction: " << function << std::endl;
}

bool TestByMacros::isTrue(bool v, const char* file, int line, const char* function)
{
    if (v)
    {
        ++goodCount;
        return true;
    }
    else
    {
        ++badCount;
        complainAboutValue(v, file, line, function);
        return false;
    };
}

bool TestByMacros::isFalse(bool v, const char* file, int line, const char* function)
{
    if (!v)
    {
        ++goodCount;
        return true;
    }
    else
    {
        ++badCount;
        complainAboutValue(v, file, line, function);
        return false;
    };
}

} // namespace test
} // namespace gp
