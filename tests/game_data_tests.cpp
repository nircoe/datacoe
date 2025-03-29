#include <gtest/gtest.h>
#include <datacoe/game_data.hpp>

namespace datacoe
{
    TEST(GameDataTest, DefaultConstructor)
    {
        GameData gd;
        ASSERT_EQ(gd.getNickname(), "");
        ASSERT_EQ(gd.getHighscore(), 0);
    }

    TEST(GameDataTest, ParameterizedConstructor)
    {
        GameData gd("Player1", 500);
        ASSERT_EQ(gd.getNickname(), "Player1");
        ASSERT_EQ(gd.getHighscore(), 500);
    }

    TEST(GameDataTest, SetAndGetNickname)
    {
        GameData gd;
        gd.setNickname("TestName");
        ASSERT_EQ(gd.getNickname(), "TestName");

        // Test changing nickname
        gd.setNickname("NewName");
        ASSERT_EQ(gd.getNickname(), "NewName");

        // Test empty nickname
        gd.setNickname("");
        ASSERT_EQ(gd.getNickname(), "");
    }

    TEST(GameDataTest, SetAndGetHighscore)
    {
        GameData gd;
        gd.setHighscore(300);
        ASSERT_EQ(gd.getHighscore(), 300);

        // Test changing highscore
        gd.setHighscore(400);
        ASSERT_EQ(gd.getHighscore(), 400);

        // Test zero highscore
        gd.setHighscore(0);
        ASSERT_EQ(gd.getHighscore(), 0);

        // Test negative highscore (if allowed by your game's rules)
        gd.setHighscore(-10);
        ASSERT_EQ(gd.getHighscore(), -10);
    }

    TEST(GameDataTest, ToJsonBasic)
    {
        GameData gd("JsonTest", 400);
        json j = gd.toJson();

        ASSERT_TRUE(j.is_object());
        ASSERT_TRUE(j.contains("nickname"));
        ASSERT_TRUE(j.contains("highscore"));
        ASSERT_EQ(j["nickname"], "JsonTest");
        ASSERT_EQ(j["highscore"], 400);
    }

    TEST(GameDataTest, ToJsonEmptyNickname)
    {
        GameData gd("", 100);
        json j = gd.toJson();

        ASSERT_EQ(j["nickname"], "");
        ASSERT_EQ(j["highscore"], 100);
    }

    TEST(GameDataTest, FromJsonBasic)
    {
        json j;
        j["nickname"] = "JsonTest";
        j["highscore"] = 400;

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "JsonTest");
        ASSERT_EQ(gd.getHighscore(), 400);
    }

    TEST(GameDataTest, FromJsonEmptyNickname)
    {
        json j;
        j["nickname"] = "";
        j["highscore"] = 400;

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "");
        ASSERT_EQ(gd.getHighscore(), 400);
    }

    TEST(GameDataTest, ToAndFromJsonRoundTrip)
    {
        GameData original("RoundTrip", 550);
        json j = original.toJson();
        GameData restored = GameData::fromJson(j);

        ASSERT_EQ(restored.getNickname(), original.getNickname());
        ASSERT_EQ(restored.getHighscore(), original.getHighscore());
    }

    TEST(GameDataTest, FromJsonMissingNickname)
    {
        json j;
        // Missing nickname field
        j["highscore"] = 400;

        ASSERT_THROW(GameData::fromJson(j), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonMissingHighscore)
    {
        json j;
        j["nickname"] = "TestName";
        // Missing highscore field

        ASSERT_THROW(GameData::fromJson(j), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonWrongTypes)
    {
        json j1;
        j1["nickname"] = 12345; // Number instead of string
        j1["highscore"] = 400;

        ASSERT_THROW(GameData::fromJson(j1), std::runtime_error);

        json j2;
        j2["nickname"] = "TestName";
        j2["highscore"] = "400"; // String instead of number

        ASSERT_THROW(GameData::fromJson(j2), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonExtraFields)
    {
        json j;
        j["nickname"] = "TestName";
        j["highscore"] = 400;
        j["extraField"] = "This should be ignored";

        // Extra fields should be ignored
        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "TestName");
        ASSERT_EQ(gd.getHighscore(), 400);
    }

    TEST(GameDataTest, FromJsonSpecialCharacters)
    {
        json j;
        j["nickname"] = "Test@#$%^&*()";
        j["highscore"] = 400;

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "Test@#$%^&*()");
        ASSERT_EQ(gd.getHighscore(), 400);
    }

    TEST(GameDataTest, FromJsonLargeValues)
    {
        json j;
        j["nickname"] = "TestName";
        j["highscore"] = 2147483647; // Max int value

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getHighscore(), 2147483647);
    }

    TEST(GameDataTest, CopyConstructor)
    {
        GameData original("Original", 100);
        GameData copy = original;

        ASSERT_EQ(copy.getNickname(), "Original");
        ASSERT_EQ(copy.getHighscore(), 100);

        // Modifying copy shouldn't affect original
        copy.setNickname("Modified");
        copy.setHighscore(200);

        ASSERT_EQ(original.getNickname(), "Original");
        ASSERT_EQ(original.getHighscore(), 100);
        ASSERT_EQ(copy.getNickname(), "Modified");
        ASSERT_EQ(copy.getHighscore(), 200);
    }

    TEST(GameDataTest, AssignmentOperator)
    {
        GameData original("Original", 100);
        GameData assigned;

        assigned = original;

        ASSERT_EQ(assigned.getNickname(), "Original");
        ASSERT_EQ(assigned.getHighscore(), 100);

        // Modifying assigned shouldn't affect original
        assigned.setNickname("Modified");
        assigned.setHighscore(200);

        ASSERT_EQ(original.getNickname(), "Original");
        ASSERT_EQ(original.getHighscore(), 100);
        ASSERT_EQ(assigned.getNickname(), "Modified");
        ASSERT_EQ(assigned.getHighscore(), 200);
    }

} // namespace datacoe