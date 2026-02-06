using System;
using System.Linq;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class LevelManager :MonoBehaviour
{
    public TMP_Dropdown levelDropdown;
    public TMP_Dropdown difficultyDropdown;
    public Button startButton;

    public AudioSource music;
    
    public Level Level { get; private set; }
    public LevelDifficulty Difficulty { get; private set; }

    void Start()
    {
        levelDropdown.ClearOptions();
        var levelNames = LevelData.LevelRegistry
            .Select(meta => $"{meta.displayName} - {meta.artist}").ToList();
        levelDropdown.AddOptions(levelNames);
        levelDropdown.onValueChanged.AddListener(_ =>
        {
            Level = (Level)levelDropdown.value;
            music.Stop();
            
            InitDifficultyOptions();
            StartMusic();
        });
        Level = (Level)levelDropdown.value;
        
        InitDifficultyOptions();
        StartMusic();

        difficultyDropdown.onValueChanged.AddListener(_ => Difficulty = (LevelDifficulty)difficultyDropdown.value);

        startButton.interactable = BleConnection.Instance.controllerConnected;
    }

    void StartMusic()
    {
        StartCoroutine(LevelLoader.LoadAudioClip(Level, clip =>
        {
            music.clip = clip;
            music.loop = true;
            music.Play();
        }));
    }

    void InitDifficultyOptions()
    {
        difficultyDropdown.ClearOptions();
        var difficultyNames = Enum.GetNames(typeof(LevelDifficulty))
            .Take(LevelData.LevelRegistry[(int)Level].difficulties).ToList();
        difficultyDropdown.AddOptions(difficultyNames);
        Difficulty = (LevelDifficulty)difficultyDropdown.value;
    }

    void Update()
    {
        if(!startButton) return;
        startButton.interactable = BleConnection.Instance.controllerConnected;
    }
}
