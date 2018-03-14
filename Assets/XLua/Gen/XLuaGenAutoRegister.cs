#if USE_UNI_LUA
using LuaAPI = UniLua.Lua;
using RealStatePtr = UniLua.ILuaState;
using LuaCSFunction = UniLua.CSharpFunctionDelegate;
#else
using LuaAPI = XLua.LuaDLL.Lua;
using RealStatePtr = System.IntPtr;
using LuaCSFunction = XLua.LuaDLL.lua_CSFunction;
#endif

using System;
using System.Collections.Generic;
using System.Reflection;


namespace XLua.CSObjectWrap
{
    public class XLua_Gen_Initer_Register__
	{
	    static XLua_Gen_Initer_Register__()
        {
		    XLua.LuaEnv.AddIniter((luaenv, translator) => {
			    
				translator.DelayWrapLoader(typeof(LuaBehaviour), LuaBehaviourWrap.__Register);
				
				translator.DelayWrapLoader(typeof(XLuaTest.Pedding), XLuaTestPeddingWrap.__Register);
				
				translator.DelayWrapLoader(typeof(XLuaTest.MyStruct), XLuaTestMyStructWrap.__Register);
				
				translator.DelayWrapLoader(typeof(XLuaTest.MyEnum), XLuaTestMyEnumWrap.__Register);
				
				translator.DelayWrapLoader(typeof(XLuaTest.NoGc), XLuaTestNoGcWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Coroutine_Runner), Coroutine_RunnerWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.WaitForSeconds), UnityEngineWaitForSecondsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.WWW), UnityEngineWWWWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Foo1Parent), Foo1ParentWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Foo2Parent), Foo2ParentWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Foo1Child), Foo1ChildWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Foo2Child), Foo2ChildWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Foo), FooWrap.__Register);
				
				translator.DelayWrapLoader(typeof(FooExtension), FooExtensionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(object), SystemObjectWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Object), UnityEngineObjectWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Vector2), UnityEngineVector2Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Vector3), UnityEngineVector3Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Vector4), UnityEngineVector4Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Quaternion), UnityEngineQuaternionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Color), UnityEngineColorWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Ray), UnityEngineRayWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Bounds), UnityEngineBoundsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Ray2D), UnityEngineRay2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Time), UnityEngineTimeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.GameObject), UnityEngineGameObjectWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Component), UnityEngineComponentWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Behaviour), UnityEngineBehaviourWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Transform), UnityEngineTransformWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Resources), UnityEngineResourcesWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.TextAsset), UnityEngineTextAssetWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Keyframe), UnityEngineKeyframeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AnimationCurve), UnityEngineAnimationCurveWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AnimationClip), UnityEngineAnimationClipWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.MonoBehaviour), UnityEngineMonoBehaviourWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem), UnityEngineParticleSystemWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.SkinnedMeshRenderer), UnityEngineSkinnedMeshRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Renderer), UnityEngineRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(System.Collections.Generic.List<int>), SystemCollectionsGenericList_1_SystemInt32_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Debug), UnityEngineDebugWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.BaseClass), TutorialBaseClassWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.TestEnum), TutorialTestEnumWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.DrivenClass), TutorialDrivenClassWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.DrivenClass.TestEnumInner), TutorialDrivenClassTestEnumInnerWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.Calc), TutorialCalcWrap.__Register);
				
				translator.DelayWrapLoader(typeof(Tutorial.DrivenClassExtensions), TutorialDrivenClassExtensionsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Font), UnityEngineFontWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.SceneManagement.SceneManager), UnityEngineSceneManagementSceneManagerWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.TextAnchor), UnityEngineTextAnchorWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.PlayerPrefs), UnityEnginePlayerPrefsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.WrapMode), UnityEngineWrapModeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RigidbodyType2D), UnityEngineRigidbodyType2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Camera), UnityEngineCameraWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Random), UnityEngineRandomWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Mathf), UnityEngineMathfWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Animator), UnityEngineAnimatorWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AnimatorStateInfo), UnityEngineAnimatorStateInfoWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.SpriteRenderer), UnityEngineSpriteRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.QualitySettings), UnityEngineQualitySettingsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshAgent), UnityEngineAINavMeshAgentWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshObstacle), UnityEngineAINavMeshObstacleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshPath), UnityEngineAINavMeshPathWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMesh), UnityEngineAINavMeshWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshPathStatus), UnityEngineAINavMeshPathStatusWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshHit), UnityEngineAINavMeshHitWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AI.NavMeshTriangulation), UnityEngineAINavMeshTriangulationWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.EventTrigger), UnityEngineEventSystemsEventTriggerWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.BaseRaycaster), UnityEngineEventSystemsBaseRaycasterWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.UIBehaviour), UnityEngineEventSystemsUIBehaviourWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.EventTrigger.TriggerEvent), UnityEngineEventSystemsEventTriggerTriggerEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.EventSystem), UnityEngineEventSystemsEventSystemWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.EventTriggerType), UnityEngineEventSystemsEventTriggerTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.PointerEventData), UnityEngineEventSystemsPointerEventDataWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.EventTrigger.Entry), UnityEngineEventSystemsEventTriggerEntryWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Rendering.RenderQueue), UnityEngineRenderingRenderQueueWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Material), UnityEngineMaterialWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.CanvasGroup), UnityEngineCanvasGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RigidbodyConstraints2D), UnityEngineRigidbodyConstraints2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.MainModule), UnityEngineParticleSystemMainModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.MinMaxGradient), UnityEngineParticleSystemMinMaxGradientWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RectTransform), UnityEngineRectTransformWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.GraphicRaycaster), UnityEngineUIGraphicRaycasterWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.GraphicRaycaster.BlockingObjects), UnityEngineUIGraphicRaycasterBlockingObjectsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.VerticalLayoutGroup), UnityEngineUIVerticalLayoutGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RectOffset), UnityEngineRectOffsetWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.HorizontalOrVerticalLayoutGroup), UnityEngineUIHorizontalOrVerticalLayoutGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.LayoutGroup), UnityEngineUILayoutGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEvent<bool>), UnityEngineEventsUnityEvent_1_SystemBoolean_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEvent<string>), UnityEngineEventsUnityEvent_1_SystemString_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEvent<float>), UnityEngineEventsUnityEvent_1_SystemSingle_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.LayerMask), UnityEngineLayerMaskWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.CameraClearFlags), UnityEngineCameraClearFlagsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RenderTexture), UnityEngineRenderTextureWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.RawImage), UnityEngineUIRawImageWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystemRenderer), UnityEngineParticleSystemRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.FontStyle), UnityEngineFontStyleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Canvas), UnityEngineCanvasWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEvent), UnityEngineEventsUnityEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEventBase), UnityEngineEventsUnityEventBaseWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Button), UnityEngineUIButtonWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Button.ButtonClickedEvent), UnityEngineUIButtonButtonClickedEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image), UnityEngineUIImageWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Text), UnityEngineUITextWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField), UnityEngineUIInputFieldWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.OnChangeEvent), UnityEngineUIInputFieldOnChangeEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.SubmitEvent), UnityEngineUIInputFieldSubmitEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Toggle), UnityEngineUIToggleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Toggle.ToggleEvent), UnityEngineUIToggleToggleEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Slider), UnityEngineUISliderWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Slider.SliderEvent), UnityEngineUISliderSliderEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Scrollbar), UnityEngineUIScrollbarWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Scrollbar.ScrollEvent), UnityEngineUIScrollbarScrollEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ScrollRect), UnityEngineUIScrollRectWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ScrollRect.ScrollRectEvent), UnityEngineUIScrollRectScrollRectEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ColorBlock), UnityEngineUIColorBlockWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Input), UnityEngineInputWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Sprite), UnityEngineSpriteWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RectTransform.Axis), UnityEngineRectTransformAxisWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RectTransform.Edge), UnityEngineRectTransformEdgeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Slider.Direction), UnityEngineUISliderDirectionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.Origin360), UnityEngineUIImageOrigin360Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.Origin180), UnityEngineUIImageOrigin180Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.Origin90), UnityEngineUIImageOrigin90Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.OriginHorizontal), UnityEngineUIImageOriginHorizontalWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.OriginVertical), UnityEngineUIImageOriginVerticalWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.FillMethod), UnityEngineUIImageFillMethodWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Image.Type), UnityEngineUIImageTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Dropdown), UnityEngineUIDropdownWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Dropdown.DropdownEvent), UnityEngineUIDropdownDropdownEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Dropdown.OptionData), UnityEngineUIDropdownOptionDataWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Dropdown.OptionDataList), UnityEngineUIDropdownOptionDataListWrap.__Register);
				
				translator.DelayWrapLoader(typeof(System.Type), SystemTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(System.Reflection.Missing), SystemReflectionMissingWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Random.State), UnityEngineRandomStateWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Camera.MonoOrStereoscopicEye), UnityEngineCameraMonoOrStereoscopicEyeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Camera.StereoscopicEye), UnityEngineCameraStereoscopicEyeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.LineType), UnityEngineUIInputFieldLineTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.CharacterValidation), UnityEngineUIInputFieldCharacterValidationWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.InputType), UnityEngineUIInputFieldInputTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.InputField.ContentType), UnityEngineUIInputFieldContentTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Toggle.ToggleTransition), UnityEngineUIToggleToggleTransitionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Scrollbar.Direction), UnityEngineUIScrollbarDirectionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ScrollRect.ScrollbarVisibility), UnityEngineUIScrollRectScrollbarVisibilityWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ScrollRect.MovementType), UnityEngineUIScrollRectMovementTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Selectable), UnityEngineUISelectableWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Selectable.Transition), UnityEngineUISelectableTransitionWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.EventSystems.BaseEventData), UnityEngineEventSystemsBaseEventDataWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Events.UnityEvent<UnityEngine.EventSystems.BaseEventData>), UnityEngineEventsUnityEvent_1_UnityEngineEventSystemsBaseEventData_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(System.Collections.Generic.List<UnityEngine.EventSystems.EventTrigger.Entry>), SystemCollectionsGenericList_1_UnityEngineEventSystemsEventTriggerEntry_Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.MaskableGraphic), UnityEngineUIMaskableGraphicWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.MaskableGraphic.CullStateChangedEvent), UnityEngineUIMaskableGraphicCullStateChangedEventWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.Graphic), UnityEngineUIGraphicWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AnimatorClipInfo), UnityEngineAnimatorClipInfoWrap.__Register);
				
				translator.DelayWrapLoader(typeof(System.ValueType), SystemValueTypeWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Rigidbody2D), UnityEngineRigidbody2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.EmitParams), UnityEngineParticleSystemEmitParamsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.Particle), UnityEngineParticleSystemParticleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.CustomDataModule), UnityEngineParticleSystemCustomDataModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.TrailModule), UnityEngineParticleSystemTrailModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.LightsModule), UnityEngineParticleSystemLightsModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.TextureSheetAnimationModule), UnityEngineParticleSystemTextureSheetAnimationModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.SubEmittersModule), UnityEngineParticleSystemSubEmittersModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.TriggerModule), UnityEngineParticleSystemTriggerModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.CollisionModule), UnityEngineParticleSystemCollisionModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.NoiseModule), UnityEngineParticleSystemNoiseModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.ExternalForcesModule), UnityEngineParticleSystemExternalForcesModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.RotationBySpeedModule), UnityEngineParticleSystemRotationBySpeedModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.RotationOverLifetimeModule), UnityEngineParticleSystemRotationOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.SizeBySpeedModule), UnityEngineParticleSystemSizeBySpeedModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.SizeOverLifetimeModule), UnityEngineParticleSystemSizeOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.ColorBySpeedModule), UnityEngineParticleSystemColorBySpeedModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.ColorOverLifetimeModule), UnityEngineParticleSystemColorOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.ForceOverLifetimeModule), UnityEngineParticleSystemForceOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.InheritVelocityModule), UnityEngineParticleSystemInheritVelocityModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.LimitVelocityOverLifetimeModule), UnityEngineParticleSystemLimitVelocityOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.VelocityOverLifetimeModule), UnityEngineParticleSystemVelocityOverLifetimeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.ShapeModule), UnityEngineParticleSystemShapeModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.EmissionModule), UnityEngineParticleSystemEmissionModuleWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.MinMaxCurve), UnityEngineParticleSystemMinMaxCurveWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.ParticleSystem.Burst), UnityEngineParticleSystemBurstWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.MeshRenderer), UnityEngineMeshRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.CircleCollider2D), UnityEngineCircleCollider2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Texture2D), UnityEngineTexture2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Texture2D.EXRFlags), UnityEngineTexture2DEXRFlagsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.CanvasRenderer), UnityEngineCanvasRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Rect), UnityEngineRectWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Matrix4x4), UnityEngineMatrix4x4Wrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.TextGenerator), UnityEngineTextGeneratorWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.TextGenerationSettings), UnityEngineTextGenerationSettingsWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ToggleGroup), UnityEngineUIToggleGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.BoxCollider), UnityEngineBoxColliderWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.RuntimeAnimatorController), UnityEngineRuntimeAnimatorControllerWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.AsyncOperation), UnityEngineAsyncOperationWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.Animation), UnityEngineAnimationWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.CapsuleCollider), UnityEngineCapsuleColliderWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.PolygonCollider2D), UnityEnginePolygonCollider2DWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.MeshCollider), UnityEngineMeshColliderWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.ContentSizeFitter), UnityEngineUIContentSizeFitterWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.LayoutRebuilder), UnityEngineUILayoutRebuilderWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.UI.GridLayoutGroup), UnityEngineUIGridLayoutGroupWrap.__Register);
				
				translator.DelayWrapLoader(typeof(UnityEngine.TrailRenderer), UnityEngineTrailRendererWrap.__Register);
				
				translator.DelayWrapLoader(typeof(ResManager), ResManagerWrap.__Register);
				
				
				
				translator.AddInterfaceBridgeCreator(typeof(InvokeLua.ICalc), InvokeLuaICalcBridge.__Create);
				
				translator.AddInterfaceBridgeCreator(typeof(XLuaTest.IExchanger), XLuaTestIExchangerBridge.__Create);
				
				translator.AddInterfaceBridgeCreator(typeof(System.Collections.IEnumerator), SystemCollectionsIEnumeratorBridge.__Create);
				
				translator.AddInterfaceBridgeCreator(typeof(CSCallLua.ItfD), CSCallLuaItfDBridge.__Create);
				
			});
		}
		
		
	}
	
}
namespace XLua
{
	public partial class ObjectTranslator
	{
		static XLua.CSObjectWrap.XLua_Gen_Initer_Register__ s_gen_reg_dumb_obj = new XLua.CSObjectWrap.XLua_Gen_Initer_Register__();
		static XLua.CSObjectWrap.XLua_Gen_Initer_Register__ gen_reg_dumb_obj {get{return s_gen_reg_dumb_obj;}}
	}
	
	internal partial class InternalGlobals
    {
	    
	    static InternalGlobals()
		{
		    extensionMethodMap = new Dictionary<Type, IEnumerable<MethodInfo>>()
			{
			    
			};
			
			genTryArrayGetPtr = StaticLuaCallbacks.__tryArrayGet;
            genTryArraySetPtr = StaticLuaCallbacks.__tryArraySet;
		}
	}
}
