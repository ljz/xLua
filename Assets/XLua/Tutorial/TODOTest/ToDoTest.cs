using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using XLua;
using LuaAPI = XLua.LuaDLL.Lua;


public class ToDoTest : MonoBehaviour {
    public static string errorMsg;
    protected LuaFunction onUpdateNetworkStateFunc = null;
    protected LuaFunction tickUpdateFunc = null;
    protected LuaFunction fixedUpdateFunc = null;
    protected LuaFunction lateUpdateFunc = null;
    protected LuaFunction onApplicationFocusFunc = null;
    protected LuaFunction onApplicationPauseFunc = null;
    protected LuaFunction onApplicationQuitFunc = null;
    protected LuaTable LuaSDKMgr = null;
    protected LuaFunction runGM = null;
    protected LuaFunction sendBackError = null;
    protected LuaFunction updateCallBack = null;
    internal static LuaEnv luaEnv = null;
    public static ToDoTest instance = null;
    private int m_networkState = -1;


    public static void SetErrorMsg(string msg)
    {
        errorMsg = msg;
    }

    public static LuaEnv GetMainState()
    {
        return luaEnv;
    }

    private void Awake()
    {
        Debug.Log("LuaClient Awake !!!");
        luaEnv = new LuaEnv();
        instance = this;
        DontDestroyOnLoad(gameObject);
    }


    //string script = @"
    //        a = 1
    //        b = 'hello world '
    //        c = true
    //        d = {
    //            f1 = 12, f2 = 13,
    //            1, 2, 3,
    //            add = function(self, a, b)
    //                print('d.add is called')
    //                return a + b
    //            end
    //        }

    //        function e()
    //            print('i am e')
    //        end

    //        function f(a, b)
    //            print('a', a, 'b', b)
    //            return 1, {f1 = 1024}
    //        end

    //        function ret_e()
    //            print('ret_e called')
    //            return e
    //        end
    //    ";

    //public class DClass
    //{
    //    public int f1;
    //    public int f2;
    //}

    //[C11111SharpCallLua]
    //public interface ItfD
    //{
    //    int f1 { get; set; }
    //    int f2 { get; set; }
    //    int add(int a, int b);
    //}

    //[C111SharpCallLua]
    //public delegate Action GetE();

    //[C111SharpCallLua]
    //public delegate int FDelegate(int a, string b, out DClass c);

