#include "gtest/gtest.h"
#include "datacoe/game_data.hpp"
#include <iostream>

namespace datacoe
{
    TEST(GameDataTest, DefaultConstructor)
    {
        GameData gd;
        ASSERT_EQ(gd.getNickname(), "");
        std::array<std::size_t, 4> expectedScores = {0, 0, 0, 0};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, ParameterizedConstructor)
    {
        std::array<std::size_t, 4> scores = {100, 200, 300, 400};
        GameData gd("Player1", scores);
        ASSERT_EQ(gd.getNickname(), "Player1");
        ASSERT_EQ(gd.getHighscores(), scores);
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

    TEST(GameDataTest, SetAndGetHighscores)
    {
        GameData gd;
        std::array<std::size_t, 4> scores = {300, 400, 500, 600};
        gd.setHighscores(scores);
        ASSERT_EQ(gd.getHighscores(), scores);

        // Test changing highscores
        std::array<std::size_t, 4> newScores = {400, 500, 600, 700};
        gd.setHighscores(newScores);
        ASSERT_EQ(gd.getHighscores(), newScores);

        // Test zero highscores
        std::array<std::size_t, 4> zeroScores = {0, 0, 0, 0};
        gd.setHighscores(zeroScores);
        ASSERT_EQ(gd.getHighscores(), zeroScores);
    }

    TEST(GameDataTest, ToJsonBasic)
    {
        std::array<std::size_t, 4> scores = {100, 200, 300, 400};
        GameData gd("JsonTest", scores);
        json j = gd.toJson();

        ASSERT_TRUE(j.is_object());
        ASSERT_TRUE(j.contains("nickname"));
        ASSERT_TRUE(j.contains("highscores"));
        ASSERT_EQ(j["nickname"], "JsonTest");

        auto highscores = j["highscores"].get<std::array<std::size_t, 4>>();
        ASSERT_EQ(highscores, scores);
    }

    TEST(GameDataTest, ToJsonEmptyNickname)
    {
        std::array<std::size_t, 4> scores = {100, 200, 300, 400};
        GameData gd("", scores);
        json j = gd.toJson();

        ASSERT_EQ(j["nickname"], "");
        auto highscores = j["highscores"].get<std::array<std::size_t, 4>>();
        ASSERT_EQ(highscores, scores);
    }

    TEST(GameDataTest, FromJsonBasic)
    {
        json j;
        j["nickname"] = "JsonTest";
        j["highscores"] = {100, 200, 300, 400};

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "JsonTest");
        std::array<std::size_t, 4> expectedScores = {100, 200, 300, 400};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, FromJsonEmptyNickname)
    {
        json j;
        j["nickname"] = "";
        j["highscores"] = {100, 200, 300, 400};

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "");
        std::array<std::size_t, 4> expectedScores = {100, 200, 300, 400};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, ToAndFromJsonRoundTrip)
    {
        std::array<std::size_t, 4> scores = {150, 250, 350, 450};
        GameData original("RoundTrip", scores);
        json j = original.toJson();
        GameData restored = GameData::fromJson(j);

        ASSERT_EQ(restored.getNickname(), original.getNickname());
        ASSERT_EQ(restored.getHighscores(), original.getHighscores());
    }

    TEST(GameDataTest, FromJsonMissingNickname)
    {
        json j;
        // Missing nickname field
        j["highscores"] = {100, 200, 300, 400};

        ASSERT_THROW(GameData::fromJson(j), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonMissingHighscores)
    {
        json j;
        j["nickname"] = "TestName";
        // Missing highscores field

        ASSERT_THROW(GameData::fromJson(j), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonWrongTypes)
    {
        json j1;
        j1["nickname"] = 12345; // Number instead of string
        j1["highscores"] = {100, 200, 300, 400};

        ASSERT_THROW(GameData::fromJson(j1), std::runtime_error);

        json j2;
        j2["nickname"] = "TestName";
        j2["highscores"] = "Not an array"; // String instead of array

        ASSERT_THROW(GameData::fromJson(j2), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonWrongArraySize)
    {
        json j;
        j["nickname"] = "TestName";
        j["highscores"] = {100, 200, 300}; // Only 3 elements instead of 4

        ASSERT_THROW(GameData::fromJson(j), std::runtime_error);

        json j2;
        j2["nickname"] = "TestName";
        j2["highscores"] = {100, 200, 300, 400, 500}; // 5 elements instead of 4

        ASSERT_THROW(GameData::fromJson(j2), std::runtime_error);
    }

    TEST(GameDataTest, FromJsonExtraFields)
    {
        json j;
        j["nickname"] = "TestName";
        j["highscores"] = {100, 200, 300, 400};
        j["extraField"] = "This should be ignored";

        // Extra fields should be ignored
        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "TestName");
        std::array<std::size_t, 4> expectedScores = {100, 200, 300, 400};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, FromJsonSpecialCharacters)
    {
        json j;
        j["nickname"] = "Test@#$%^&*()";
        j["highscores"] = {100, 200, 300, 400};

        GameData gd = GameData::fromJson(j);

        ASSERT_EQ(gd.getNickname(), "Test@#$%^&*()");
        std::array<std::size_t, 4> expectedScores = {100, 200, 300, 400};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, FromJsonLargeValues)
    {
        json j;
        j["nickname"] = "TestName";
        j["highscores"] = {9999999, 9999999, 9999999, 9999999}; // Large values

        GameData gd = GameData::fromJson(j);

        std::array<std::size_t, 4> expectedScores = {9999999, 9999999, 9999999, 9999999};
        ASSERT_EQ(gd.getHighscores(), expectedScores);
    }

    TEST(GameDataTest, CopyConstructor)
    {
        std::array<std::size_t, 4> scores = {100, 200, 300, 400};
        GameData original("Original", scores);
        GameData copy = original;

        ASSERT_EQ(copy.getNickname(), "Original");
        ASSERT_EQ(copy.getHighscores(), scores);

        // Modifying copy shouldn't affect original
        copy.setNickname("Modified");
        std::array<std::size_t, 4> newScores = {500, 600, 700, 800};
        copy.setHighscores(newScores);

        ASSERT_EQ(original.getNickname(), "Original");
        ASSERT_EQ(original.getHighscores(), scores);
        ASSERT_EQ(copy.getNickname(), "Modified");
        ASSERT_EQ(copy.getHighscores(), newScores);
    }

    TEST(GameDataTest, AssignmentOperator)
    {
        std::array<std::size_t, 4> scores = {100, 200, 300, 400};
        GameData original("Original", scores);
        GameData assigned;

        assigned = original;

        ASSERT_EQ(assigned.getNickname(), "Original");
        ASSERT_EQ(assigned.getHighscores(), scores);

        // Modifying assigned shouldn't affect original
        assigned.setNickname("Modified");
        std::array<std::size_t, 4> newScores = {500, 600, 700, 800};
        assigned.setHighscores(newScores);

        ASSERT_EQ(original.getNickname(), "Original");
        ASSERT_EQ(original.getHighscores(), scores);
        ASSERT_EQ(assigned.getNickname(), "Modified");
        ASSERT_EQ(assigned.getHighscores(), newScores);
    }

} // namespace datacoe