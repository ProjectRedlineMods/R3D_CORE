class R3D_ToggleAircraftGearAction : ScriptedUserAction
{
	ADM_AirplaneControllerComponent m_AirplaneController;
	ADM_AirplaneInput m_Input;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_AirplaneController = ADM_AirplaneControllerComponent.Cast(pOwnerEntity.FindComponent(ADM_AirplaneControllerComponent));
		m_Input = ADM_AirplaneInput.Cast(pOwnerEntity.FindComponent(ADM_AirplaneInput));
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (Replication.IsServer())
		{
			m_AirplaneController.ToggleGear();
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		bool GearDeployed = m_AirplaneController.IsGearDeployed();
		
		if (GearDeployed)
		{
			outName = "Raise Landing Gear";
		}
		else
		{
			outName = "Lower Landing Gear";
		}
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_Input.IsControlActive();
	}
}