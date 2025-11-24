using UnityEngine;

public class NoteObject :MonoBehaviour
{
    [Header("Note Settings")] public bool canBePressed;
    public bool isLongNote;
    public bool isBeingHeld;
    public KeyCode keyToPress;

    [Header("Effects")] public GameObject hitEffect, goodEffect, perfectEffect, missEffect;

    void Update()
    {
        // Optional manual key press //to change
        if(!Input.GetKeyDown(keyToPress) || !canBePressed) return;
        Pressed();
    }

    // Called when a single note is hit
    public void Pressed() //to change - timely pressed
    {
        if(!canBePressed) return;

        var yDist = Mathf.Abs(transform.position.y);

        switch (yDist)
        {
            case > 0.5f:
            {
                Debug.Log("Good!");
                GameManager.Instance.GoodHit();
                if(goodEffect) Instantiate(goodEffect, transform.position, goodEffect.transform.rotation);
                break;
            }
            case > 0.25f:
            {
                Debug.Log("Hit!");
                GameManager.Instance.NormalHit();
                if(hitEffect) Instantiate(hitEffect, transform.position, hitEffect.transform.rotation);
                break;
            }
            default:
            {
                Debug.Log("Perfect!");
                GameManager.Instance.PerfectHit();
                if(perfectEffect) Instantiate(perfectEffect, transform.position, perfectEffect.transform.rotation);
                break;
            }
        }

        canBePressed = false;
        Destroy(gameObject);
    }

    // Called when player starts holding a long note
    public void HoldStart()
    {
        if(!isLongNote) return;
        isBeingHeld = true;
        canBePressed = false;
    }

    // Called when player stops holding a long note
    public void HoldEnd()
    {
        if(!isBeingHeld) return;
        isBeingHeld = false;

        GameManager.Instance.GoodHit(); // Example: reward for holding
        Destroy(gameObject);
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if(!other.CompareTag("Activator")) return;
        canBePressed = true;
    }

    void OnTriggerExit2D(Collider2D other)
    {
        if(!other.CompareTag("Activator")) return;
        canBePressed = false;
        GameManager.Instance.NoteMissed();
        if(missEffect) Instantiate(missEffect, transform.position, missEffect.transform.rotation);
    }
}
