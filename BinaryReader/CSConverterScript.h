#pragma once

const char* pScript = R"(
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[InitializeOnLoad]
public static class TextureLoader
{
	static private string sJson = @"%s";
	static private Texture2D pT2D;
	
	[System.Serializable]
	public class TextureInfo
	{
		public int iWidth;
		public int iHeight;
		public int iTextureFormat;
		public bool bMipChain;
		public bool bLinear;
		public string sPath;
		public TextureInfo(int _iWidth, int _iHeight, int _iTextureFormat, bool _bMipChain, bool _bLinear, string _sPath)
		{
			iWidth = _iWidth;
			iHeight = _iHeight;
			iTextureFormat = _iTextureFormat;
			bMipChain = _bMipChain;
			bLinear = _bLinear;
			sPath = _sPath;
		}
	}

	[System.Serializable]
	public class TextureArray
	{
		public List<TextureInfo> Textures;
	}
	
	// https://stackoverflow.com/questions/51315918/how-to-encodetopng-compressed-textures-in-unity
	private static Texture2D DeCompress(Texture2D source)
    {
        RenderTexture renderTex = RenderTexture.GetTemporary(
                    source.width,
                    source.height,
                    0,
                    RenderTextureFormat.Default,
                    RenderTextureReadWrite.Linear);

        Graphics.Blit(source, renderTex);
        RenderTexture previous = RenderTexture.active;
        RenderTexture.active = renderTex;
        Texture2D readableText = new Texture2D(source.width, source.height);
        readableText.ReadPixels(new Rect(0, 0, renderTex.width, renderTex.height), 0, 0);
        readableText.Apply();
        RenderTexture.active = previous;
        RenderTexture.ReleaseTemporary(renderTex);
        return readableText;
    }
	
    static TextureLoader()
    {
		Debug.Log("[PolygonRipper] Converting all Textures...");
		TextureArray pTextures = JsonUtility.FromJson<TextureArray>(sJson);
		foreach(var Texture in pTextures.Textures)
		{
			string sBase64Texture = File.ReadAllText(Texture.sPath);
		
			try {
				byte[] TextureData = Convert.FromBase64String(sBase64Texture);
				
				pT2D = new Texture2D(Texture.iWidth, Texture.iHeight, (TextureFormat)Texture.iTextureFormat, Texture.bMipChain, Texture.bLinear);
				pT2D.LoadRawTextureData(TextureData);
				pT2D.Apply();
				pT2D.wrapMode = TextureWrapMode.Repeat;
				
				Texture2D pT2DDecompressed = DeCompress(pT2D);
				
				string sOutputPath = Texture.sPath + ".png";
				File.WriteAllBytes(sOutputPath, pT2DDecompressed.EncodeToPNG());
				File.Delete(Texture.sPath);
			}
			catch(Exception ex)
			{
				continue;
			}
		}
		Debug.Log("[PolygonRipper] Texture Converted!");
    }
}
)";