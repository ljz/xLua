﻿using  UnityEngine;
using  UnityEngine.Playables;
using UnityEngine.UI;

public  class TestExample : PlayableBehaviour
{
    public GameObject m_MySceneObject;
    public Vector3 m_SceneObjectVelocity;
    public string targetStr;

    public override void PrepareFrame(Playable playable, FrameData frameData)
    {
        //If the Scene GameObject exists, move it continuously until the Playable pauses
        if (m_MySceneObject != null)
            //Move the GameObject using the velocity you set in your Playable Track's inspector
            // m_MySceneObject.transform.Translate(m_SceneObjectVelocity);
            m_MySceneObject.GetComponent<Text>().text = targetStr;
    }
}