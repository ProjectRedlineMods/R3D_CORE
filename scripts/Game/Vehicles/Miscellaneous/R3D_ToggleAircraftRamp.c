[BaseContainerProps()]
class R3D_RampState
{
	[Attribute()]
	string Name;
	
	[Attribute()]
	float RampAngle;
	
	[Attribute()]
	bool CanSetInFlight;
}

class R3D_ToggleAircraftRampAction : ScriptedUserAction
{
	ADM_AirplaneControllerComponent m_AirplaneController;
	ADM_AirplaneInput m_Input;
	
	[Attribute()]
	string m_sSignal;
	
	[Attribute()]
	string m_RampName;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref R3D_RampState> m_RampStates;
	
	[Attribute()]
	int m_DefaultState = 0;
	
	int m_CountFlightStates = 0;
	
	int m_CurrentState = 0;
	
	int NextState(int from, bool inflight)
	{
		int idx = (from + 1) % m_RampStates.Count();
		if (inflight)
		{
			if (m_CountFlightStates < 1)
				return -1;
			
			// Cycle until we find a state that can be set
			for (int i = 0; !m_RampStates[idx].CanSetInFlight && i < m_RampStates.Count(); i++)
				idx = (idx + 1) % m_RampStates.Count();
		}
		
		return idx;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_AirplaneController = ADM_AirplaneControllerComponent.Cast(pOwnerEntity.FindComponent(ADM_AirplaneControllerComponent));
		m_Input = ADM_AirplaneInput.Cast(pOwnerEntity.FindComponent(ADM_AirplaneInput));
		if (m_RampStates)
		{
			foreach (auto state : m_RampStates)
			{
				if (state.CanSetInFlight)
					m_CountFlightStates += 1;
			}
		}
		
		m_CurrentState = m_DefaultState;
		if (Replication.IsServer())
		{
			m_AirplaneController.Rpc_Server_SetSignalValue(m_sSignal, m_RampStates[m_CurrentState].RampAngle);
		}
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		int idx = NextState(m_CurrentState, GetOwner().GetPhysics().GetVelocity().LengthSq() > 1);
		if (idx < 0) return;
		m_CurrentState = idx;
		if (Replication.IsServer())
		{
			m_AirplaneController.Rpc_Server_SetSignalValue(m_sSignal, m_RampStates[idx].RampAngle);
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		int idx = NextState(m_CurrentState, GetOwner().GetPhysics().GetVelocity().LengthSq() > 1);
		if (idx < 0) return false;
		outName = string.Format("Set %1: %2", m_RampName, m_RampStates[idx].Name);
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return true; // TODO: Would be better to have a check for hydraulics here
	}
}