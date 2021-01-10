using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;

/// <summary>
/// ZumoControlScript
/// </summary>
public class ZumoControlScript : MonoBehaviour
{
    public RawImage cameraImage;
    public InputField ipAddressText;
    public Slider powerSlider;
    public SampleWebView webView;

    public enum ZUMO_MODE
    {
        Front,
        Back,
        Left,
        Right,
        Round,
        Camera
    }


    /// <summary>
    /// Start
    /// </summary>
    void Start()
    {
        if (ipAddressText != null)
        {
            var address = PlayerPrefs.GetString("address");
            ipAddressText.text = string.IsNullOrEmpty(address) ? "http://192.168.10.12" : address;
        }
    }

    /// <summary>
    /// OnApplicationQuit
    /// </summary>
    private void OnApplicationQuit()
    {
        if (ipAddressText != null)
        {
            PlayerPrefs.SetString("address", ipAddressText.text);
        }
    }


    /// <summary>
    /// CreateSpriteFromByte
    /// </summary>
    /// <param name="data"></param>
    /// <returns></returns>
    protected Sprite CreateSpriteFromByte(byte[] data)
    {
        var tex = new Texture2D(320, 240);
        tex.LoadImage(data);

        return Sprite.Create(tex, new Rect(0, 0, 320, 240), Vector2.zero);
    }

    public Texture readByBinary(byte[] bytes)
    {
        var texture = new Texture2D(1, 1);
        texture.LoadImage(bytes);
        return texture;
    }


    /// <summary>
    /// SendZumo
    /// </summary>
    protected IEnumerator SendZumo(ZUMO_MODE mode)
    {
        if (ipAddressText != null && powerSlider != null)
        {
            var url = ipAddressText.text;
            var power = powerSlider.value * 600f;
            if (power <= 10)
            {
                power = 50;
            }

            switch (mode)
            {
                case ZUMO_MODE.Front:
                    url += string.Format("/foward/{0}", (int)power);
                    break;

                case ZUMO_MODE.Back:
                    url += string.Format("/back/{0}", (int)power);
                    break;

                case ZUMO_MODE.Left:
                    url += string.Format("/left/{0}", (int)power);
                    break;

                case ZUMO_MODE.Right:
                    url += string.Format("/right/{0}", (int)power);
                    break;

                case ZUMO_MODE.Round:
                    url += string.Format("/auto/{0}", (int)power);
                    break;

                case ZUMO_MODE.Camera:
                    url += "/camera";
                    break;

            }

            Debug.Log(url);


            // texture
            if (mode == ZUMO_MODE.Camera && cameraImage != null)
            {
                webView.Url = url;
                webView.LoadContent();

            }
            else
            {
                WWW www = new WWW(url);
                yield return www;
            }
        }
    }


    public void FrontAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Front));
    }

    public void BackAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Back));
    }

    public void LeftAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Left));
    }

    public void RightAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Right));
    }

    public void RoundAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Round));
    }

    public void CameraAction()
    {
        StartCoroutine(SendZumo(ZUMO_MODE.Camera));
    }
}

