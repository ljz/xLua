//#define DEBUG_RES_MANAGER

using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;
using System.IO;
#if UNITY_EDITOR
using UnityEditor;
#endif

//加载资源请求类
public class LoadAssetRequest
{
    public string assetPath;//资源路径
    private LuaFunction onComplete;//加载资源请求完成之后的回调函数
    //这个onCompleteCSharp,不知道是啥,完成了的CSharp回调函数??
    private UnityEngine.Events.UnityAction<UnityEngine.Object> onCompleteCSharp;

    //类型
    public Type type;

    //加载资源请求构造函数1
    //资源路径;资源类型;加载请求完成的回调函数
    public LoadAssetRequest(string assetPath, Type type, LuaFunction onComplete)
    {
        this.assetPath = assetPath;
        this.onComplete = onComplete;
        this.type = type;
        this.onCompleteCSharp = null;
    }

    //加载资源请求构造函数2
    //资源路径;资源类型;加载请求完成的CSharp回调函数
    public LoadAssetRequest(string assetPath, Type type, UnityEngine.Events.UnityAction<UnityEngine.Object> onComplete)
    {
        this.assetPath = assetPath;
        this.onComplete = null;
        this.type = type;
        this.onCompleteCSharp = onComplete;
    }

    //析构函数,回掉函数设为nil
    public void Release()
    {
        if (onComplete != null)
        {
            onComplete.Dispose();//应该是释放回掉函数
            onComplete = null;
        }
        if (onCompleteCSharp != null)
        {
            onCompleteCSharp = null;
        }
    }

    //回调,通过这个接口来调用回调函数
    //资源对象
    public bool Callback(UnityEngine.Object asset)
    {
        if (onComplete != null)
        {
            onComplete.Call(asset);//调用lua端的回调函数,资源对象作为参数
            onComplete.Dispose();//回调完了之后将回调函数的引用释放掉
            onComplete = null;//回调函数设为空
            return true;
        }
        else if (onCompleteCSharp != null)//调用CSharp端的回掉函数,具体过程和lua的一样.
        {
            onCompleteCSharp(asset);
            onCompleteCSharp = null;
            return true;
        }
        else//没有回调函数
        {
            return false;
        }
    }
}


//资源包信息类对象
class AssetBundleInfo
{
    public AssetBundle assetBundle;//资源包
    public int refCount;//引用数量

    //资源包信息对象的构造函数
    //参数是一个资源包
    public AssetBundleInfo(AssetBundle ab)
    {
        assetBundle = ab;
        refCount = 0;//刚开始的时候引用数量为0
    }

    //添加引用数量
    public void AddRef(int inc = 1)
    {
        refCount += inc;
    }

    //减少引用数量
    public void DecRef()
    {
        refCount -= 1;
        Debug.Assert(refCount >= 0, "refCount <= 0 is impossible!");
    }
}

//加载资源包的进度类
class LoadBundleProgress
{
    //依赖数组
    public string[] dependencies;

    //如果资源包还没有加载,那么这个progress就是-1; 大于0的话就是表示加载到第几个依赖包了.
    public int progress;    // 单个assetBundle加载的进度，-1：未加载bundle，>0：加载到第几个依赖包
    public bool isStarted;//是否开始加载资源包
    public string assetBundlePath;//资源包的路径

    //加载资源包的进程,刚开始创建的时候没有开始,所以isStarted初始化设置为false
    public LoadBundleProgress()
    {
        isStarted = false;
    }

    //重置
    //资源包路径,依赖的资源数组
    //初始化的时候进度为-1,表示未加载.
    //资源包路径设置成新的资源路径
    //如果资源路径不为空,则开始标记设置为真,否则设置为假
    public void Reset(string abPath, string[] dependencies)
    {
        this.dependencies = dependencies;
        progress = -1;
        assetBundlePath = abPath;
        isStarted = !string.IsNullOrEmpty(abPath);
    }

    //重置,将所有信息都清理掉.
    public void Reset()
    {
        Reset(null, null);
    }

    //应该是是否正在加载.
    //没有开始,或者说是开始了且已经加载完成了.都返回true
    public bool IsDone()
    {
        return (!isStarted || progress >= dependencies.Length);
    }

    //加载下一个引用
    //首先把资源包本省加载,然后再加载引用资源
    public string MoveNext()
    {
        progress++;//首先这个进度+1
        //加1后如果为0,说明刚开始是-1,说明是第一次加载,这里为什么要返回资源路径呢?
        if (progress == 0)
        {
            return assetBundlePath;
        }
        else
        {
            return dependencies[progress - 1];//第一个是资源包本身,所以这里要-1
        }
    }
}


//资源包加载器
class AssetBundleLoader
{
    //回调函数,应该是加载资源完成之后的回调函数
    //资源路径,资源包
    public delegate void Callback(string abPath, AssetBundle ab);

