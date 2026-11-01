class HDSN_ChargeParams
{
	string Classname;

	int DamageToObjects;
	float DamageToDestroyableObjectsRadius;
	int VerticalDistanceModeObjects; //0 = No vertical distance checks, 1 = distance from charge, 2 = distance from charge target, 3 = checks both distances
	float MaxVerticalDistanceObjects;
	float MaxDamageToPlayers;
	float MinDamageToPlayers;
	float DamageToPlayersRadius;
	float MaxDamageToPlayersRadius;
	float MaxVerticalDistancePlayers;

	bool OnlyDestroyLocks;
	bool DeleteObjectsDirectly;
	bool DestroyLocksFirst;

	float PlacementDistance;
	float MinPlacementDistance;
	float ToolDamageOnDefuse;
	bool DestroyOtherCharges;
	bool AutomaticallyArm;

	float TimeToPlant;
	float TimeToExplode;
	float TimeToDefuse;

	float LightBrightness;
	float LightRadius;
	vector LightColorStart;
	vector LightColorHalfway;
	vector LightColorEnd;

	string BeepingSoundSet;
	float BeepingSoundEndTime;
	float SwitchInterval;
	string ExplosionSoundSet;

	ref array<string> DefuseTools;

	void HDSN_ChargeParams()
	{
		DamageToObjects = 1;
		DamageToDestroyableObjectsRadius = 0;
		TimeToPlant = 30;
		TimeToExplode = 180;
		TimeToDefuse = 20;
		MaxDamageToPlayers = 100;
		MinDamageToPlayers = 50;
		DamageToPlayersRadius = 5;
		MaxDamageToPlayersRadius = 2;
		LightBrightness = 0.5;
		LightRadius = 10.0;
		LightColorStart = "0.0 1.0 0.0"; //Green
		LightColorHalfway = "1.0 1.0 0.0"; //Yellow
		LightColorEnd= "1.0 0.0 0.0"; //Red
		BeepingSoundSet = "HDSN_SoundSet_Beeping";
		BeepingSoundEndTime = 2;
		SwitchInterval = 2.0;
		ExplosionSoundSet = "Landmine_Explosion_SoundSet";

		OnlyDestroyLocks = false;
		DeleteObjectsDirectly = false;
		DestroyLocksFirst = false;

		PlacementDistance = 2.0;
		MinPlacementDistance = 0.0;
		ToolDamageOnDefuse = 10;

		DefuseTools = new array<string>;
		DefuseTools.Insert("Unarmed");

		VerticalDistanceModeObjects = 0;
		MaxVerticalDistanceObjects = 0.0;
		MaxVerticalDistancePlayers = 0.0;

		DestroyOtherCharges = false;
		AutomaticallyArm = false;
	}

	bool CanBeDefusedBy(string name)
	{
		for(int i = 0; i < DefuseTools.Count(); i++)
		{
			if(DefuseTools[i] == name)
			{
				return true;
			}
		}

		return false;
	}
}

class HDSN_TierParams
{
	string Name;
	int Health;
	ref array<string> AcceptedChargeTypes;

	void HDSN_TierParams(string name = "Default", int health = 1)
	{
		Name = name;
		Health = health;
		AcceptedChargeTypes = new array<string>;
	}

	bool Contains(string chargename)
	{
		if(AcceptedChargeTypes.Find(chargename) >= 0)
		{
			return true;
		}

		return false;
	}

	int GetObjectHealth()
	{
		return Health;
	}

	bool IsChargeTypeAllowed(string name)
	{
		return Contains(name);
	}
}

class HDSN_BreachingChargeConfigData
{
	bool CreateLogs;
	bool EnableExperimentalPlacing;
	float ExperimentalPlacingRaycastDistance;
	ref array<ref HDSN_ChargeParams> Charges;
	ref array<ref HDSN_TierParams> Tiers;
	ref map<string, string> DestroyableObjects;

	void HDSN_BreachingChargeConfigData()
	{
		Charges = new array<ref HDSN_ChargeParams>;
		Tiers = new array<ref HDSN_TierParams>;
		DestroyableObjects = new map<string, string>;
	}
}

class HDSN_BreachingChargeConfigManager
{
	const private static string configPath = "$profile:/BreachingCharge/";
	const private static string configName = "breachingcharge.json";
	static ref array<string> timezoneTypes = {"GMT","CEST","EST","PST","CST","UTC"};

	private static ref HDSN_BreachingChargeConfigManager instance;
	static int HDSN_TimezonesIdentifier;
	static ref HDSN_BreachingChargeConfigData configData;

	void HDSN_BreachingChargeConfigManager()
	{
		if(GetGame().IsServer())
		{
			GetRPCManager().AddRPC("HDSN_BreachingCharge", "SendConfigToClient", this, SingleplayerExecutionType.Server);
			LoadConfig();
		}

		if(GetGame().IsClient())
		{
			GetRPCManager().AddRPC("HDSN_BreachingCharge", "ReceiveConfigFromServer", this, SingleplayerExecutionType.Client);
			RequestConfigFromServer();
		}
	}

	static HDSN_BreachingChargeConfigData GetConfig()
	{
		return GetInstance().GetConfigData();
	}

	static HDSN_BreachingChargeConfigManager GetInstance()
	{
		if(instance == null)
		{
			instance = new HDSN_BreachingChargeConfigManager();
		}
		return instance;
	}

	static void ClearInstance()
	{
		instance = null;
	}

