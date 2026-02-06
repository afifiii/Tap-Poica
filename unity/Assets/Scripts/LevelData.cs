using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(fileName = "LevelData", menuName = "LevelData", order = 1)]
public class LevelData :ScriptableObject
{

    public static readonly List<LevelMetadata> LevelRegistry = new()
    {
        new LevelMetadata
        {
            displayName = "How You Like That",
            folderName = "howyoulikethat",
            artist = "BLACKPINK",
        },
        new LevelMetadata
        {
            displayName = "DDU-DU DDU-DU",
            folderName = "ddududdudu",
            artist = "BLACKPINK",
            difficulties = 4
        },
        new LevelMetadata
        {
            displayName = "APT.",
            folderName = "apt",
            artist = "ROSÉ",
        },
        new LevelMetadata
        {
            displayName = "Kill This Love",
            folderName = "killthislove",
            artist = "BLACKPINK",
            difficulties = 4
        },
        new LevelMetadata
        {
            displayName = "Fancy",
            artist = "TWICE",
            folderName = "fancy",
        },
        new LevelMetadata
        {
            displayName = "Iris Out",
            artist = "Kenshi Yonezu",
            folderName = "irisout",
        },
        new LevelMetadata
        {
            displayName = "Jane Doe",
            artist = "Kenshi Yonezu, Hikaru Utada",
            folderName = "janedoe",
            difficulties =  2,
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
        new LevelMetadata
        {
            displayName = "Yuusha",
            folderName = "yuusha",
            artist = "YOASOBI",
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
        new LevelMetadata
        {
            displayName = "HITOMANIA",
            folderName = "hitomania",
            artist = "Sasuke Haraguchi",
            difficulties = 2,
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
        new LevelMetadata
        {
            displayName = "MEGALOVANIA",
            folderName = "megalovania",
            artist = "Toby Fox",
        },
        new LevelMetadata
        {
            displayName = "Shinzou wo Sasageyo!",
            folderName = "sasageyo",
            artist = "Linked Horizon",
        },
        new LevelMetadata
        {
            displayName = "SPECIALZ",
            folderName = "specialz",
            artist = "King Gnu",
            difficulties = 2,
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
        new LevelMetadata
        {
            displayName = "IDOL",
            folderName = "idol",
            artist = "YOASOBI",
            difficulties = 2,
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
        new LevelMetadata
        {
            displayName = "il vento d'oro",
            folderName = "goldenwind",
            artist = "Yugo Kanno",
            difficulties = 2,
        },
        new LevelMetadata
        {
            displayName = "Otonoke",
            folderName = "otonoke",
            artist = "Creepy Nuts",
            difficulties = 4,
            audioType = AudioType.OGGVORBIS,
            fileExtension = "ogg",
        },
    };
}

public enum Level
{
    HowYouLikeThat,
    DduDuDduDu,
    Apt,
    KillThisLove,
    Fancy,
    IrisOut,
    JaneDoe,
    Yuusha,
    Hitomania,
    Megalovania,
    Sasageyo,
    Specialz,
    Idol,
    GoldenWind,
    Otonoke,
}

public enum LevelDifficulty
{
    Easy = 0,
    Medium,
    Hard,
    Expert,
    Insane,
}

public class LevelMetadata
{
    public string displayName;
    public string artist;
    public string folderName; // e.g. "irisout"
    public int difficulties = 3;
    public AudioType audioType = AudioType.MPEG;
    public string fileExtension = "mp3";
}
