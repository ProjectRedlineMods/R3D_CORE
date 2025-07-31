class R3D_LoadableCargoSlot : EntitySlotInfo {};

class R3D_LoadAircraftCargoAction : ScriptedUserAction
{
	ADM_AirplaneControllerComponent m_AirplaneController;
	ADM_AirplaneInput m_Input;
	SlotManagerComponent m_SlotManager;
	R3D_LoadableCargoSlot m_TargetSlot;
	
	[Attribute()]
	string RampBoneName;
	int RampBone = -1;
	float RampBoneAngleDeg = 0.0;
	
	[Attribute()]
	string SlotName; // TODO does nothing, would be needed for multi slots
	
	[Attribute()]
	vector LoadingPos;
	
	[Attribute("5.0")]
	float MaxLoadDist;
	
	IEntity curPayload = null;
	IEntity loadTarget = null;
	
	void UpdateRampBoneAngle()
	{
		// And you may ask yourself: Why is this necessary? We use a smoothing variable that isn't exposed for the animation.
		// So, the only true reference on where the door is (outside the anim graph) is the bone.
		IEntity owner = GetOwner();
		vector mat[4];
		owner.GetAnimation().GetBoneMatrix(RampBone, mat);
		vector angles = Math3D.MatrixToAngles(mat);
		RampBoneAngleDeg = angles[1];
	}
	
	bool IsRampOpen()
	{
		return (RampBoneAngleDeg > -38.0); // Hack: C130 FLIGHT POS is -37.47
	}
	
	bool IsRampGrounded()
	{
		return (RampBoneAngleDeg > -23.0); // Hack: C130 OPEN POS is -22.47
	}
	
	ref array<IEntity> targets = {};
	bool QueryCallback(IEntity e)
	{
		if (e != GetOwner() && e.Type().IsInherited(BaseVehicle))
			targets.Insert(e);
		return true;
	}
	
	IEntity GetBestTarget()
	{
		targets.Clear();
		
		vector LoadWorldPos = GetOwner().CoordToParent(LoadingPos);
		
		GetGame().GetWorld().QueryEntitiesBySphere(LoadWorldPos, MaxLoadDist, QueryCallback);
		IEntity closest = null;
		float closest_dist = float.MAX;
		foreach (IEntity t : targets)
		{
			// kind of assuming t is in world space here, which is an okay-ish assumption
			float dist = vector.Distance(LoadWorldPos, t.GetOrigin());
			if (closest_dist > dist)
			{
				closest = t;
				closest_dist = dist;
			}
		}
		return closest;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_AirplaneController = ADM_AirplaneControllerComponent.Cast(pOwnerEntity.FindComponent(ADM_AirplaneControllerComponent));
		m_Input = ADM_AirplaneInput.Cast(pOwnerEntity.FindComponent(ADM_AirplaneInput));
		m_SlotManager = SlotManagerComponent.Cast(pOwnerEntity.FindComponent(SlotManagerComponent));
		RampBone = pOwnerEntity.GetAnimation().GetBoneIndex(RampBoneName);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ref array<EntitySlotInfo> slots = {};
		m_SlotManager.GetSlotInfos(slots);
		foreach (EntitySlotInfo s : slots)
		{
			if (R3D_LoadableCargoSlot.Cast(s))
				m_TargetSlot = R3D_LoadableCargoSlot.Cast(s); // This is my slot, it was made for me
		}
		
		if (!m_TargetSlot || !m_TargetSlot.IsEnabled()) return;
		
		IEntity curCargo = m_TargetSlot.GetAttachedEntity();
		if (curCargo) {
			m_TargetSlot.DetachEntity(true);
			UpdateRampBoneAngle();
			if (IsRampGrounded()) { // For now, if we have cargo on the ground, just move it out. In flight, leave it be for physics to handle.
				vector LoadWorldPos = GetOwner().CoordToParent(LoadingPos);
				curCargo.SetOrigin(LoadWorldPos);
			}
			curPayload = null;
			// Clear mass if we have the component. TODO: This will reset other pylon masses...
			R3D_DynamicMassComponent mcmp = R3D_DynamicMassComponent.Cast(pOwnerEntity.FindComponent(R3D_DynamicMassComponent));
			if (mcmp) {
				mcmp.ResetInitialMassProperties();
			}
		} else if (loadTarget) {
			float mass = loadTarget.GetPhysics().GetMass();
			R3D_DynamicMassComponent mcmp = R3D_DynamicMassComponent.Cast(pOwnerEntity.FindComponent(R3D_DynamicMassComponent));
			if (mcmp) {
				mcmp.AddMass(mass, "0 1.5 0"); // TODO hack for now, get the actual slot pos later
			}
			m_TargetSlot.AttachEntity(loadTarget);
			curPayload = loadTarget;
			loadTarget = null;
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (curPayload) {
			outName = "Unload Vehicle";
			loadTarget = null;
			return true;
		}
		
		loadTarget = GetBestTarget();
		string loadTargetName = "Load Vehicle (None)";
		if (loadTarget) {
			SCR_EditableVehicleComponent editinfo = SCR_EditableVehicleComponent.Cast(loadTarget.FindComponent(SCR_EditableVehicleComponent));
			if (editinfo)
				outName = string.Format("Load Vehicle (%1)", editinfo.GetDisplayName());
			else
				outName = "Load Vehicle (Unknown)";
		}
		
		return true;
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		UpdateRampBoneAngle();
		
		if (curPayload && IsRampOpen())
			return true;
		
		if (!curPayload && loadTarget && IsRampGrounded())
			return true;
		
		return false;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		// TODO: Check for JM/Pilot role to prevent people from trolling and throwing cargo out, lol
		return true;
	}
}