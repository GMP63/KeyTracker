/**
 * @file test-macros.h
 * @brief Header file declaring macros to test the exam logic.
 * @author Guillermo M. Paris
 * @date 2019-12-10
 */

#include <cassert>
#include <iostream>

namespace gp
{
namespace test
{

class TestByMacros
{
public:
    TestByMacros();
    TestByMacros(TestByMacros&)  = delete;
    TestByMacros(TestByMacros&&) = delete;
    TestByMacros(int good, int bad);

    int  getCounters(int& good, int& bad) const;
    void printCounters(std::ostream& stream) const;
    void complainAboutValue(bool value, const char* file,
                            int line, const char* function);
    bool isTrue(bool v, const char* file, int line, const char* function);
    bool isFalse(bool v, const char* file, int line, const char* function);

    //-----------------------------------
    // Template functions declarations  |
    //-----------------------------------

    template<typename V>
    bool isZero(V v, const char* file, int line, const char* function);
    template<typename V>
    bool isNotZero(V v, const char* file, int line, const char* function);
    template<typename V>
    bool isNull(V v, const char* file, int line, const char* function);
    template<typename V>
    bool isNotNull(V v, const char* file, int line, const char* function);

    template<typename A, typename B>
    bool areEquals(A a, B b, const char* file, int line, const char* function);
    template<typename A, typename B>
    bool areNotEquals(A a, B b, const char* file, int line, const char* function);
    template<typename A, typename B>
    bool isGreaterOrEqual(A a, B b, const char* file, int line, const char* function);
    template<typename A, typename B>
    bool isLowerOrEqual(A a, B b, const char* file, int line, const char* function);
    template<typename A, typename B>
    bool isGreater(A a, B b, const char* file, int line, const char* function);
    template<typename A, typename B>
    bool isLower(A a, B b, const char* file, int line, const char* function);

private:
    template<typename V, typename E>
    static void complainAboutValue(V value, E expected, const char* reason,
                                   const char* file, int line, const char* function);

    static const char* sFailed;
    static const char* sTested;
    static const char* sNotExpected;
    static const char* sEqual;
    static const char* sNotEqual;
    static const char* sNull;
    static const char* sNotNull;

    static const char* sNotGreaterOrEqual;
    static const char* sNotLowerOrEqual;
    static const char* sNotGreater;
    static const char* sNotLower;

    static const char* sGoodMsg;
    static const char* sBadMsg;

    int goodCount;
    int badCount;
};

} // namespace test
} // namespace gp

using tst = gp::test::TestByMacros;

// Macro definitions
#define START_TESTS             tst   oTest
#define TEST_REF                tst&  oTest
#define TEST_OBJ                      oTest
#define TEST                          oTest

#define EXPECT_TRUE(testVal)    oTest.isTrue(testVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_FALSE(testVal)   oTest.isFalse(testVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_Z(testVal)       oTest.isZero(testVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_NZ(testVal)      oTest.isNotZero(testVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_NULL(testVal)    oTest.isNull(testVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_NOTNULL(testVal) oTest.isNotNull(testVal, __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_EQ(testVal,expectedVal)  oTest.areEquals(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_NEQ(testVal,expectedVal) oTest.areNotEquals(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_GE(testVal,expectedVal)  oTest.isGreaterOrEqual(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_LE(testVal,expectedVal)  oTest.isLowerOrEqual(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_GT(testVal,expectedVal)  oTest.isGreater(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)
#define EXPECT_LW(testVal,expectedVal)  oTest.isLower(testVal,expectedVal, __FILE__, __LINE__, __FUNCTION__)

#define TEST_RESULTS(good,bad)  oTest.getCounters(good,bad)
#define PRINT_RESULTS(stream)   oTest.printCounters(stream)


// Template functions definitions
#include "test-macros.inl"
