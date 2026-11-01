modded class MissionServer
{
    void MissionServer()
    {
		HDSN_BreachingChargeLogger.CreateInstance();
		HDSN_BreachingChargeConfigManager.ClearInstance();
        HDSN_BreachingChargeConfigManager.GetInstance();
    }
}
