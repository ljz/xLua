Shader "Unlit/NewUnlitShader"
{
	Properties
	{
		_MainTex ("Texture", 2D) = "white" {}
		_TintColor("Color", Color) = (1,1,1,1)
	}
	SubShader
	{
		//形式：Tags { "TagName1" = "Value1" "TagName2" = "Value2" }
		//控制渲染引擎“何时”、“如何”将子Shader内容进行呈现。表示的是“渲染输出的是非透明物体”。
		//Tags { “Queue” = “Opaque” } 表示的是“指定在渲染非透明物体的顺序队列”
		//这两者最主要的区别在于“RenderType”表示的是渲染什么样的物体，而“Queue”表示的是在什么样的实ti渲染物体。
		Tags { "RenderType"="Opaque" }
		LOD 100

		Pass
		{
			CGPROGRAM
			//#pragma的作用是指示编译对应的着色器函数。
			#pragma vertex vert//声明一个名为vert的顶点函数（Vertex Function）
			#pragma fragment frag//声明一个名为frag的片元函数（Fragment Function）
			// make fog work
			#pragma multi_compile_fog
			
			//导入Unity通用CG预定义文件，后面的UnityObjectToClipPos函数就是在该文件里定义好的。
			#include "UnityCG.cginc"

			//mesh data
			//输入顶点函数的结构体：
			struct appdata
			{
				//float4 4 wei, 4 float data:x,y,z,w  r,g,b,a
				//float 32bits
				//half 16bits
				//fixed 11bits
				float4 vertex : POSITION;//声明了一个名为vertex的四维浮点数，语义为网格的顶点坐标数据。
				//用于记录引入纹理的UV坐标。
				float2 uv : TEXCOORD0;//ding dian zhi jian de bian shu ju....
			};

			//vertext to fragment struct
			//顶点输出到片元输入的结构体：
			struct v2f
			{
				//这里使用 2D 纹理，因此是二维的,用于记录引入纹理的UV坐标。
				float2 uv : TEXCOORD0;//UV(W) 是纹理空间中的多维坐标系，值域 0 到 1。
				UNITY_FOG_COORDS(1)
				//而语义上的SV所代表的是System Value（系统值），SV_POSITION对应就是屏幕上的像素位置。
				float4 vertex : SV_POSITION;//网格顶点对应到屏幕上的坐标
			};

			float4 _TintColor;//实例化声明,这个float4的变量与属性里的名字是一致。
			//这个float4变量是将属性（Properties）里的变量在Unity着色器内部进行数据绑定用的，
			//为了CG 程序正常访问属性（Properties）的变量，CG程序中的变量必须使用和之前变量相同的名字进行声明
			sampler2D _MainTex;
			float4 _MainTex_ST;
			
			//Vertex function
			//input mesh data
			//return vertext to fragment struct
			//顶点函数实现：
			v2f vert (appdata v)
			{
				v2f o;
				//vertext data transform from object space to screen space.
				//使用UnityObjectToClipPos方法，将输入网格顶点对象空间转换到屏幕剪裁平面。
				//
				o.vertex = UnityObjectToClipPos(v.vertex);

				//texture process
				//获取到的网格数据上的UV信息（网格平铺成二维与纹理的一一对应），赋值给v2f结构体中。
				o.uv = TRANSFORM_TEX(v.uv, _MainTex);


				//fog
				UNITY_TRANSFER_FOG(o,o.vertex);
				return o;
			}
			
			//片元函数:渲染纹理
			//fragment function
			//input v2f struct
			//return data to screen,such as pixel points.
			//实际就是屏幕的像素，最后frag函数return的就是像素，
			//即RGBA颜色，因此frag返回的类型就是fixed4类型。
			fixed4 frag (v2f i) : SV_Target//SV_Target就是System Value Target
			{
				// sample the texture
				//tex2D:
				//input texture and uv, return uv texture color.
				//then we can see the colorful image.
				fixed4 col = tex2D(_MainTex, i.uv);
				// apply fog
				UNITY_APPLY_FOG(i.fogCoord, col);
				return col;
			}
			ENDCG
		}
	}
}

//其他有特色的顶点/片元着色器效果介绍：

