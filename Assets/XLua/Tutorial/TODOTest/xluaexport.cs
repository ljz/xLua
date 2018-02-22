﻿using System.Collections;
using System.Collections.Generic;
using XLua;
using UnityEngine;

public static class CodGen
{
    [LuaCallCSharp]
    public static List<System.Type> module_lua_call_cs_list = new List<System.Type>()
    {
        // UnityEngine
        typeof(GameObject),
        typeof(UnityEngine.Object),
        typeof(System.Object),
        typeof(UnityEngine.Font),
        typeof(UnityEngine.SceneManagement.SceneManager),
        typeof(UnityEngine.TextAnchor),
        typeof(UnityEngine.PlayerPrefs),
        typeof(UnityEngine.WrapMode),
        typeof(UnityEngine.RigidbodyType2D),
        typeof(UnityEngine.Resources),
        typeof(UnityEngine.Camera),
        typeof(UnityEngine.Debug),
        typeof(UnityEngine.Random),


        typeof(Mathf),
        typeof(Color),
        typeof(Vector3),
        typeof(Vector2),
        typeof(Vector4),
        typeof(Transform),
        typeof(Quaternion),
        typeof(Animator),
        typeof(AnimatorStateInfo),
        typeof(SpriteRenderer),


        typeof(UnityEngine.QualitySettings),
        typeof(UnityEngine.AI.NavMeshAgent),
        typeof(UnityEngine.AI.NavMeshObstacle),
        typeof(UnityEngine.AI.NavMeshPath),
        typeof(UnityEngine.AI.NavMesh),
        typeof(UnityEngine.AI.NavMeshPathStatus),
        typeof(UnityEngine.AI.NavMeshHit),
        typeof(UnityEngine.AI.NavMeshTriangulation),
        typeof(UnityEngine.EventSystems.EventTrigger),
        typeof(UnityEngine.EventSystems.BaseRaycaster),
        typeof(UnityEngine.EventSystems.UIBehaviour),
        typeof(MonoBehaviour),
        typeof(Behaviour),



        typeof(UnityEngine.EventSystems.EventTrigger.TriggerEvent),
        typeof(UnityEngine.EventSystems.EventSystem),
        typeof(UnityEngine.EventSystems.EventTriggerType),
        typeof(UnityEngine.EventSystems.PointerEventData),
        typeof(UnityEngine.EventSystems.EventTrigger.Entry),
        typeof(UnityEngine.Rendering.RenderQueue),
        typeof(Material),
        typeof(CanvasGroup),
        typeof(UnityEngine.RigidbodyConstraints2D),
        typeof(UnityEngine.ParticleSystem),
        typeof(UnityEngine.ParticleSystem.MainModule),
        typeof(UnityEngine.ParticleSystem.MinMaxGradient),
        typeof(UnityEngine.RectTransform),
        typeof(UnityEngine.Renderer),
        typeof(UnityEngine.UI.GraphicRaycaster),
        typeof(UnityEngine.UI.GraphicRaycaster.BlockingObjects),
        typeof(UnityEngine.UI.VerticalLayoutGroup),
        typeof(UnityEngine.RectOffset),


        typeof(UnityEngine.UI.HorizontalOrVerticalLayoutGroup),
        typeof(UnityEngine.UI.LayoutGroup),


        typeof(UnityEngine.Events.UnityEvent<System.Boolean>),
        typeof(UnityEngine.Events.UnityEvent<System.String>),
        typeof(UnityEngine.Events.UnityEvent<System.Single>),
        typeof(UnityEngine.LayerMask),
        typeof(UnityEngine.CameraClearFlags),
        typeof(UnityEngine.RenderTexture),
        typeof(UnityEngine.UI.RawImage),
        typeof(UnityEngine.ParticleSystemRenderer),
        typeof(UnityEngine.FontStyle),
        typeof(UnityEngine.Canvas),


        //// UnityEngine.UI
        typeof(UnityEngine.Events.UnityEvent),
        typeof(UnityEngine.Events.UnityEventBase),
        typeof(UnityEngine.UI.Button),
        typeof(UnityEngine.UI.Button.ButtonClickedEvent),
        typeof(UnityEngine.UI.Image),
        typeof(UnityEngine.UI.Text),
        typeof(UnityEngine.UI.InputField),
        typeof(UnityEngine.UI.InputField.OnChangeEvent),
        typeof(UnityEngine.UI.InputField.SubmitEvent),
        typeof(UnityEngine.UI.Toggle),
        typeof(UnityEngine.UI.Toggle.ToggleEvent),
        typeof(UnityEngine.UI.Slider),
        typeof(UnityEngine.UI.Slider.SliderEvent),
        typeof(UnityEngine.UI.Scrollbar),
        typeof(UnityEngine.UI.Scrollbar.ScrollEvent),
        typeof(UnityEngine.UI.ScrollRect),
        typeof(UnityEngine.UI.ScrollRect.ScrollRectEvent),
        typeof(UnityEngine.UI.ColorBlock),
        typeof(UnityEngine.Input),
        typeof(UnityEngine.Sprite),
        typeof(UnityEngine.RectTransform.Axis),
        typeof(UnityEngine.RectTransform.Edge),
        typeof(UnityEngine.Component),
        typeof(UnityEngine.UI.Slider.Direction),
        //typeof(TMPro.TextMeshProUGUI),
        //typeof(TMPro.TMP_Text),
        typeof(UnityEngine.UI.Image.Origin360),
        typeof(UnityEngine.UI.Image.Origin180),
        typeof(UnityEngine.UI.Image.Origin90),
        typeof(UnityEngine.UI.Image.OriginHorizontal),
        typeof(UnityEngine.UI.Image.OriginVertical),
        typeof(UnityEngine.UI.Image.FillMethod),
        typeof(UnityEngine.UI.Image.Type),
        typeof(UnityEngine.UI.Dropdown),
        typeof(UnityEngine.UI.Dropdown.DropdownEvent),
        typeof(UnityEngine.UI.Dropdown.OptionData),
        typeof(UnityEngine.UI.Dropdown.OptionDataList),
        typeof(System.Type),
        typeof(System.Reflection.Missing),
        typeof(UnityEngine.Random.State),
        typeof(Camera.MonoOrStereoscopicEye),
        typeof(Camera.StereoscopicEye),
        typeof(UnityEngine.UI.InputField.LineType),
        typeof(UnityEngine.UI.InputField.CharacterValidation),
        typeof(UnityEngine.UI.InputField.InputType),
        typeof(UnityEngine.UI.InputField.ContentType),
        typeof(UnityEngine.UI.Toggle.ToggleTransition),
        typeof(UnityEngine.UI.Scrollbar.Direction),
        typeof(UnityEngine.UI.ScrollRect.ScrollbarVisibility),
        typeof(UnityEngine.UI.ScrollRect.MovementType),
        typeof(UnityEngine.UI.Selectable),
        typeof(UnityEngine.UI.Selectable.Transition),
        typeof(UnityEngine.EventSystems.BaseEventData),
        typeof(UnityEngine.EventSystems.EventTrigger.Entry),
        typeof(UnityEngine.Events.UnityEvent<UnityEngine.EventSystems.BaseEventData>),
        typeof(List<UnityEngine.EventSystems.EventTrigger.Entry>),
        typeof(UnityEngine.UI.MaskableGraphic),
        typeof(UnityEngine.UI.MaskableGraphic.CullStateChangedEvent),
        typeof(UnityEngine.UI.Graphic),
        typeof(UnityEngine.AnimatorClipInfo),
        typeof(System.ValueType),
        //typeof(System.IO.Directory),
        typeof(UnityEngine.Rigidbody2D),
        typeof(UnityEngine.ParticleSystem.EmitParams),
        typeof(UnityEngine.ParticleSystem.Particle),
        typeof(UnityEngine.ParticleSystem.CustomDataModule),
        typeof(UnityEngine.ParticleSystem.TrailModule),
        typeof(UnityEngine.ParticleSystem.LightsModule),
        typeof(UnityEngine.ParticleSystem.TextureSheetAnimationModule),
        typeof(UnityEngine.ParticleSystem.SubEmittersModule),
        typeof(UnityEngine.ParticleSystem.TriggerModule),
        typeof(UnityEngine.ParticleSystem.CollisionModule),
        typeof(UnityEngine.ParticleSystem.NoiseModule),
        typeof(UnityEngine.ParticleSystem.ExternalForcesModule),
        typeof(UnityEngine.ParticleSystem.RotationBySpeedModule),
        typeof(UnityEngine.ParticleSystem.RotationOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.SizeBySpeedModule),
        typeof(UnityEngine.ParticleSystem.SizeOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.ColorBySpeedModule),
        typeof(UnityEngine.ParticleSystem.ColorOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.ForceOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.InheritVelocityModule),
        typeof(UnityEngine.ParticleSystem.LimitVelocityOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.VelocityOverLifetimeModule),
        typeof(UnityEngine.ParticleSystem.ShapeModule),
        typeof(UnityEngine.ParticleSystem.EmissionModule),
        typeof(UnityEngine.ParticleSystem.MinMaxCurve),
        typeof(UnityEngine.ParticleSystem.Burst),
        typeof(UnityEngine.MeshRenderer),
        typeof(UnityEngine.CircleCollider2D),
        typeof(UnityEngine.Texture2D),
        typeof(UnityEngine.Texture2D.EXRFlags),
        typeof(UnityEngine.CanvasRenderer),
        typeof(UnityEngine.Rect),
        typeof(UnityEngine.Matrix4x4),
        typeof(UnityEngine.TextGenerator),
        typeof(UnityEngine.TextGenerationSettings),
        typeof(UnityEngine.UI.ToggleGroup),
        typeof(UnityEngine.BoxCollider),
        typeof(UnityEngine.RuntimeAnimatorController),
        typeof(UnityEngine.AsyncOperation),
        typeof(UnityEngine.Animation),
        typeof(UnityEngine.CapsuleCollider),
        typeof(UnityEngine.PolygonCollider2D),
        typeof(UnityEngine.MeshCollider),
        typeof(UnityEngine.UI.ContentSizeFitter),
        typeof(System.IO.Directory),
        typeof(UnityEngine.UI.LayoutRebuilder),
        typeof(UnityEngine.UI.GridLayoutGroup),
        typeof(TrailRenderer),
        typeof(SkinnedMeshRenderer),
        //typeof(InverseMask),
        //typeof(UnityEngine.UI.Graphic),

    };





    }