	void LoadConfig()
	{
		configData  = new HDSN_BreachingChargeConfigData();

		if (FileExist(configPath + configName))
        {
			Print("[Breachingcharge]	An existing breachingcharge.json has been found!");
            JsonFileLoader<HDSN_BreachingChargeConfigData>.JsonLoadFile(configPath + configName, configData);
        }
        else
        {
			Print("[Breachingcharge]	No existing breachingcharge.json has been found, generating a new default json-file!");
            CreateDefaultConfig();
        }
	}

	void CreateDefaultConfig()
	{
		SetDefaultParams();

		if(!FileExist(configPath))
		{
			MakeDirectory(configPath);
		}

        JsonFileLoader<HDSN_BreachingChargeConfigData>.JsonSaveFile(configPath + configName, configData);
	}

	HDSN_BreachingChargeConfigData GetConfigData()
	{
		if(configData == null)
		{
			LoadConfig();
		}
		return configData;
	}

	protected void SaveConfigDataToJson()
	{
		Print("Saving config file breachingcharge.json");

		if(!FileExist(configPath))
		{
			MakeDirectory(configPath);
		}

		JsonFileLoader<HDSN_BreachingChargeConfigData>.JsonSaveFile(configPath + configName, configData);

		Print("Saved config file breachingcharge.json");
	}

	protected void RequestConfigFromServer()
	{
		if(GetGame().IsClient())
		{
			GetRPCManager().SendRPC("HDSN_BreachingCharge", "SendConfigToClient", null, true);
		}
	}

	protected void SendConfigToClient(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
        if(type == CallType.Server)
		{
			GetRPCManager().SendRPC("HDSN_BreachingCharge", "ReceiveConfigFromServer", new Param1<HDSN_BreachingChargeConfigData>(GetConfigData()), true, sender);
		}
	}

	protected void ReceiveConfigFromServer(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param1<HDSN_BreachingChargeConfigData> param;
        if(!ctx.Read(param)) return;
        if(type == CallType.Client)
		{
			configData = param.param1;
		}
		//SaveConfigDataToJson();
	}

	protected void SetDefaultParams()
	{
		configData.CreateLogs = true;
		configData.EnableExperimentalPlacing = false;
		configData.ExperimentalPlacingRaycastDistance = 0.3;

		InsertCharge("HDSN_BreachingCharge", 1);
		InsertCharge("HDSN_BreachingChargeHeavy", 2);

		InsertTier("Tier1", 1);
		InsertAcceptedChargeType("Tier1", "HDSN_BreachingCharge");
		InsertAcceptedChargeType("Tier1", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier2", 2);
		InsertAcceptedChargeType("Tier2", "HDSN_BreachingCharge");
		InsertAcceptedChargeType("Tier2", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier3", 4);
		InsertAcceptedChargeType("Tier3", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier4", 6);
		InsertAcceptedChargeType("Tier4", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier5", 8);
		InsertAcceptedChargeType("Tier5", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier6", 10);
		InsertAcceptedChargeType("Tier6", "HDSN_BreachingChargeHeavy");

		InsertTier("Tier7", 12);
		InsertAcceptedChargeType("Tier7", "HDSN_BreachingChargeHeavy");

		InsertDestroyableObject("Fence_Frame", "Tier1");
		InsertDestroyableObject("Fence", "Tier1");
		InsertDestroyableObject("Fence_HalfMetal", "Tier2");
		InsertDestroyableObject("Fence_Metal", "Tier3");

		InsertDestroyableObject("Fence_Gate", "Tier1");
		InsertDestroyableObject("Fence_Gate_HalfMetal", "Tier2");
		InsertDestroyableObject("Fence_Gate_Metal", "Tier3");

		InsertDestroyableObject("Watchtower", "Tier1");
		InsertDestroyableObject("Watchtower_HalfMetal", "Tier2");
		InsertDestroyableObject("Watchtower_Metal", "Tier3");
	}

	bool FindChargeParams(string chargetype, out HDSN_ChargeParams params)
	{
		for(int i = 0; i < configData.Charges.Count(); i++)
		{
			if(configData.Charges[i].Classname == chargetype)
			{
				params = configData.Charges[i];
				return true;
			}
		}

		return false;
	}

	bool FindTierParams(string tiername, out HDSN_TierParams params)
	{
		for(int i = 0; i < configData.Tiers.Count(); i++)
		{
			if(configData.Tiers[i].Name == tiername)
			{
				params = configData.Tiers[i];
				return true;
			}
		}

		return false;
	}

	bool FindTierNameByObjectName(string objectname, out string tiername)
	{
		tiername = configData.DestroyableObjects.Get(objectname);

		if(tiername)
		{
			return true;
		}

		return false;
	}

	bool FindTierParamsByObjectName(string objectname, out HDSN_TierParams params)
	{
		string tierName;

		if(FindTierNameByObjectName(objectname, tierName))
		{
			if(FindTierParams(tierName, params))
			{
				return true;
			}
		}

		return false;
	}

	void InsertCharge(string classname, int damage)
	{
		HDSN_ChargeParams newParams = new HDSN_ChargeParams;
		newParams.Classname = classname;
		newParams.DamageToObjects = damage;
		configData.Charges.Insert(newParams);
	}

	void InsertTier(string tiername, int health = 1)
	{
		HDSN_TierParams newParams = new HDSN_TierParams;
		newParams.Name = tiername;
		newParams.Health = health;
		configData.Tiers.Insert(newParams);
	}

	bool InsertAcceptedChargeType(string tiername, string chargename)
	{
		HDSN_TierParams tier;
		if(FindTierParams(tiername, tier))
		{
			tier.AcceptedChargeTypes.Insert(chargename);
			return true;
		}

		return false;
	}

	void InsertDestroyableObject(string objectname, string tiername = "default")
	{
		configData.DestroyableObjects.Insert(objectname, tiername);
	}
}
