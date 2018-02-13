using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;

public class ToDoTest : MonoBehaviour {
    LuaEnv luaEnv = null;

    string script = @"
            a = 1
            b = 'hello world '
            c = true
            d = {
                f1 = 12, f2 = 13,
                1, 2, 3,
                add = function(self, a, b)
                    print('d.add is called')
                    return a + b
                end
            }

            function e()
                print('i am e')
            end

            function f(a, b)
                print('a', a, 'b', b)
                return 1, {f1 = 1024}
            end

            function ret_e()
                print('ret_e called')
                return e
            end
        ";

    public class DClass
    {
        public int f1;
        public int f2;
    }

    [CSharpCallLua]
    public interface ItfD
    {
        int f1 { get; set; }
        int f2 { get; set; }
        int add(int a, int b);
    }

    [CSharpCallLua]
    public delegate Action GetE();

    [CSharpCallLua]
    public delegate int FDelegate(int a, string b, out DClass c);

    // Use this for initialization
    void Start () {
        Debug.Log("in ToDoTest Start");
        luaEnv = new LuaEnv();

        //1.执行简单的lua代码
        //luaEnv.DoString("print('lua code in ToDoTest')");

        //2.加载lua文件，这里这种方式只能够加载Resources文件夹下面的，并且是lua.txt类型的文件，感觉没啥乱用。
        //文档你说的是Resources文件夹下面的才需要加txt后缀，那么就是说当前文件夹下面的不需要。但是实验得出的是找不到模块。什么鬼？
        //luaEnv.DoString("require('testloadfile')");
        //luaEnv.DoString("require('testloadfile2')");




        //3.自定义Loader
        //luaEnv.AddLoader((ref string filename) =>
        //{
        //    if (filename == "InMemory")
        //    {
        //        string script = "return {ccc = 999666}";
        //        return System.Text.Encoding.UTF8.GetBytes(script);
        //    }
        //    return null;
        //});
        //luaEnv.DoString("print('InMemory.cc=',require('InMemory').ccc) ");

        //4.C# Call Lua
        luaEnv.DoString(script);

        Debug.Log("_G.a =" + luaEnv.Global.Get<int>("a"));
        Debug.Log("_G.b =" + luaEnv.Global.Get<string>("b"));
        Debug.Log("_G.c =" + luaEnv.Global.Get<bool>("c"));

        //映射到有对应字段的Class， by value
        DClass d = luaEnv.Global.Get<DClass>("d");
        Debug.Log("_G.d = {f1=" + d.f1 + ", f2 = " + d.f2 + "}");

        //映射到Dictinoary<string, double>, by value
        Dictionary<string, double> d1 = luaEnv.Global.Get<Dictionary<string, double>>("d");
        Debug.Log("_G.d = {f1=" + d1["f1"] + ", f2=" + d1["f2"] + "}");

        //映射到List<double>,by value
        List<double> d2 = luaEnv.Global.Get<List<double>>("d");
        Debug.Log("_G.d = {f1 =" + d2[1] + "f2=" + d2[2]+"},_G.d.len = "+d2.Count);
        

        //映射到interface实例，by ref,这个要求将interface加入到生成列表。否则会返回null，建议用法。
        ItfD d3 = luaEnv.Global.Get<ItfD>("d");
        d3.f2 = 1000;
        Debug.Log("_G.d = {f1 =" + d3.f1 + "f2=" + d3.f2 + "}");
        Debug.Log("_G.d:add(1, 2) = "+ d3.add(1, 2) );

        //映射到LuaTable， by ref
        LuaTable d4 = luaEnv.Global.Get<LuaTable>("d");
        Debug.Log("_G.d = {f1 = " + d4.Get<int>("f1") + ", f2 = " + d4.Get<int>("f2") + "}");

        //映射到一个delegate， 要求delegate加到生成列表。建议用法。
        Action e = luaEnv.Global.Get<Action>("e");
        e();

        FDelegate f = luaEnv.Global.Get<FDelegate>("f");
        DClass d_ret;

        //lua的多返回值的映射：从左往右映射到C#的输出参数，输出参数包括返回值，out参数，ref参数。
        int f_ret = f(100, "John", out d_ret);
        Debug.Log("ret.d = {f1 = " + d_ret.f1 + ", f2 = " + d_ret.f2 + "}, ret =" + f_ret);

        GetE ret_e = luaEnv.Global.Get<GetE>("ret_e");
        e = ret_e();

        e();

        LuaFunction d_e = luaEnv.Global.Get<LuaFunction>("e");
        d_e.Call();


        //5.lua调用C#

    }
	
	// Update is called once per frame
	void Update () {
        if (luaEnv != null)
        {
            luaEnv.Tick();
        }
		
	}
}