//1.双纹理混合（Texture Blending - Lerp!）
//做双纹理混合肯定需要引入两张不同的纹理，
//分别声明了Main Texture和Second Texture，
//然后可以通过一个_Blend_Amount参数来调节两个纹理的混合比例。
//接着，在片元函数部分，分别获取两个纹理对应UV的像素颜色，通过Lerp函数进行混合。

//Lerp的功能是基于权重返回两个标量或向量的线性差值。

//调节_Blend_Amount，就可以获得双纹理混合的显示效果




//2.颜色渐变（Color Ramp - Texture Sample）
//[Header(Color Ramp Sample)]它用于在检视窗口中添加一个标签文本。
//[NoScaleOffset]一般引入一个2D的纹理，都会有Tiling和Offset显示，即可以调节纹理的缩放与平移，
//使用了[NoScaleOffset]就会将这两个参数设置禁用，仅仅获得纹理的原始比例与平移。
//_ColorRamp_Evaluation其实是获取这张渐变图的水平中心像素点的位置，从而得到该位置的像素。
//将主纹理的颜色与渐变纹理的颜色相乘，即可获得混合后的颜色

//颜色与颜色之间可以进行加减乘除进行混合运算。
//加法可以起到颜色叠加的效果，但是由于颜色值的值域为0~1，
//相加很容易达到1，就会颜色会愈发明亮，因此叠加建议使用乘法；
//减法可以进行反色处理，但是同样是值域的原因，数值达到0，颜色就很暗淡，因此要做反色建议使用除法。


//3.纹理剔除（Texture Cutout）
//在片元函数中使用了Clip方法，该方法的功能是当输入的参数小于等于0时，就会删除对应位置的像素。
//这里使用了一个有不同颜色分布的纹理作为剔除纹理，使用CutOut Value作为剔除参考值，
//当该纹理像素某位置上RGB分别减去这个CutOut Value有小于等于0的时候，
//这个蜘蛛机器人就会有镂空的效果


//4.世界坐标-梯度（World Space - Gradient）
//在v2f结构体中，添加了世界坐标
//而这个坐标是通过unity_ObjectToWorld，来获取每个顶点在世界空间中的坐标。
//在输入的部分加入了两个表示高低不同位置的颜色，
//片元函数中对两者根据踢动值进行了线性插值处理。
//最后以相乘的方式进行了像素混合，呈现出图中的画面效果。



//5.法线挤压（Normal Extrusion）
//在appdata结构体中引入了法线（Normal）。
//在顶点函数计算的时候，将发现xyz值乘以挤压值（Extrusion Amount），
//而后叠加到顶点的xyz上，这样就可以根据这个挤压值对于顶点的对象空间位置进行法线相关的偏移处理，
//最后可以得到“膨胀变胖”、“挤压变瘦”的有趣效果。


//6.时间相关（Time）
//_Time为Unity着色器默认载入UnityShaderVariables.cginc的变量，_Time.y表示游戏自启动开始后的时间。
//通过波速、波距和波频三个参数是将机器人进行波形化处理。

//7.漫反射光照（Diffuse Lighting）
//首先就需要添加Tags的LightMode为ForwardBase，基于前向的光照模式，
//还引入了UnityLightingCommon.cginc预定义文件来辅助光照的运算。
//appdata结构体中不会缺少网格的法线数据。
//在顶点函数中，通过UnityObjectToWorldNormal获取网格对象在世界坐标中的法线值。
//通过dot方法得到法线值与世界空间光照坐标的点积值，作为漫反射参考值。
//然后，将这个漫反射参考值与光照颜色相乘获得光照的漫反射颜色。
//最后在片元函数中将主纹理颜色与之相乘混合，得到最终的像素颜色。



//其他控制Unity着色器的方法

//8.通过动画控制（Animation Clip）
//没有使用额外的代码，仅仅依靠动画片段（Animation Clip）来控制，着色器的参数值。
//Unity着色器依附于材质（Material），而材质需要渲染器（Renderer）使之生效。
//因此通过动画控制Unity着色器只要找到对应的参数，
//就可以像制作其他角色动画一样，让策划或美术调整着色器显示的效果。

//9.脚本控制（Scripting）
//只要通过Renderer → Material → Shader就可以访问到需要的参数。
//因此，可以通过Get和Set的方式去获取对应参数的值，或者去修改对应参数的值。







































