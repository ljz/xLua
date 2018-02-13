using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;

namespace Tutorialtest
{
    [LuaCallCSharp]
    public class BaseClass
    {

        public static int BSF = 1;
        public static void BSFunc()
        {
            Debug.Log("Driven Static Func, BSF = "+ BSF);
        }

        public int BMF { get; set; }

        public void BMFunc()
        {
            Debug.Log("Driven Member Func, BMF = " + BMF);
        }

    }

    public struct Param1
    {
        public int x;
        public string y;
    }

    [LuaCallCSharp]
    public enum TestEnum
    {
        E1,
        E2
    }

    [LuaCallCSharp]
    public class DrivenClass : BaseClass
    {
        [LuaCallCSharp]
        public enum TestEnumInner
        {
            E3,
            E4
        }


        public void DMFunc()
        {
            Debug.Log("Driven Memver Func, DMF = " + DMF);
        }

        public int DMF { get; set;}




}

public class ToDoLuaCallCSharpTest : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
