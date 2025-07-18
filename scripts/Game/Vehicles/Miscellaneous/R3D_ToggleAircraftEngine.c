class R3D_ToggleAircraftEngineAction : ScriptedUserAction
{
	[Attribute(defvalue: "-1")]
	int idx;
	
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
			m_AirplaneController.Rpc_Server_ToggleEngine(idx);
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		bool myEngineOn = false;
		
		foreach(ADM_EngineComponent engine : m_AirplaneController.GetEngines())
		{
			if (idx < 0 || engine.GetIndex() == idx)
				myEngineOn = engine.GetEngineStatus();
		}
		
		if (myEngineOn)
		{
			outName = "Stop Engine";
			if (idx != -1) outName += string.Format(" %1", idx);
		}
		else
		{
			outName = "Start Engine";
			if (idx != -1) outName += string.Format(" %1", idx);
		}
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_Input.IsControlActive();
	}
}