    Dictionary<string, List<Callback>> requests;//请求字典

    Dictionary<string, AssetBundleInfo> loaded;//资源包信息字典

    Dictionary<string, string[]> dependencies;//依赖字典

    Queue<string> loading;  // 需要依次加载的AssetBundle

    LoadBundleProgress progress;    // 当前正在加载的AssetBundle的进度

    AssetBundleManifest manifest;//manifest是资源包的依赖清单,记录了资源的信息比如依赖关系,自身完整性校验信息等等.
    string assetBundleRoot;//资源包的根路径
    string patchRoot;//补丁根路径
    HashSet<string> allBundles;//所有的包的哈希 列表.
#if DEBUG_RES_MANAGER//如果是调式的资源管理器
        int step = 1;//初始步骤为1
        
        //获取总共的引用数量
        //遍历资源包的信息资源,将每一个资源包信息中的引用数量字段加起来.
        public int GetTotalRef()
        {
            int ret = 0;
            foreach (KeyValuePair<string, AssetBundleInfo> entry in loaded)
            {
                ret += entry.Value.refCount;
            }
            return ret;
        }
#else
    //不是调试的资源管理器的话,初始化步骤为3,为什么要这么写还不知道.
    int step = 3;
#endif

    //资源包加载器的构造函数
    //资源包的根路径,补丁根路径,资源包信息文件路径
    public AssetBundleLoader(string assetBundleRoot, string patchRoot, string manifestPath)
    {

        //先把资源包根路径和补丁根路径存起来,然后初始化其他信息,比如请求列表初始化为空列表,资源包的信息字典设置为空字典,
        //需要依次加载的资源包列表设置为空
        //创建一个进度对象
        //依赖字典设置为空字典
        this.assetBundleRoot = assetBundleRoot;
        this.patchRoot = patchRoot;

        requests = new Dictionary<string, List<Callback>>();
        loaded = new Dictionary<string, AssetBundleInfo>();
        loading = new Queue<string>();
        progress = new LoadBundleProgress();
        dependencies = new Dictionary<string, string[]>();

        //然后利用资源包信息文件路径做初始化.调用初始化函数,将资源包信息文件路径作为参数传入
        Init(manifestPath);
    }

    public void Init(string manifestPath)
    {
        //首先必须要满足请求列表为空的,需要加载的资源包列表也是空的,进度对象没有开始搞事情.
        //否则的话就告知,异步加载还在跑
        Debug.Assert(requests.Count == 0 && loading.Count == 0 && progress.IsDone(), "Async requests still running");

        //先清理一下状态.
        loaded.Clear();
        dependencies.Clear();
        progress.Reset();

#if UNITY_EDITOR
        //如果是编辑器下,那么是编辑器中的模拟资源包处理,则这里什么都不做,直接返回.
        if (ResManager.SimulateAssetBundleInEditor)
        {
            return;
        }
#endif
        //真实的设备上,
        //如果资源信息对象不是空的,则卸载这个对象,并设置为空
        if (manifest != null)
        {
            Resources.UnloadAsset(manifest);
            manifest = null;
        }
        //从参数路径中加载资源信息,生成资源信息对象,如果没有加载成功,则报错.
        manifest = LoadManifest(manifestPath);
        if (manifest == null)
        {
            Debug.LogError("无法加载AssetBundle，请执行菜单项：Pet/打包Prefab");
        }

        //从这个资源信息对象中获取所有的资源包的哈希信息,存到allBundles这个哈希列表中.
        allBundles = new HashSet<string>(manifest.GetAllAssetBundles());
    }

    //根据资源包路径获取完整的路径
    //优先检查补丁路径下是否存在这个资源,如果存在则使用补丁中的内容;否则使用资源包根路径下的.
    private string GetFullPath(string abPath)
    {
        //合并补丁根路径和资源包路径
        string fullPath = Path.Combine(patchRoot, abPath);
        //如果不存在,则合并资源包根路径和资源包路径
        if (!File.Exists(fullPath))
            fullPath = Path.Combine(assetBundleRoot, abPath);
        return fullPath;
    }

    //根据资源包路径生成资源包信息对象
    private AssetBundleManifest LoadManifest(string abPath)
    {
        try
        {
            //从这个资源包路径中去加载资源包对象.
            AssetBundle manifestBundle = AssetBundle.LoadFromFile(GetFullPath(abPath));

            //然后从这个资源包中去得到资源包信息对象
            AssetBundleManifest manifest = manifestBundle.LoadAsset<AssetBundleManifest>("AssetBundleManifest");
            //然后把加载的资源包对象给卸载掉.
            manifestBundle.Unload(false);
            //最后返回这个资源包信息对象
            return manifest;
        }
        //如果发生异常则返回null.
        catch (Exception)
        {
            return null;
        }
    }

