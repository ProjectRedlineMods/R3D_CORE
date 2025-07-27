class ADM_AirplaneControllerComponentClass : CarControllerComponentClass
{
}

/*!
	Class responsible for game airplane.
	It connects all airplane components together and handles all comunication between them.
*/
class ADM_AirplaneControllerComponent: CarControllerComponent
{
	[RplProp()] 
	protected bool m_bIsEngineOn = false; //Set the engine to be off by default
	
	[RplProp()] 
	protected bool m_bGearDeployed = true; // true = deployed, false = retracted

	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.1", desc: "Adjustment on actuation velocity when applying trim")]
	float m_fTrimVelocityAdjustment;
	
	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.01")]
	float m_fThrustVelocity;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Return steering to zero when no input applied.", defvalue: "1")]
	bool m_bAutoZeroSteerAngle;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Max angle defined in vehicle wheeled simulation steering component. THIS MUST MATCH FOR STEERING STRENGTH TO BE MAPPED CORRECTLY!")]
	float m_fMaxSteerAngle;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when actively steering away from center [km/h deg/s]")]
	ref Curve m_fSteeringForwardStrength;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when actively steering away toward center [km/h deg/s]")]
	ref Curve m_fSteeringBackwardStrength;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when no steering applied [km/h deg/s]")]
	ref Curve m_fSteeringCenterStrength;
	
	[Attribute(category: "Fixed Wing Simulation", defvalue: "30", desc: "degrees")]
	protected float m_fMinZoomFOV;
	
	protected ref array<ADM_EngineComponent> m_Engines = {};
	protected ADM_FixedWingSimulation m_FixedWingSim;
	protected ADM_AirplaneInput m_AirplaneInput;
	protected RplComponent m_RplComponent;
	protected CameraManager m_CameraManager;
	protected float m_fFOVMax;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		array<Managed> components = {};
		owner.FindComponents(ADM_EngineComponent, components);
		foreach (Managed cmp : components)
		{
			ADM_EngineComponent engCmp = ADM_EngineComponent.Cast(cmp);
			if (engCmp) m_Engines.Insert(engCmp);
		}
		
		m_FixedWingSim = ADM_FixedWingSimulation.Cast(owner.FindComponent(ADM_FixedWingSimulation));
		m_AirplaneInput = ADM_AirplaneInput.Cast(owner.FindComponent(ADM_AirplaneInput));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_CameraManager = GetGame().GetCameraManager();
		
		if (m_CameraManager)
			m_fFOVMax = m_CameraManager.GetVehicleFOV();
		else
			m_fFOVMax = 75;
	}
	
	//------------------------------------------------------------------------------------------------
	ADM_FixedWingSimulation GetFixedWingSimulation()
	{
		return m_FixedWingSim;
	}
	
	//------------------------------------------------------------------------------------------------
	ADM_AirplaneInput GetAirplaneInput()
	{
		return m_AirplaneInput;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ADM_EngineComponent> GetEngines()
	{
		return m_Engines;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAirplaneEngineOn()
	{
		return m_bIsEngineOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetTrim()
	{
		m_FixedWingSim.ResetTrim();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFOV(float fovZoom)
	{
		float fov = m_fFOVMax + (m_fMinZoomFOV-m_fFOVMax)*fovZoom;
		m_CameraManager.SetVehicleFOV(fov);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Server_ToggleEngine(int idx)
	{
		foreach(ADM_EngineComponent engine : m_Engines)
		{
			if (idx < 0 || engine.GetIndex() == idx) {
				engine.SetEngineStatus(!engine.GetEngineStatus());
				m_bIsEngineOn |= engine.GetEngineStatus();
			}
		}
		auto sigs = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		int sig_idx = sigs.AddOrFindMPSignal("AircraftIsEngineOn", 1, 30, 0, SignalCompressionFunc.Bool);
		sigs.SetSignalValue(sig_idx, m_bIsEngineOn);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleGear()
	{
		Rpc(Rpc_Server_ToggleGear);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Server_ToggleGear()
	{
		m_bGearDeployed = !m_bGearDeployed;
		
		for (int i=0; i < m_FixedWingSim.m_bGearDeployed.Count(); i++)
		{
			m_FixedWingSim.m_bGearDeployed[i] = m_bGearDeployed;
		}
		Replication.BumpMe();
	}
	
}