using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using OsuParser;
using UnityEngine;
using UnityEngine.Networking;

public class LevelLoader :MonoBehaviour
{


    static readonly List<LevelMetadata> LevelRegistry = new()
    {
        new LevelMetadata
        {
            displayName = "Iris Out",
            artist = "Kenshi Yonezu",
            folderName = "irisout",
            difficulties = 3
        },
        new LevelMetadata
        {
            displayName = "Fancy",
            artist = "TWICE",
            folderName = "fancy",
            difficulties = 3
        },
        new LevelMetadata()
        {
            displayName = "DDU-DU DDU-DU",
            folderName = "ddududdudu",
            artist = "BLACKPINK",
            difficulties = 4
        },
        new LevelMetadata()
        {
            displayName = "How You Like That",
            folderName = "howyoulikethat",
            artist = "BLACKPINK",
            difficulties = 3
        },
        new LevelMetadata()
        {
            displayName = "APT.",
            folderName = "apt",
            artist = "ROSÉ",
            difficulties = 3
        },
        new LevelMetadata()
        {
            displayName = "Kill This Love",
            folderName = "killthislove",
            artist = "BLACKPINK",
            difficulties = 4
        }
    };

    public void Load(Level level, int difficultyIndex, Action<OsuBeatmap> onLoaded)
    {
        Debug.Log($"LevelLoader: Requested load for {level} at difficulty {difficultyIndex}");
        var levelMetadata = LevelRegistry[(int)level];

        if(levelMetadata == null)
        {
            Debug.LogError($"LevelLoader: Could not find level named '{level}'");
            return;
        }

        if(difficultyIndex < 0 || difficultyIndex >= levelMetadata.difficulties)
        {
            Debug.LogError($"LevelLoader: Difficulty index {difficultyIndex} is out of range for {level}");
            return;
        }

        StartCoroutine(LoadRoutine(levelMetadata.folderName, difficultyIndex, onLoaded));
    }

// --- 3. The Internal Logic ---

    IEnumerator LoadRoutine(string folderName, int difficultyIndex, Action<OsuBeatmap> callback)
    {
        // Construct path: StreamingAssets/irisout/1.osu
        var mapPath = Path.Combine(Application.streamingAssetsPath, folderName, difficultyIndex + ".osu");

        OsuBeatmap loadedOsuBeatmap;

        if(!mapPath.Contains("://"))
            mapPath = "file://" + mapPath;

        using (var www = UnityWebRequest.Get(mapPath))
        {
            yield return www.SendWebRequest();

            if(www.result != UnityWebRequest.Result.Success)
                Debug.LogError($"LevelLoader Error: {www.error}");

            Debug.Log("LevelLoader: File read. Parsing...");

            // Parse it using the OsuParser we made earlier
            loadedOsuBeatmap = FileParser.Parse(www.downloadHandler.text);
        }

        // Construct path: StreamingAssets/irisout/audio.mp3
        var audioPath = Path.Combine(Application.streamingAssetsPath, folderName, "audio.mp3");

        using (var wwwAudio = UnityWebRequestMultimedia.GetAudioClip(audioPath, AudioType.MPEG))
        {
            yield return wwwAudio.SendWebRequest();

            loadedOsuBeatmap.audioClip = wwwAudio.result == UnityWebRequest.Result.Success
                ? DownloadHandlerAudioClip.GetContent(wwwAudio)
                : null;
            // Send the data back to whoever asked for it
        }

        callback?.Invoke(loadedOsuBeatmap);
    }

// Simple data class for your registry
}

public enum Level
{
    IrisOut,
    Fancy,
    DduDuDduDu,
    HowYouLikeThat,
    Apt,
}

public enum LevelDifficulty
{
    Easy,
    Medium,
    Hard,
    Expert
}

internal class LevelMetadata
{
    public string displayName;
    public string artist;
    public string folderName; // e.g. "irisout"
    public int difficulties;
}
