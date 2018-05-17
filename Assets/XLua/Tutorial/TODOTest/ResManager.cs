using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;
using System.IO;
using UnityEditor;

public class ResManager : MonoBehaviour {
	Dictionary<GameObject, UnityEngine.Object> loadedGameObject;

	public static ResManager instance;

	/// <summary>
	/// Awake is called when the script instance is being loaded.
	/// </summary>
	void Awake()
	{
        Debug.Log("ResManager:Awake is called when the script instance is being loaded.");
		instance  = this;
		loadedGameObject = new Dictionary<GameObject, UnityEngine.Object>();

	}


	public GameObject LoadGameObject(string prefabPath)
	{
		GameObject prefab = LoadAsset(prefabPath) as GameObject;
		GameObject go = Instantiate(prefab);
		Debug.Log("go==============="+go);
		loadedGameObject[go] = prefab;
		return go;
	}

	public UnityEngine.Object LoadAsset(string prefabPath, Type type = null)
	{
		if (type == null)
		{
			type = typeof(UnityEngine.Object);
		}

		var obj = AssetDatabase.LoadAssetAtPath(prefabPath, type);
		if (obj == null)
		{
			Debug.LogErrorFormat("the asset {0} not exist in the editor!", prefabPath);
		}
		Debug.Log("obj ====="+obj);
		return obj;
	}



}
