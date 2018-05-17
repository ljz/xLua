
function InitUnityConst()
	if CS == nil then
		error("CS is nil")
		return
	end

	RegGlobalObj("UnityEngine", CS.UnityEngine)
	RegGlobalObj("Vector3", UnityEngine.Vector3)
	RegGlobalObj("Vector2", UnityEngine.Vector2)
	RegGlobalObj("Quaternion", UnityEngine.Quaternion)
	RegGlobalObj("Color", UnityEngine.Color)
	RegGlobalObj("Transform", UnityEngine.Transform)
	RegGlobalObj("RectTransform", UnityEngine.RectTransform)
	RegGlobalObj("Animator", UnityEngine.Animator)
	RegGlobalObj("Sprite", UnityEngine.Sprite)
	RegGlobalObj("SpriteRenderer", UnityEngine.SpriteRenderer)
	RegGlobalObj("NavMeshAgent", UnityEngine.AI.NavMeshAgent)
	RegGlobalObj("NavMeshPath", UnityEngine.AI.NavMeshPath)
	RegGlobalObj("NavMeshPathStatus", UnityEngine.AI.NavMeshPathStatus)
	RegGlobalObj("Random", UnityEngine.Random)
	RegGlobalObj("Time", UnityEngine.Time)
	RegGlobalObj("GameObject", UnityEngine.GameObject)
	RegGlobalObj("Button", UnityEngine.UI.Button)
	RegGlobalObj("Text", UnityEngine.UI.Text)
	RegGlobalObj("Slider", UnityEngine.UI.Slider)
	RegGlobalObj("Image", UnityEngine.UI.Image)
	RegGlobalObj("Application", CS.UnityEngine.Application)
	RegGlobalObj("Download", CS.Download.instance)
	RegGlobalObj("Bugly", CS.BuglyAgent)
	RegGlobalObj("Camera", UnityEngine.Camera)
	RegGlobalObj("LineRenderer", UnityEngine.LineRenderer)
	RegGlobalObj("Destroy", UnityEngine.Object.Destroy)
end