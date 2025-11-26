using System.Collections.Generic;
using UnityEngine;

namespace OsuParser
{
    public class OsuBeatmap
    {
        public double beatLength;
        public double globalBpm;
        public AudioClip audioClip;
        public readonly List<HitObject> hitObjects = new();
    }
}
