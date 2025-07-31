class R3D_ToggleAircraftAPU : ScriptedUserAction
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
			m_AirplaneController.Rpc_Server_ToggleAPU();
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (m_AirplaneController.GetAPUStatus())
		{
			outName = "Stop APU";
		}
		else
		{
			outName = "Start APU";
		}
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_AirplaneController.GetPowerStatus();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		return true; // todo pilot/copilot check
	}
}