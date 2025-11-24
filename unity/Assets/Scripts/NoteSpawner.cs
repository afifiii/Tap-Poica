using System;
using UnityEngine;

[Serializable]
public class NoteData
{
    public float time; // when the note should be hit (seconds)
    public bool isLongNote;
}

public class NoteSpawner :MonoBehaviour
{
    public GameObject shortNotePrefab;
    public GameObject longNotePrefab;
    public NoteData[] notes;
    public float spawnLeadTime = 2f; // how early to spawn before it reaches button
    public AudioSource music;

    int _nextIndex;

    void Update()
    {
        if(_nextIndex >= notes.Length) return;

        var songTime = music.time;

        if(songTime + spawnLeadTime < notes[_nextIndex].time) return;
        var data = notes[_nextIndex];

        var prefabToSpawn = data.isLongNote ? longNotePrefab : shortNotePrefab;
        Instantiate(prefabToSpawn, transform.position, Quaternion.identity);

        _nextIndex++;
    }
}