    //根据资源包的路径去加载资源包的信息
    private AssetBundleInfo LoadFromFile(string abPath)
    {
        AssetBundleInfo abInfo = null;
        if (!loaded.TryGetValue(abPath, out abInfo))//如果没有获取这个资源包路径的资源信息成功,那么就进入if里面去单独获取;否则给这个资源添加引用
                                                    //然后返回这个信息
        {
            //利用AssetBundle去加载资源包,然后根据这个资源包生成一个资源包信息对象,然后缓存到列表中.
            var ab = AssetBundle.LoadFromFile(GetFullPath(abPath));
            if (ab == null)
                return null;
            //Debug.Assert(ab != null, string.Format("{0} can't be loaded!", abPath));
            abInfo = new AssetBundleInfo(ab);
            loaded[abPath] = abInfo;
        }

        //有人来获取了一次,引用就加一
        abInfo.AddRef();
        return abInfo;
    }

    //是否存在这个资源包路径.
    public bool Exists(string abPath)
    {
        //就是看所有的资源包的哈希列表中是否包含这个资源包路径.这个哈希列表是所有的资源包路径的集合.存在则说明有;否则没有.
        return allBundles.Contains(abPath);
    }

    //根据资源包路径加载资源包
    public AssetBundle Load(string abPath)
    {
        //Debug.LogFormat("=== AB: Sync Load Start {0}", abPath);
        AssetBundleInfo abInfo = null;
        //首先必须要存在这个路径,不存在就返回null
        if (Exists(abPath))
        {
            //先加载资源的所有引用,如果有引用加载失败了,那么就卸载已经加载了的这些资源,然后返回null
            if (!LoadDependencies(abPath))
            {
                Unload(abPath);
                return null;
            }
            //到这里说明引用加载成功了,
            //接下来加载资源包本身.
            abInfo = LoadFromFile(abPath);
            //最后返回资源包信息中的资源包对象
            return abInfo.assetBundle;
        }
        return null;
    }


    //异步加载.
    //资源包路径
    //完成回调函数
    public IEnumerator LoadAsync(string abPath, Callback onComplete)
    {

        //只有当有资源包正在加载,则不需要启用协程来处理;否则需要启用协程来处理.
        //如果有工作做了,说明已经有这么一个协程在跑了,不需要重新再搞一个了.否则就需要再搞一个协程.
        bool needCoroutine = !HasJob();

        List<Callback> callbacks = null;
        //先从请求列表中获取这个资源包路径的回调函数.如果没有获取到,那么就创建一个缓存到requests列表中.
        if (!requests.TryGetValue(abPath, out callbacks))
        {
            //回调函数列表
            callbacks = new List<Callback>();
            requests[abPath] = callbacks;
            loading.Enqueue(abPath);//将这个资源包路径加到正在加载的列表尾部.
        }
        callbacks.Add(onComplete);//将这个回调函数加到回掉函数列表.


        if (needCoroutine)//如果需要开启携程进行异步处理.则开始异步加载.
        {
            //Debug.LogFormat("=== AB: ASync Load Start {0}", abPath);
            progress.Reset(abPath, GetDependencies(abPath));
            return DoJob();
        }
        else
        {
            return null;
        }
    }

    public IEnumerator DoJob()
    {
        int n = 0;
        string abPath = null;

        //用一个while循环来搞事情
        while (HasJob())
        {
            abPath = NextJob();
            if (!loaded.ContainsKey(abPath))
                ++n;
            //Debug.LogFormat("AB: Async Load Dependency: {0}", abPath);
            LoadFromFile(abPath);
            if (progress.IsDone())
                OnAssetBundleLoaded();
            if (n >= step)
                yield return null;
            n = 0;
        }
    }

    //是否有资源包正在加载
    public bool HasJob()
    {
        //正在加载的列表长度大于0,且进去对象没有完成,则说明有正在加载的.
        return loading.Count > 0 && !progress.IsDone();
    }

    private string NextJob()
    {
        while (loading.Count > 0)
        {
            //等到这个处理完,再返回下一个,继续处理.这个会不会有点浪费,卡在这里呢?
            while (!progress.IsDone())
                return progress.MoveNext();
        }
        Debug.Assert(false, "Make sure HasJob() before Call NextJob()");
        return null;
    }

    //获取资源包的所有依赖信息.
    private string[] GetDependencies(string abPath)
    {
        string[] d = null;
        if (!dependencies.TryGetValue(abPath, out d))
        {
            d = manifest.GetAllDependencies(abPath);
            dependencies[abPath] = d;
        }
        return d;
    }

