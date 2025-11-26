using UnityEngine;

public class NoteObject : MonoBehaviour
{
    [Header("Note Settings")] public bool canBePressed;
    public NoteType noteType;
    public bool isBeingHeld;
    public KeyCode keyToPress;

    const float WindowGood = 0.5f;
    const float WindowNormal = 0.25f;

    [Header("Effects")] public GameObject hitEffect, goodEffect, perfectEffect, missEffect;

    float _speed;
    double _duration;

    public void Initialize(NoteData data, float speed)
    {
        _duration = data.durationMs;
        _speed = speed;

        if (noteType != NoteType.Long) return;
        var collider = gameObject.GetComponent<BoxCollider2D>();
        var line = gameObject.GetComponent<LineRenderer>();
        var ends = FindObjectsByType<CircleCollider2D>(FindObjectsSortMode.None);
        CircleCollider2D tail = null;

        foreach (var end in ends)
            if (end.CompareTag("Tail"))
                tail = end;

        var visualHeight = speed * (float)_duration / 1000f;
        Vector3[] pointPositions = new Vector3[2];
        line.GetPositions(pointPositions);
        // bar.transform.localScale = new Vector3(1, visualHeight, 1);
        pointPositions[1] = new Vector3(pointPositions[1].x, visualHeight, pointPositions[1].z);
        // bar.
        // bar.size = new Vector2(bar.size.x, visualHeight);

        if (!tail) return;
        tail.transform.localPosition =
            new Vector3(tail.transform.localPosition.x, visualHeight, tail.transform.localPosition.z);
        // tail.transform.position = new Vector3(tail.transform.position.x, tail.transform.position.y + visualHeight,
        //     tail.transform.position.z);
    }

    void Update()
    {
        transform.Translate(_speed * Time.deltaTime * Vector3.down);
        // Optional manual key press //to change
        if (!Input.GetKeyDown(keyToPress) || !canBePressed) return;
        Pressed();

        if (noteType != NoteType.Long) return;

        if (Input.GetKey(keyToPress) && canBePressed && !isBeingHeld)
        {
            // Logic to start holding if we hit the head successfully
            // (Usually called inside Pressed() if it's a long note)
        }

        if (!Input.GetKeyUp(keyToPress) || !isBeingHeld) return;
        HoldEnd();
    }

    // Called when a single note is hit
    public void Pressed() //to change - timely pressed
    {
        if (!canBePressed) return;

        var yDist = Mathf.Abs(transform.position.y);

        Judge(yDist);

        canBePressed = false;
        Destroy(gameObject);
    }

    // Called when player starts holding a long note
    public void HoldStart()
    {
        if (noteType != NoteType.Long) return;
        isBeingHeld = true;
        // canBePressed = false;
    }

    // Called when player stops holding a long note
    public void HoldEnd()
    {
        if (!isBeingHeld) return;
        isBeingHeld = false;

        // Calculate Tail Distance
        var headY = transform.position.y;
        var noteHeight = transform.localScale.y;
        var tailY = headY + noteHeight;

        var dist = Mathf.Abs(tailY); // assuming judgment line is at 0

        // Use EXACT same judgment as Pressed
        Judge(dist);

        GameManager.Instance.GoodHit(); // Example: reward for holding
        Destroy(gameObject);
    }

    void SpawnEffect(GameObject effectPrefab)
    {
        if (effectPrefab) Instantiate(effectPrefab, transform.position, effectPrefab.transform.rotation);
    }

    void Judge(float distance)
    {
        switch (distance)
        {
            case > WindowGood: // > 0.5
                // Debug.Log("Good!");
                GameManager.Instance.GoodHit();
                SpawnEffect(goodEffect);
                break;

            case > WindowNormal: // > 0.25
                // Debug.Log("Hit!");
                GameManager.Instance.NormalHit();
                SpawnEffect(hitEffect);
                break;

            default: // <= 0.25 (The best one)
                // Debug.Log("Perfect!");
                GameManager.Instance.PerfectHit();
                SpawnEffect(perfectEffect);
                break;
        }
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (!other.CompareTag("Activator")) return;
        canBePressed = true;
    }

    void OnTriggerExit2D(Collider2D other)
    {
        if (!other.CompareTag("Activator")) return;
        canBePressed = false;
        GameManager.Instance.NoteMissed();
        SpawnEffect(missEffect);
        if (missEffect) Instantiate(missEffect, transform.position, missEffect.transform.rotation);
    }
}
