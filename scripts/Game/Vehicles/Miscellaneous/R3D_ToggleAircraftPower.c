class R3D_ToggleAircraftPower : ScriptedUserAction
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
			m_AirplaneController.Rpc_Server_TogglePower();
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (m_AirplaneController.GetPowerStatus())
		{
			outName = "Battery Off";
		}
		else
		{
			outName = "Battery On";
		}
		return true;
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		return m_Input.IsControlActive();
	}
}