    //资源包被加载完成的回调.
    private void OnAssetBundleLoaded()
    {
        string abPath = loading.Dequeue();//拿出一个路径

        //Debug.LogFormat("=== AB: ASync Load Done! {0}", abPath);
        var callbacks = requests[abPath];//得到这个路径的回调列表
        var ab = loaded[abPath];//拿出已经加载的资源包
        //回调每个回掉函数
        for (int i = 0; i < callbacks.Count; ++i)
        {
            callbacks[i](abPath, ab.assetBundle);
        }
        //然后从请求列表中移除
        requests.Remove(abPath);


        if (callbacks.Count > 1)
        {
            int inc = callbacks.Count - 1;
            //因为每个回调函数都会有一次引用,所以这里记录一下.
            //已经加载的资源包列表中去添加引用.资源包增加引用,这个资源依赖的资源也要增加引用.
            loaded[abPath].AddRef(inc);
            for (int i = 0; i < progress.dependencies.Length; ++i)
                loaded[progress.dependencies[i]].AddRef(inc);
        }

        if (loading.Count > 0)
        {
            string nextAb = loading.Peek();//返回列表开始的对象,但是不删除.
            //重置进度对象
            progress.Reset(nextAb, GetDependencies(nextAb));
            //Debug.LogFormat("===continue to Load AB {0}", nextAb);
        }
        else
            progress.Reset();
    }

    public AssetBundleManifest GetManifest()
    {
        return manifest;
    }

    //根据资源路径卸载某个资源包
    public void Unload(string abPath)
    {
        UnloadRaw(abPath);
        UnloadDependencies(abPath);
    }

    //raw:未加工的
    private void UnloadRaw(string abPath)
    {
        AssetBundleInfo abInfo = null;
        if (loaded.TryGetValue(abPath, out abInfo))
        {
            abInfo.DecRef();//引用-1
            if (abInfo.refCount <= 0)
            {
                abInfo.assetBundle.Unload(true);//真正的卸载
                loaded.Remove(abPath);//移除路径
                //Debug.LogFormat("Assetbundle Unloaded! {0}", abPath);
            }
        }
    }

    //移除引用
    private void UnloadDependencies(string abPath)
    {
        var dependencies = GetDependencies(abPath);//获取引用
        for (int i = 0; i < dependencies.Length; ++i)//移除每一个引用
            UnloadRaw(dependencies[i]);
    }

    //加载引用,其实这里只是看能否成功加载引用,如果可以就返回true;否则返回false;
    private bool LoadDependencies(string abPath)
    {
        String[] dependencies = GetDependencies(abPath);
        for (int i = 0; i < dependencies.Length; ++i)
        {
            var depend = dependencies[i];
            //Debug.LogFormat("AB: Load Dependency: {0}", depend);
            var dependInfo = LoadFromFile(depend);//加载引用
            if (dependInfo == null)
            {
                Debug.LogErrorFormat("Load Dependency error: {0} while Loading assetbundle {1}", depend, abPath);
                return false;
            }
        }
        return true;
    }
}



//资源管理器.
public class ResManager : MonoBehaviour
{
#if UNITY_EDITOR
    // Flag to indicate if we want to simulate assetBundles in Editor without building them actually.
    //表明是否我们需要在编辑器里模拟资源包的标记.实际中不需要使用
    static int m_SimulateAssetBundleInEditor = -1;
    const string kSimulateAssetBundles = "SimulateAssetBundles";//模拟资源包

    //黑名单,说明不要lua来引用这个接口
    [XLua.BlackList]

    //在编辑器中模拟资源包特性.
    //获取
    //设置
    public static bool SimulateAssetBundleInEditor
    {
        get
        {
            //获取
            if (m_SimulateAssetBundleInEditor == -1)
                m_SimulateAssetBundleInEditor = EditorPrefs.GetBool(kSimulateAssetBundles, true) ? 1 : 0;

            return m_SimulateAssetBundleInEditor != 0;
        }
        set
        {
            //有新值,则标记为1,否则标记为0.并且把新值设置到EditorPrefs中.
            int newValue = value ? 1 : 0;
            if (newValue != m_SimulateAssetBundleInEditor)
            {
                m_SimulateAssetBundleInEditor = newValue;
                EditorPrefs.SetBool(kSimulateAssetBundles, value);
            }
        }
    }
    //创建一个编辑器加载资源请求列表
    private List<LoadAssetRequest> editorLoadAssetRequests;

#endif
    const string abExt = ".assetbundle";//资源包的扩展名

    static string assetBundleRoot;//资源包根路径
    public static ResManager instance;//资源管理器实例

    AssetBundleLoader abLoader;//资源包加载器(资源管理器有一个成员:资源包加载器)
    Dictionary<string, List<LoadAssetRequest>> asyncRequests;//异步请求列表.
    int loadingCount;//正在加载的数量

