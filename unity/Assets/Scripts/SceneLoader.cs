using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneLoader :MonoBehaviour
{
    public void LoadNextScene()
    {
        var currentIndex = SceneManager.GetActiveScene().buildIndex;
        var nextIndex = currentIndex + 1;

        if(nextIndex < SceneManager.sceneCountInBuildSettings)
        {
            SceneManager.LoadScene(nextIndex);
            return;
        }

        Debug.Log("No more levels!");
    }
}