    // Use this for initialization
    void Start () {
        Debug.Log("in ToDoTest Start");

        //这里没有InitEngine，有可能是发哥改造过lua虚拟机才会有。
        //LuaAPI.InitEngine(luaEnv.L);//这里的初始化是做了什么事情了？为什么不是new LuaEnv就能直接使用了？到底做了什么？(先不管。)

        //List<string> paths = new List<string>() {
        //        XLuaConst.luaResDir,
        //        Path.Combine(XLuaConst.luaResDir, "client"),
        //        Path.Combine(XLuaConst.luaResDir, "globalcommon"),
        //        "script",
        //        "script/client",
        //        "script/globalcommon",
        //        XLuaConst.luaDir,
        //        Path.Combine(XLuaConst.luaDir, "client"),
        //        Path.Combine(XLuaConst.luaDir, "globalcommon"),
        //    };
        //这个接口也需要改造之后才有。估计是在初始化中增加的。
        //LuaAPI.AddSearchPath(path);

        //1.执行简单的lua代码
        //luaEnv.DoString("print('lua code in ToDoTest')");

        //2.加载lua文件，这里这种方式只能够加载Resources文件夹下面的，并且是lua.txt类型的文件，感觉没啥乱用。
        //文档你说的是Resources文件夹下面的才需要加txt后缀，那么就是说当前文件夹下面的不需要。但是实验得出的是找不到模块。什么鬼？
        //我看FQA中说的是：如果要打包到安装包里面，就必须是lua.txt,如果不打到里面就随便什么后缀都可以，然后通过CustomLoader或者设置package.path去读这个目录。
        //3.自定义Loader
        luaEnv.AddLoader((ref string filename) =>
        {
            filename = Application.dataPath + "/XLua/Tutorial/TODOTest/Resources/" + filename.Replace('.', '/') + ".lua";
            if (File.Exists(filename))
            {
                var script = File.ReadAllText(filename);
                return System.Text.Encoding.Default.GetBytes(script);
            }
            else
            {
                Debug.Log("不存在这个文件" + filename);
                return null;
            }

        });
        //加载这个文件，进而加载一系列的lua文件。
        luaEnv.DoString("require('importlist')");
        luaEnv.DoString("require('main')");
        //luaEnv.DoString("require('testloadfile2')");

        //luaenv.AddLoader(new SignatureLoader(PUBLIC_KEY, (ref string filepath) =>
        //{
        //    filepath = Application.dataPath + "/XLua/Examples/10_SignatureLoader/" + filepath.Replace('.', '/') + ".lua";
        //    if (File.Exists(filepath))
        //    {
        //        return File.ReadAllBytes(filepath);
        //    }
        //    else
        //    {
        //        return null;
        //    }
        //}));




        //luaEnv.DoString("print('InMemory.cc=',require('InMemory').ccc) ");

        //4.C# Call Lua
        //luaEnv.DoString(script);

        //Debug.Log("_G.a =" + luaEnv.Global.Get<int>("a"));
        //Debug.Log("_G.b =" + luaEnv.Global.Get<string>("b"));
        //Debug.Log("_G.c =" + luaEnv.Global.Get<bool>("c"));

        //映射到有对应字段的Class， by value
        //DClass d = luaEnv.Global.Get<DClass>("d");
        //Debug.Log("_G.d = {f1=" + d.f1 + ", f2 = " + d.f2 + "}");

        //映射到Dictinoary<string, double>, by value
        //Dictionary<string, double> d1 = luaEnv.Global.Get<Dictionary<string, double>>("d");
        //Debug.Log("_G.d = {f1=" + d1["f1"] + ", f2=" + d1["f2"] + "}");

        //映射到List<double>,by value
        //List<double> d2 = luaEnv.Global.Get<List<double>>("d");
        //Debug.Log("_G.d = {f1 =" + d2[1] + "f2=" + d2[2]+"},_G.d.len = "+d2.Count);


        //映射到interface实例，by ref,这个要求将interface加入到生成列表。否则会返回null，建议用法。
        //ItfD d3 = luaEnv.Global.Get<ItfD>("d");
        //d3.f2 = 1000;
        //Debug.Log("_G.d = {f1 =" + d3.f1 + "f2=" + d3.f2 + "}");
        //Debug.Log("_G.d:add(1, 2) = "+ d3.add(1, 2) );

        //映射到LuaTable， by ref
        //LuaTable d4 = luaEnv.Global.Get<LuaTable>("d");
        //Debug.Log("_G.d = {f1 = " + d4.Get<int>("f1") + ", f2 = " + d4.Get<int>("f2") + "}");

        //映射到一个delegate， 要求delegate加到生成列表。建议用法。
        //Action e = luaEnv.Global.Get<Action>("e");
        //e();

        //FDelegate f = luaEnv.Global.Get<FDelegate>("f");
        //DClass d_ret;

        //lua的多返回值的映射：从左往右映射到C#的输出参数，输出参数包括返回值，out参数，ref参数。
        //int f_ret = f(100, "John", out d_ret);
        //Debug.Log("ret.d = {f1 = " + d_ret.f1 + ", f2 = " + d_ret.f2 + "}, ret =" + f_ret);

        //GetE ret_e = luaEnv.Global.Get<GetE>("ret_e");
        //e = ret_e();

        //e();

        //LuaFunction d_e = luaEnv.Global.Get<LuaFunction>("e");
        //d_e.Call();


        //5.lua调用C#

    }
	
	// Update is called once per frame
	void Update () {

        UpdateNetworkState();

        if (errorMsg != null)
        {
            LuaTbCallBack(errorMsg);
            errorMsg = null;
        }

        UpdateTick();

        // 清除Lua的未手动释放的LuaBase（比如，LuaTable， LuaFunction），以及其它一些事情。
        if (luaEnv != null)
        {
            luaEnv.Tick();
        }
	}

    void UpdateNetworkState()
    {
        //更新网络状态
        //print(">>>>>>更新网络状态");
        if (onUpdateNetworkStateFunc == null)
        {
            return;
        }

        int state = -1;
        if (Application.internetReachability == NetworkReachability.NotReachable)
        {
            //NotReachable
            state = 1;
        }
        else if (Application.internetReachability == NetworkReachability.ReachableViaCarrierDataNetwork)
        {
            //ReachableViaCarrierDataNetwork
            state = 2;
        }
        else if (Application.internetReachability == NetworkReachability.ReachableViaLocalAreaNetwork)
        {
            //ReachableViaLocalAreaNetwork
            state = 3;
        }
        //if (m_networkState != state)
        //{
        //    onUpdateNetworkStateFunc.Call(state);
        //    m_networkState = state;
        //}
    }

    public void LuaTbCallBack(string msg)
    {
        //C#层调用lua层的TB,报错。
        print("C#层调用lua层的TB,报错。");
        //if (sendBackError == null)
        //{
        //    Debug.LogError("LuaClient LuaTbCallBack is null");
        //    return;
        //}
        //else
        //{
        //    sendBackError.Call(msg);
        //}
    }


    void UpdateTick()
    {
        //print(">>>>>>>>>更新Tick");
        //if (tickUpdateFunc != null)
        //{
        //    tickUpdateFunc.Call();
        //}
    }

    protected virtual void CallLuaFunc(string strFunc)
    {
        LuaFunction luaFunc = luaEnv.Global.Get<LuaFunction>(strFunc);
        if (luaFunc == null)
        {
            Debug.LogError(string.Format("No LuaFunction with name {0}", strFunc));
            return;
        }
        luaFunc.Call();
        luaFunc.Dispose();
    }

}