    Dictionary<UnityEngine.Object, int> refCount;//引用数量,每个对象以及对应的引用数量关系.
    //资源对象对应的资源路径关系列表,已经加载的
    Dictionary<UnityEngine.Object, string> loadedAsset;             // asset -> assetBundlePath
    //游戏对象对应的资源/预制体关系列表
    Dictionary<GameObject, UnityEngine.Object> loadedGameObject;    // GameObject -> asset(Prefab)

    //资源管理器激活
    private void Awake()
    {
        instance = this;//实例为自己.单例模式
        //Applicaiton.dataPath在安卓上的路径是/data/app/package name-1/base.apk
        //ios是:/var/containers/Bundle/Application/app sandbox/xxx.app/Data 

        if (Application.platform == RuntimePlatform.Android)//如果是安卓平台,则资源包根路径是Application.dataPath + "!assets"合并上"res"
            assetBundleRoot = Path.Combine(Application.dataPath + "!assets", "res");
        else
            //streamingAssetsPath在android是:jar:file:///data/app/package name-1/base.apk!/assets
            //ios:/var/containers/Bundle/Application/app sandbox/test.app/Data/Raw
            assetBundleRoot = Path.Combine(Application.streamingAssetsPath, "res");
        //补丁路径
        //Application.persistentDataPath:
        //安卓:/storage/emulated/0/Android/data/package name/files
        //ios:/var/mobile/Containers/Data/Application/app sandbox/Documents
        string patchRoot = Path.Combine(Application.persistentDataPath, "res");

        //创建一个资源包加载器,参数是资源包根路径和补丁根路径, 以及manifest的相对路径这里是res
        abLoader = new AssetBundleLoader(assetBundleRoot, patchRoot, "res");

        //创建异步加载请求列表.刚开始是空的.
        asyncRequests = new Dictionary<string, List<LoadAssetRequest>>();
        //创建已经加载的资源字典,刚开始是空的.
        loadedAsset = new Dictionary<UnityEngine.Object, string>();
        //已经加载的游戏对象字典.刚开始是空的.
        loadedGameObject = new Dictionary<GameObject, UnityEngine.Object>();
        //创建对象以及其引用数量的关系列表.刚开始是空的.
        refCount = new Dictionary<UnityEngine.Object, int>();

        //切换场景的时候这个对象是不允许被销毁.
        DontDestroyOnLoad(this);
#if UNITY_EDITOR
        //如果是编辑器还要创建一个编辑器加载资源请求列表,刚开始是空的.
        editorLoadAssetRequests = new List<LoadAssetRequest>();
#endif
    }


    //重置资源管理器
    //卸载所有的资源包
    //重新初始化资源加载器
    public void Reset()
    {
        UnloadAll();
        abLoader.Init("res");
    }

    //获取资源包的路径.
    //传进来的只是路径,没有扩展名
    public string GetAssetBundlePath(string assetPath)
    {
        string abPath = assetPath + abExt;
        if (abLoader.Exists(abPath))//如果真的存在这个路径就返回.
        {
            return abPath;
        }
        else//否则就去掉最后一个反斜杠后面的内容,然后再加上".assetbundle"扩展名作为路径,饭后返回.这里具体为什么这么做还不知道.
        {
            var splitIndex = abPath.LastIndexOf('/');
            string abPath2 = abPath.Substring(0, splitIndex) + ".assetbundle";
            return abPath2;
        }
    }


    //归一化路径,或者教格式化路径.
    //路径，如果是unity编辑器的话，就是： "Assets/res/" + assetPath;
    //否则就是：return ("assets/res/" + assetPath).ToLower();
    private string NormalizePath(string assetPath)
    {
#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            return "Assets/res/" + assetPath;
        }       
#endif
        return ("assets/res/" + assetPath).ToLower();
    }

    //加载资源，传入prefabPath, type（type可以不传。）
    public UnityEngine.Object LoadAsset(string prefabPath, Type type = null)
    {
        //转换路径
        prefabPath = NormalizePath(prefabPath);
#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            if (type == null)
            {
                type = typeof(UnityEngine.Object);//如果没有指定类型，其实就是任意类型，Object是其他类型的基类，所以什么类型都行。
            }
            //加载prefabPath路径下的资源对象。调用资源数据库的指定路径加载资源接口去加载对象.
            var obj = AssetDatabase.LoadAssetAtPath(prefabPath, type);
            if (obj == null)
            {
                Debug.LogErrorFormat("the asset {0} not exist in the editor!", prefabPath);
            }
            return obj;
        }
