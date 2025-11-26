using System;
using UnityEngine;

public class LightstickInput :MonoBehaviour
{
    public ButtonController button; // drag your button here
    LightStickData _lightStickData;
    double _packetDelay;

    [Flags]
    enum LightStickData
    {
        Tap = 1 << 0,
        ShakeStart = 1 << 1,
        ShakeEnd = 1 << 2
    }

    void Update()
    {
        if(_lightStickData.HasFlag(LightStickData.Tap))
        {
            Debug.Log("TAP");
            button.OnTapFromController();
            _lightStickData &= ~LightStickData.Tap; // Clear the flag after processing
        }

        if(_lightStickData.HasFlag(LightStickData.ShakeStart))
        {
            Debug.Log("HOLD START");
            button.OnHoldStartFromController();
            _lightStickData &= ~LightStickData.ShakeStart; // Clear the flag after processing
        }

        if(_lightStickData.HasFlag(LightStickData.ShakeEnd))
        {
            Debug.Log("HOLD END");
            button.OnHoldEndFromController();
            _lightStickData &= ~LightStickData.ShakeEnd; // Clear the flag after processing
        }
    }

    void UpdateFromPacket(LightStickPacket data)
    {
        _lightStickData = (LightStickData)data.data;
        _packetDelay = data.delay;
    }
}

struct LightStickPacket
{
    public double delay; // delay in ms
    public byte data; // lightstick data (tap/shake)
}
