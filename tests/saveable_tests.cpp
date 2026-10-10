#include <datacoe/saveable.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>

using namespace datacoe;
using namespace datacoe_test;

//==============================================================================
//                  SaveableTests - saveable concept tests
//==============================================================================

class SaveableTests : public ::testing::Test
{
};

//==============================================================================
//                              Concept Checks
//==============================================================================

TEST_F(SaveableTests, ConceptSatisfaction)
{
    // Test 1: type with the macro is saveable
    static_assert(saveable<player_save>);

    // Test 2: hand-written conversions count, detection is compile-time only so throwing is fine
    static_assert(saveable<throwing_save>);

    // Test 3: no to_json/from_json
    static_assert(!saveable<plain_struct>);

    // Test 4: conversions exist but no default constructor
    static_assert(!saveable<no_default_save>);

    // Test 5: pointers are not semiregular with a json conversion
    static_assert(!saveable<int *>);

    // Test 6: types nlohmann converts by itself
    static_assert(saveable<std::string>);
}

//==============================================================================
//                              nlohmann Contract
//==============================================================================

TEST_F(SaveableTests, MacroRoundTrip)
{
    // Test 1: value survives json and back
    {
        const player_save original{"nir", 42, {1, 2, 3}};

        EXPECT_EQ(nlohmann::json(original).get<player_save>(), original);
    }

    // Test 2: a missing key falls back to the default, datacoe relies on this
    {
        const auto j = nlohmann::json::parse(R"({"nickname":"partial"})");
        const auto loaded = j.get<player_save>();

        EXPECT_EQ(loaded.nickname, "partial");
        EXPECT_EQ(loaded.highscore, 0);
        EXPECT_TRUE(loaded.unlocked.empty());
    }
}