#endif
        //不是unity编辑器的话，就要先获取AssetBundle的路径了。
        string abPath = GetAssetBundlePath(prefabPath);
        //然后调用assetbundle 的加载器去加载指定路径的对象。
        var ab = abLoader.Load(abPath);//这个最后是调用AssetBundle.LoadFromFile接口去加载的.

        //加载了对象，还要加载资源？
        UnityEngine.Object asset = null;
        if (type == null)
            //AssetBundle的LoadAsset接口
            asset = ab.LoadAsset(prefabPath);//没有类型的
        else
            asset = ab.LoadAsset(prefabPath, type);//有类型的

        if (asset == null)
        {
            abLoader.Unload(abPath);//加载失败了,要卸载掉.
            Debug.LogErrorFormat("Asset: Async Load Error, assetPath = {0}, abPath = {1}, type = {2}", prefabPath, abPath, type);
        }
        else
        {
            loadedAsset[asset] = abPath;
            int n = 0;
            refCount.TryGetValue(asset, out n);
            ++n;//引用+1
            refCount[asset] = n;
            //Debug.LogFormat("Asset: Sync Load Done, asset={0}", asset.ToString());
        }
        return asset;
    }

    //根据预制体的路径 去加载对象，就是加载显示的对象。
    public GameObject LoadGameObject(string prefabPath)
    {
        //路径对应的对象载入内存
        GameObject prefab = LoadAsset(prefabPath) as GameObject;
        //对象的clone，实例化。
        GameObject go = Instantiate(prefab);
        loadedGameObject[go] = prefab;//记录这个对象是哪个prefab实例化来的。
        return go;
    }

    //异步加载
    //预制体路径和回调函数
    public LoadAssetRequest LoadAssetAsync(string prefabPath, LuaFunction onComplete)
    {
        return _LoadAssetAsync(prefabPath, null, onComplete, null);
    }

    //指定了类型的异步加载
    public LoadAssetRequest LoadAssetAsync(string prefabPath, Type type, LuaFunction onComplete)
    {
        return _LoadAssetAsync(prefabPath, type, onComplete, null);
    }

    //回调函数是C#的异步加载
    public LoadAssetRequest LoadAssetAsync(string prefabPath, Type type, UnityEngine.Events.UnityAction<UnityEngine.Object> onCompleteCSharp)
    {
        return _LoadAssetAsync(prefabPath, type, null, onCompleteCSharp);
    }

#if UNITY_EDITOR
    //编辑器模拟的异步加载
    private void OnEditorSimulateAsyncLoad()
    {
        //遍历编辑器加载请求列表
        //使用资源数据库的LoadAssetAtPath接口去依次加载每一个资源.
        //如果请求类型是GameObject的话,那么就用加载出来的资源实例化一个GameObject对象,
        //然后调用回调函数,通过接口SafeCallback,参数是req和asset
        //遍历完成后清理这个编辑器加载请求列表.
        foreach (LoadAssetRequest req in editorLoadAssetRequests)
        {
            var asset = AssetDatabase.LoadAssetAtPath(req.assetPath, req.type);
            if (req.type == typeof(GameObject))
            {
                asset = GameObject.Instantiate(asset);
            }
            SafeCallback(req, asset);
        }
        editorLoadAssetRequests.Clear();
    }
#endif

    //真实的异步加载函数
    private LoadAssetRequest _LoadAssetAsync(string assetPath, Type type, LuaFunction onComplete, UnityEngine.Events.UnityAction<UnityEngine.Object> onCompleteCSharp)
    {
        //
        assetPath = NormalizePath(assetPath);
        LoadAssetRequest req;

        if (type == null)
        {
            type = typeof(UnityEngine.Object);
        }

        if (onComplete != null)
        {
            req = new LoadAssetRequest(assetPath, type, onComplete);
        }
        else
        {
            req = new LoadAssetRequest(assetPath, type, onCompleteCSharp);
        }

#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            editorLoadAssetRequests.Add(req);
            Invoke("OnEditorSimulateAsyncLoad", 1.0f / 1000f);  // 此处异步返回，保持与正常游戏模式同样的行为
            return req;
        }
