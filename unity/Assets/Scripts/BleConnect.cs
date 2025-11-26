using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class Demo : MonoBehaviour
{
    public bool isScanningDevices = false;
    public bool isScanningServices = false;
    public bool isScanningCharacteristics = false;
    public bool isSubscribed = false;
    public Text deviceScanButtonText;
    public Text deviceScanStatusText;
    public GameObject deviceScanResultProto;
    public Text errorText;
    public Text TextSubscribe;

    Transform scanResultRoot;
    public readonly string deviceName = "TapPioca";
    public string deviceId;
    public readonly string serviceId = "67676767-6702-6767-6767-676767676767";
    public readonly string characteristicId = "TODO";


    Dictionary<string, Dictionary<string, string>> devices = new Dictionary<string, Dictionary<string, string>>();
    Dictionary<string, string> characteristicNames = new Dictionary<string, string>();
    string lastError;

    // Start is called before the first frame update
    void Start()
    {
        scanResultRoot = deviceScanResultProto.transform.parent;
        deviceScanResultProto.transform.SetParent(null);
    }

    // Update is called once per frame
    void Update()
    {
        if (isScanningDevices)
        {
            ScanDevices();
        }
        if (isScanningServices)
        {
            ScanServices();
        }
        if (isScanningCharacteristics)
        {
            ScanCharacteristics();
        }
        if (isSubscribed)
        {
            BleApi.BLEData res = new BleApi.BLEData();
            while (BleApi.PollData(out res, false))
            {
                TextSubscribe.text = BitConverter.ToString(res.buf, 0, res.size);
                // TextSubscribe.text = Encoding.ASCII.GetString(res.buf, 0, res.size);
            }
        }

        {
            // log potential errors
            BleApi.ErrorMessage res = new BleApi.ErrorMessage();
            BleApi.GetError(out res);
            if (lastError != res.msg)
            {
                Debug.LogError(res.msg);
                errorText.text = res.msg;
                lastError = res.msg;
            }
        }
    }

    private void OnApplicationQuit()
    {
        BleApi.Quit();
    }

    public void StartStopDeviceScan()
    {
        if (!isScanningDevices)
        {
            // start new scan
            devices.Clear();
            for (int i = scanResultRoot.childCount - 1; i >= 0; i--)
            {
                Destroy(scanResultRoot.GetChild(i).gameObject);
            }
            BleApi.StartDeviceScan();
            isScanningDevices = true;
            deviceScanButtonText.text = "Stop scan";
            deviceScanStatusText.text = "scanning";
        }
        else
        {
            // stop scan
            isScanningDevices = false;
            BleApi.StopDeviceScan();
            deviceScanButtonText.text = "Start scan";
            deviceScanStatusText.text = "stopped";
        }
    }

    private void ScanDevices()
    {
        BleApi.DeviceUpdate res = new BleApi.DeviceUpdate();
        while (true)
        {
            // Non-blocking poll
            BleApi.ScanStatus status = BleApi.PollDevice(ref res, false);
            if (status == BleApi.ScanStatus.FINISHED)
            {
                isScanningDevices = false;
                deviceScanButtonText.text = "Scan devices";
                deviceScanStatusText.text = "No device";
            }
            if (status != BleApi.ScanStatus.AVAILABLE)
            {
                break;
            }

            if (devices.ContainsKey(res.id))
            {
                continue;
            }
            if (!devices.ContainsKey(res.id))
            {
                devices[res.id] = new Dictionary<string, string>() {
                    { "name", "" },
                    { "isConnectable", "False" }
                };
            }
            if (res.nameUpdated)
            {
                devices[res.id]["name"] = res.name;
            }
            if (res.isConnectableUpdated)
            {
                devices[res.id]["isConnectable"] = res.isConnectable.ToString();
            }
            // Consider only devices which have the right name and which are connectable
            if (devices[res.id]["name"] == deviceName && devices[res.id]["isConnectable"] == "True")
            {
                // This is our device
                deviceId = res.id;
                StartStopDeviceScan();
                deviceScanStatusText.text = "connecting...";
                StartServiceScan();
                break;
            }
        }
    }

    public void StartServiceScan()
    {
        if (!isScanningServices)
        {
            // start new scan
            BleApi.ScanServices(deviceId);
            isScanningServices = true;
        }
    }

    private void ScanServices()
    {
        BleApi.Service res = new BleApi.Service();
        while (true)
        {
            BleApi.ScanStatus status = BleApi.PollService(out res, false);
            if (status == BleApi.ScanStatus.FINISHED)
            {
                isScanningServices = false;
                deviceScanStatusText.text = "failed";
            }
            if (status != BleApi.ScanStatus.AVAILABLE)
            {
                break;
            }

            if (res.uuid == serviceId)
            {
                // Found our service
                isScanningServices = false;
                StartCharacteristicScan();
                break;
            }
        }
    }

    public void StartCharacteristicScan()
    {
        if (!isScanningCharacteristics)
        {
            BleApi.ScanCharacteristics(deviceId, serviceId);
            isScanningCharacteristics = true;
        }
    }

    private void ScanCharacteristics()
    {
        BleApi.Characteristic res = new BleApi.Characteristic();
        while (true)
        {
            BleApi.ScanStatus status = BleApi.PollCharacteristic(out res, false);
            if (status == BleApi.ScanStatus.FINISHED)
            {
                isScanningCharacteristics = false;
                deviceScanStatusText.text = "failed";
            }
            if (status != BleApi.ScanStatus.AVAILABLE)
            {
                break;
            }

            if (res.uuid == characteristicId)
            {
                // Found our characteristic
                isScanningCharacteristics = false;
                Subscribe();
                deviceScanStatusText.text = "connected";
                break;
            }
        }
    }

    public void Subscribe()
    {
        // no error code available in non-blocking mode
        BleApi.SubscribeCharacteristic(deviceId, serviceId, characteristicId, false);
        isSubscribed = true;
    }

    public void Write()
    {
        byte[] payload = Encoding.ASCII.GetBytes("stuff to write");
        BleApi.BLEData data = new BleApi.BLEData();
        data.buf = new byte[512];
        data.size = (short)payload.Length;
        data.deviceId = deviceId;
        data.serviceUuid = serviceId;
        data.characteristicUuid = characteristicId;
        for (int i = 0; i < payload.Length; i++)
            data.buf[i] = payload[i];
        // no error code available in non-blocking mode
        BleApi.SendData(in data, false);
    }
}
