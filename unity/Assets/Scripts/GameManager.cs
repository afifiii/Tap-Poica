using System.Globalization;
using BLE;
using BLE.Commands;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class GameManager :MonoBehaviour
{
    public static GameManager Instance;

    [Header("Audio & Gameplay")] public AudioSource audioSource;
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

    [Header("UI Elements")] public TextMeshProUGUI scoreTxt;
    public TextMeshProUGUI multiTxt;

    [Header("Results UI")] public GameObject resultsScreen;
    public TextMeshProUGUI percentHitTxt;
    public TextMeshProUGUI normalHitTxt;
    public TextMeshProUGUI goodHitTxt;
    public TextMeshProUGUI perfectHitTxt;
    public TextMeshProUGUI missedHitTxt;
    public TextMeshProUGUI rankTxt;
    public TextMeshProUGUI finalScoreText;

    [Header("Stats Tracking")] public float totalNotes;
    public float normalHits;
    public float goodHits;
    public float perfectHits;
    public float missedHits;

    byte _sensorPacket;
    string _deviceName;
    string _deviceAddress;

    const string DeviceName = "TapPiocaController";
    const string ServiceUuid = "67676701-6767-6767-6767-676767676767";
    const string CharacteristicUuid = "67676702-6767-6767-6767-676767676767";

    void Start()
    {
        Instance = this;

        scoreTxt.text = "Score: 0";
        currentMultiplier = 1;

        totalNotes = FindObjectsByType<NoteObject>(FindObjectsSortMode.None).Length;

        // ✅ Hide results at start
        resultsScreen.SetActive(false);

        // Ensure the music won't loop forever
        audioSource.loop = false;


        // Example BLE usage: Initialize BLE manager
        if(!BleManager.IsInitialized)
        {
            BleManager.Instance.Initialize();
        }

        BleManager.Instance.QueueCommand(new DiscoverDevices(OnDeviceFound));
    }
    static void OnDeviceFound(string deviceAddress, string deviceName)
    {
        if(string.Equals(deviceName, DeviceName))
            BleManager.Instance.QueueCommand(new ConnectToDevice(deviceAddress, OnDeviceConnected));
    }

    static void OnDeviceConnected(string deviceAddress)
    {
        Debug.Log("Connected to device: " + deviceAddress);
        BleManager.Instance.QueueCommand(new SubscribeToCharacteristic(deviceAddress, ServiceUuid, CharacteristicUuid,
            OnCharacteristicSubscribed, true));
    }

    static void OnCharacteristicSubscribed(byte[] data)
    {
        if(data.Length is <= 0 or > 1) return;

        Instance._sensorPacket = data[0];
        Debug.Log(data[0].ToString());
        // Process sensor data as needed
    }

    void Update()
    {
        if(!_startingPoint && Input.anyKeyDown)
        {
            _startingPoint = true;
            beatScroller.hasStarted = true;
            audioSource.Play();
            return;
        }

        // ✅ Only show results when song ends, and only once
        if(_resultsShown || audioSource.isPlaying) return;
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

        // ✅ Rank calculation

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
    }

    // --- Scoring System ---
    void NoteHit()
    {
        Debug.Log("Hit on time.");

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
        Debug.Log("Missed note.");
        currentMultiplier = 1;
        multiplierTracker = 0;
        multiTxt.text = "Multiplier: x" + currentMultiplier;
        missedHits++;
    }
}
