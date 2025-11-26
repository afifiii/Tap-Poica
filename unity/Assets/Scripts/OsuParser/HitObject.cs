using System;
using System.Collections.Generic;
using UnityEngine;

namespace OsuParser
{
    public abstract class HitObject
    {

        public int X { get; set; }
        public int Y { get; set; }
        public int Time { get; set; } // ms
        public HitObjectType Type { get; set; }

        public int HitSound { get; set; }

        // We can keep HitSample as a string for now since you didn't specify parsing it deep
        public string HitSample { get; set; }
    }

    [Flags]
    public enum HitObjectType
    {
        Circle = 1 << 0, // 1
        Slider = 1 << 1, // 2
        Spinner = 1 << 3, // 8
        ManiaHold = 1 << 7 // 128
    }

    public class Slider :HitObject
    {
        public char CurveType { get; set; }
        public List<Vector2> CurvePoints { get; set; } // Keeping as string "x:y" for now
        public int Slides { get; set; }
        public double Length { get; set; }
    }

    public class Spinner :HitObject
    {
        public int EndTime { get; set; }
    }

    public class ManiaHold :HitObject
    {
        public int EndTime { get; set; } // ms
    }

    public class HitCircle :HitObject
    {
    }
}
