using UnityEngine;

public class EffectObject :MonoBehaviour
{
    public float lifetime = 1f;

    void Update()
    {
        Destroy(gameObject, lifetime);
    }
}