#endif
        string abPath = GetAssetBundlePath(assetPath);

        // 添加请求
        List<LoadAssetRequest> requests = null;
        if (!asyncRequests.TryGetValue(abPath, out requests))
        {
            requests = new List<LoadAssetRequest>();
            asyncRequests[abPath] = requests;
        }

        requests.Add(req);

        //创建一个异步加载.
        loadingCount += 1;
        IEnumerator co = abLoader.LoadAsync(abPath, OnAssetBundleLoaded);
        if (co != null)
            StartCoroutine(co);
        return req;
    }

    //卸载
    public void UnloadAsset(UnityEngine.Object asset)
    {
        //Debug.Log("start unload asset" + asset.name);
        string abPath = null;
        if (asset == null || !loadedAsset.TryGetValue(asset, out abPath))
            return;

        abLoader.Unload(abPath);
        int n = --refCount[asset];
        if (n <= 0)
        {
            loadedAsset.Remove(asset);
            refCount.Remove(asset);
            //string assetName = asset.name;
            //Debug.LogFormat("Removed Entry!!! AssetUnloaded!" + assetName);
        }
        else
        {
            //Debug.LogFormat("asset ref --count = {0}", n);
        }
    }

    public void UnloadGameObject(GameObject go)
    {
        UnityEngine.Object prefab = null;
        if (go == null || !loadedGameObject.TryGetValue(go, out prefab))
            return;
        UnloadAsset(prefab);
        if (!refCount.ContainsKey(prefab))
            loadedGameObject.Remove(go);
    }

    //加载场景包
    // raw interfaces
    public string LoadSceneBundle(string assetPath)
    {
        assetPath = NormalizePath(assetPath);
#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            if (assetPath.EndsWith("navmesh.unity"))
            {
                EditorApplication.LoadLevelAdditiveInPlayMode(assetPath);
                return null;
            }

            return Path.GetFileNameWithoutExtension(assetPath);

        }
#endif

        string abPath = GetAssetBundlePath(assetPath);
        AssetBundle ab = abLoader.Load(abPath);
        return ab.GetAllScenePaths()[0];
    }

    public void UnloadSceneBundle(string assetPath)
    {
        assetPath = NormalizePath(assetPath);
        string abPath = GetAssetBundlePath(assetPath);
        abLoader.Unload(abPath);
    }

    //加载完成调用函数
    public void OnAssetBundleLoaded(string abPath, AssetBundle ab)
    {
        List<LoadAssetRequest> requests = null;
        if (asyncRequests.TryGetValue(abPath, out requests))
        {
            StartCoroutine(OnLoadAsset(abPath, ab, requests.ToArray()));
            asyncRequests.Remove(abPath);
        }
    }

    IEnumerator OnLoadAsset(string abPath, AssetBundle ab, LoadAssetRequest[] requests)
    {
        AssetBundleRequest unityRequest;
        for (int i = 0; i < requests.Length; ++i)
        {
            if (requests[i].type == null)
                unityRequest = ab.LoadAssetAsync(requests[i].assetPath);
            else
                unityRequest = ab.LoadAssetAsync(requests[i].assetPath, requests[i].type);
            yield return unityRequest;
            loadingCount -= 1;

            var asset = unityRequest.asset;
            UnityEngine.Object ret = asset;
            if (asset != null)
            {
                loadedAsset[asset] = abPath;

                // Add Ref
                int n = 0;
                refCount.TryGetValue(asset, out n);
                ++n;
                refCount[asset] = n;

                if (requests[i].type == typeof(GameObject))
                {
                    ret = Instantiate(asset as GameObject);
                    loadedGameObject[ret as GameObject] = asset;
                }
            }
            else
            {
                Debug.LogError("[OnLoadAsset] Load Asset Failed: assetBundle = " + abPath + "\n\tasset = " + requests[i].assetPath);
                abLoader.Unload(abPath);
            }

            SafeCallback(requests[i], ret);
        }
    }

    private void SafeCallback(LoadAssetRequest request, UnityEngine.Object ret)
    {
        if (!request.Callback(ret))
        {
            if (request.type == typeof(GameObject))
                UnloadGameObject(ret as GameObject);
            else
                UnloadAsset(ret);
        }
    }

    public void UnloadAll()
    {
        UnityEngine.Object[] assets = new UnityEngine.Object[loadedAsset.Count];
        loadedAsset.Keys.CopyTo(assets, 0);
        for (int i = 0; i < assets.Length; ++i)
        {
            int n = refCount[assets[i]];
            //Debug.LogFormat("remove {0} {1} times", assets[i].name, n);
            for (int j = 0; j < n; ++j)
            {
                abLoader.Unload(loadedAsset[assets[i]]);
            }
        }
        loadedGameObject.Clear();
        loadedAsset.Clear();
        refCount.Clear();
        // asyncRequests保留
    }

    public void Report()
    {
        if (loadedAsset.Count <= 0)
        {
            Debug.Log("[ASSET] no asset has been loaded, possibly using simulate mode?");
            return;
        }
        Debug.LogFormat("[ASSET] {0} loaded asset(s):", loadedAsset.Count);
        foreach (KeyValuePair<UnityEngine.Object, string> entry in loadedAsset)
        {
            var asset = entry.Key;
            var assetBundlePath = entry.Value;
            Debug.LogFormat("  [ASSET] <b>{0}</b>\n  <i>{1}</i>", asset.name, assetBundlePath);
        }
    }

    #region Debug
