using System;
using UnityEngine;

[Serializable]
public class RhythmLevelData
{
    public int levelNumber;
    public string difficulty;
    public int bpm;
    public int cellMs;
    public string pattern;
    public int patternLength;
    public float hitDensity;
    public float patternDurationSeconds;
}

public static class RhythmLevels
{
    public static readonly RhythmLevelData[] Levels =
    {
        new()
        {
            levelNumber = 1,
            difficulty = "Easy",
            bpm = 80,
            cellMs = 60000 / 80,
            pattern = "10001000",
            patternLength = 8,
            hitDensity = 0.125f,
            patternDurationSeconds = (60000f / 80f) * 8f / 1000f
        },

        new()
        {
            levelNumber = 2,
            difficulty = "Normal",
            bpm = 110,
            cellMs = 60000 / 110,
            pattern = "101001011010",
            patternLength = 12,
            hitDensity = 0.5f,
            patternDurationSeconds = (60000f / 110f) * 12f / 1000f
        },

        new()
        {
            levelNumber = 3,
            difficulty = "Hard",
            bpm = 140,
            cellMs = 60000 / 140,
            pattern = "1110111011101110",
            patternLength = 16,
            hitDensity = 0.75f,
            patternDurationSeconds = (60000f / 140f) * 16f / 1000f
        },

        new()
        {
            levelNumber = 4,
            difficulty = "Expert",
            bpm = 170,
            cellMs = 60000 / 170,
            pattern = "100101110010111010001011",
            patternLength = 24,
            hitDensity = 0.625f,
            patternDurationSeconds = (60000f / 170f) * 24f / 1000f
        },

        new()
        {
            levelNumber = 5,
            difficulty = "Insane",
            bpm = 200,
            cellMs = 60000 / 200,
            pattern = "11001100110011001100110011001100",
            patternLength = 32,
            hitDensity = 0.5f,
            patternDurationSeconds = (60000f / 200f) * 32f / 1000f
        }
    };

    public static RhythmLevelData GetLevel(int levelNumber)
    {
        foreach (var lvl in Levels)
            if(lvl.levelNumber == levelNumber)
                return lvl;

        Debug.LogError("Level not found: " + levelNumber);
        return null;
    }
}
