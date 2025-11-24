using System.Globalization;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class GameManager :MonoBehaviour
{
    public static GameManager Instance;

    [Header("Audio & Gameplay")] public AudioSource theMusic;
    public BeatScroller beatScroller;
    bool _startingPoint;
    bool _resultsShown;

    [Header("Score Settings")] public int currentScore;
    public int scorePerNote = 100;
    public int scorePerGoodNote = 125;
    public int scorePerPerfectNote = 150;

    [Header("Multiplier Settings")] public int currentMultiplier;
    public int multiplierTracker;
    public int[] multiplierThresholds;

    [Header("UI Elements")] public Text scoreTxt;
    public Text multiTxt;

    [Header("Results UI")] public GameObject resultsScreen;
    public Text percentHitTxt;
    public Text normalHitTxt;
    public Text goodHitTxt;
    public Text perfectHitTxt;
    public Text missedHitTxt;
    public Text rankTxt;
    public Text finalScoreText;

    [Header("Next Level Button")] public Button nextLevelButton;

    [Header("Stats Tracking")] public float totalNotes;
    public float normalHits;
    public float goodHits;
    public float perfectHits;
    public float missedHits;

    void Start()
    {
        Instance = this;

        scoreTxt.text = "Score: 0";
        currentMultiplier = 1;

        totalNotes = FindObjectsByType<NoteObject>(FindObjectsSortMode.None).Length;

        resultsScreen.SetActive(false);
        nextLevelButton.gameObject.SetActive(false);
        theMusic.loop = false;
    }

    void Update()
    {
        if(_startingPoint || Input.anyKeyDown) return;
        _startingPoint = true;
        beatScroller.hasStarted = true;
        theMusic.Play();

        if(_resultsShown || theMusic.isPlaying) return;
        ShowResults();
        _resultsShown = true;
    }

    void ShowResults()
    {
        resultsScreen.SetActive(true);

        normalHitTxt.text = normalHits.ToString(CultureInfo.CurrentCulture);
        goodHitTxt.text = goodHits.ToString(CultureInfo.CurrentCulture);
        perfectHitTxt.text = perfectHits.ToString(CultureInfo.CurrentCulture);
        missedHitTxt.text = missedHits.ToString(CultureInfo.CurrentCulture);

        var totalHit = normalHits + goodHits + perfectHits;
        var percentHit = (totalNotes > 0) ? (totalHit / totalNotes) * 100f : 0f;
        percentHitTxt.text = percentHit.ToString("F1") + "%";

        var rankVal = percentHit switch
        {
            > 95 => "S",
            > 85 => "A",
            > 70 => "B",
            > 55 => "C",
            > 40 => "D",
            _ => "F"
        };
        rankTxt.text = rankVal;

        finalScoreText.text = currentScore.ToString();

        nextLevelButton.gameObject.SetActive(true);
    }

// --- Hit & Hold Notes Programmatically ---
    public void HitNote()
    {
        var notes = FindObjectsByType<NoteObject>(FindObjectsSortMode.None);
        NoteObject closest = null;
        var bestDist = Mathf.Infinity;

        foreach (var n in notes)
        {
            var d = Mathf.Abs(n.transform.position.y);
            if(!n.canBePressed && d >= bestDist) continue;
            bestDist = d;
            closest = n;
        }

        if(!closest) return;
        closest.Pressed();
    }

    public void HoldStart()
    {
        foreach (var n in FindObjectsByType<NoteObject>(FindObjectsSortMode.None))
        {
            if(!n.canBePressed || !n.isLongNote) continue;
            n.HoldStart();
            return;
        }
    }

    public void HoldEnd()
    {
        foreach (var n in FindObjectsByType<NoteObject>(FindObjectsSortMode.None))
        {
            if(!n.isBeingHeld) continue;
            n.HoldEnd();
            return;
        }
    }

// --- Scoring System ---
    public void NoteHit()
    {
        if(currentMultiplier - 1 < multiplierThresholds.Length)
        {
            multiplierTracker++;

            if(multiplierThresholds[currentMultiplier - 1] <= multiplierTracker)
            {
                multiplierTracker = 0;
                currentMultiplier++;
            }
        }

        multiTxt.text = "Multiplier: x" + currentMultiplier;
        scoreTxt.text = "Score: " + currentScore;
    }

    public void NormalHit()
    {
        currentScore += scorePerNote * currentMultiplier;
        NoteHit();
        normalHits++;
    }

    public void GoodHit()
    {
        currentScore += scorePerGoodNote * currentMultiplier;
        NoteHit();
        goodHits++;
    }

    public void PerfectHit()
    {
        currentScore += scorePerPerfectNote * currentMultiplier;
        NoteHit();
        perfectHits++;
    }

    public void NoteMissed()
    {
        currentMultiplier = 1;
        multiplierTracker = 0;
        multiTxt.text = "Multiplier: x" + currentMultiplier;
        missedHits++;
    }

// ----------------------------------------------------------
// NEXT LEVEL BUTTON → Load next scene
// ----------------------------------------------------------
    public void LoadNextLevel()
    {
        Time.timeScale = 1f;

        var currentIndex = SceneManager.GetActiveScene().buildIndex;
        var nextIndex = currentIndex + 1;

        if(nextIndex < SceneManager.sceneCountInBuildSettings)
        {
            SceneManager.LoadScene(nextIndex);
        }
        else
        {
            Debug.Log("Reached the last level!");
        }
    }

// ----------------------------------------------------------
// EXIT LAST LEVEL → Load specific scene (e.g. Main Menu)
// ----------------------------------------------------------
    public void ExitToScene(string sceneName)
    {
        Time.timeScale = 1f;
        SceneManager.LoadScene(sceneName);
    }
}