#if DEBUG_RES_MANAGER

        private void OnGUI()
        {
            if (GUI.Button(new Rect(0, 0, 100, 100), "同步加载(随机)"))
            {
                LoadAsset(GetRandomPrefabPath());
            }
            if (GUI.Button(new Rect(0, 100, 100, 100), "同步加载(全部)"))
            {
                foreach (string prefabPath in GetAllPrefabPath())
                {
                    LoadAsset(prefabPath);
                }
            }
            if (GUI.Button(new Rect(0, 200, 100, 100), "异步加载(随机)"))
            {
                LoadAssetAsync(GetRandomPrefabPath(), null);
            }
            if (GUI.Button(new Rect(0, 300, 100, 100), "异步加载(全部)"))
            {
                foreach(string prefabPath in GetAllPrefabPath())
                {
                    LoadAssetAsync(prefabPath, null);
                }
            }
            GUI.TextArea(new Rect(120, 400, 150, 20), "Loading: " + loadingCount.ToString());
            GUI.TextArea(new Rect(120, 420, 150, 20), "Loaded: " + loadedAsset.Count);
            int totalRefCount = 0;
            foreach (KeyValuePair<UnityEngine.Object, int> entry in refCount)
                totalRefCount += entry.Value;
            GUI.TextArea(new Rect(120, 440, 150, 20), "TotRef: " + totalRefCount);
            GUI.TextArea(new Rect(120, 460, 150, 20), "BundleRef: " + abLoader.GetTotalRef());
            if (GUI.Button(new Rect(270, 400, 150, 100), "卸载全部"))
            {
                UnloadAll();
            }
            if (GUI.Button(new Rect(0, 400, 100, 100), "随机混合加载"))
            {
                foreach (string prefabPath in GetAllPrefabPath())
                {
                    if (UnityEngine.Random.Range(0, 1) == 0)
                    {
                        LoadAssetAsync(prefabPath, null);
                    } else
                    {
                        LoadAsset(prefabPath);
                    }
                }
            }

        if (GUI.Button(new Rect(0, 500, 100, 100), "随机操作"))
        {
            if (randomTestCo != null)
            {
                StopCoroutine(randomTestCo);
            }
            randomTestCo = StartCoroutine(RandomTest());
        }

        if (GUI.Button(new Rect(0, 700, 100, 100), "停止随机操作"))
        {
            if (randomTestCo != null)
            {
                StopCoroutine(randomTestCo);
            }
        }

        //if (GUI.Button(new Rect(0, 600, 100, 100), "一个资源连续加载N次"))
        //{
        //    string _p = GetRandomPrefabPath();
        //    for (int i = 0; i < 200; ++ i)
        //    {
        //        LoadAssetAsync(_p, null);
        //    }
        //}
        if (GUI.Button(new Rect(0, 600, 100, 100), "随机卸载"))
            {
                UnityEngine.Object[] assets = new UnityEngine.Object[loadedAsset.Count];
                if (assets.Length > 0)
                {
                    loadedAsset.Keys.CopyTo(assets, 0);
                    int idx = UnityEngine.Random.Range(0, assets.Length);
                    Debug.LogWarningFormat("trying to Unload {0}", loadedAsset[assets[idx]]);
                    UnloadAsset(assets[idx]);
                }
            }
        }

    private Coroutine randomTestCo;
    IEnumerator RandomTest()
    {
        string[] allAssetPath = GetAllPrefabPath();
        while (true)
        {
            int op = UnityEngine.Random.Range(0, 10);
            if (op < 5)
            {
                LoadAsset(allAssetPath[UnityEngine.Random.Range(0, allAssetPath.Length)]);
            }
            else if (op < 8)
            {
                LoadAssetAsync(allAssetPath[UnityEngine.Random.Range(0, allAssetPath.Length)], null);
            }
            else
            {
                UnityEngine.Object[] assets = new UnityEngine.Object[loadedAsset.Count];
                if (assets.Length > 0)
                {
                    loadedAsset.Keys.CopyTo(assets, 0);
                    UnloadAsset(assets[UnityEngine.Random.Range(0, assets.Length)]);
                }
            }
            yield return null;
        }
    }
        private string GetRandomPrefabPath()
        {
            string[] allPrefabs = GetAllPrefabPath();
            int i = UnityEngine.Random.Range(0, allPrefabs.Length);
            return allPrefabs[i];
        }

        private string[] GetAllPrefabPath()
        {
            AssetBundleManifest manifest = abLoader.GetManifest();
            string[] allBundles = manifest.GetAllAssetBundles();
            List<string> allPrefabPath = new List<string>();
            foreach (string abPath in allBundles)
            {
                if (abPath.EndsWith(".prefab" + abExt))
                {
                    allPrefabPath.Add(abPath.Substring(0, abPath.Length - abExt.Length));
                }
            }
            return allPrefabPath.ToArray();
        }
#endif
    #endregion
}