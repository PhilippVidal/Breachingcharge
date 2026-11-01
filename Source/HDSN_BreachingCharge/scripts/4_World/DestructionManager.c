class HDSN_DestructionManager
{
	//protected static ref HDSN_DestructionManager instance;
	ref array<string> FenceConstructionList = {
		"wall_gate",
		"wall_base_down", "wall_base_up",
		"wall_platform", "wall_platform_stairs_left", "wall_platform_stairs_right",
		"wall_wood_down", "wall_wood_up",
		"wall_metal_down", "wall_metal_up"
	};

	ref array<string> WatchtowerConstructionList = {
		"base_down", "base_up",
		"wood_down", "wood_up",
		"metal_down", "metal_up"
	};

	protected static ref HDSN_DestructionManager instance;

	static HDSN_DestructionManager GetInstance()
	{
		if(instance)
		{
			return instance;
		}

		CreateInstance();
		return instance;
	}

	static void CreateInstance()
	{
		ClearInstance();
		instance = new HDSN_DestructionManager;
	}

	static void ClearInstance()
	{
		delete instance;
	}


	HDSN_BreachingChargeConfigManager GetConfigManagerInstance()
	{
		return HDSN_BreachingChargeConfigManager.GetInstance();
	}

	HDSN_BreachingChargeConfigData GetConfig()
	{
		return GetConfigManagerInstance().GetConfigData();
	}


	//Override this if you want to disable raiding under certain conditions
	//If return value is false charges can not be placed on any objects
	bool IsRaidingAllowed()
	{
		return true;
	}

	//Checks if a specific charge is allowed to be placed
	bool IsChargeAllowed(Object target, Object charge, int contactComponent = 0, int level = 0, int wall = 0)
	{
		string type = "";

		if(target.GetType() == "Fence")
		{
			//Prevents placement on barbed  wire
			if(contactComponent == 16 || contactComponent == 21)
			{
				return false;
			}
		}

		if(target.GetType() == "Watchtower")
		{
			if(contactComponent == 17 || contactComponent == 63)
			{
				return false;
			}
		}

		type = GetWallType(target, level, wall);
		//Print("[Breaching Charge Debug]	Json Wall Type => " + type);
		HDSN_TierParams tierParams;
		if(GetConfigManagerInstance().FindTierParamsByObjectName(type, tierParams))
		{
			if(tierParams.IsChargeTypeAllowed(charge.GetType()))
			{
				return true;
			}
		}

		return false;
	}

	int HandleHealth(Object target, HDSN_BreachingChargeBase charge)
	{
		if(target.IsInherited(ItemBase))
		{
			return HandleHealthItemBase(target, charge);
		}
		else
		{
			return HandleHealthNonItemBase(target, charge);
		}
	}

	int HandleHealthItemBase(Object target, HDSN_BreachingChargeBase charge)
	{
		int objHealth;
		int level = -1;
		int wall = -1;

		ItemBase obj = ItemBase.Cast(target);
		Watchtower watchTower;

		if(target.GetType() == "Watchtower")
		{
			level = GetWatchtowerFloorIndex(target, charge.GetPosition());
			wall = GetWatchtowerWallIndex(target, charge.GetPosition(), level);
		}


		if(wall == 0)
		{
			return 0;
		}

		objHealth = GetTargetHealth(target, level, wall);

		//If current health is less than zero (health is -1 by default) set the max health
		if(objHealth <= 0)
		{
			int objHealthMax = GetMaxHealthOfTarget(target, level, wall);
			objHealth = objHealthMax;
		}
		objHealth -= charge.GetChargeDamage();
		SetTargetHealth(target, objHealth, level, wall);

		return objHealth;
	}

	int HandleHealthNonItemBase(Object target, HDSN_BreachingChargeBase charge)
	{
		return -1;
	}

	int GetMaxHealthOfTarget(Object target, int level = 0, int wall = 0)
	{
		int objHealthMax = -1;
		string wallType = GetWallType(target, level, wall);

		HDSN_TierParams params;
		if(GetConfigManagerInstance().FindTierParamsByObjectName(wallType, params))
		{
			objHealthMax = params.GetObjectHealth();
		}

		return objHealthMax;
	}

	void SetTargetHealth(Object target, int value, int level = 0, int wall = 0)
	{
		if(target.IsInherited(ItemBase))
		{
			SetTargetHealthItemBase(target, value, level, wall);
		}
		else
		{
			SetTargetHealthNonItemBase(target, value, level, wall);
		}
	}

	void SetTargetHealthItemBase(Object target, int value, int level = 0, int wall = 0)
	{
		if(target.IsInherited(ItemBase))
		{
			if(target.GetType() == "Watchtower")
			{
				SetHealthHDSNWatchtower(Watchtower.Cast(target), value, level, wall);
			}
			else
			{
				SetHealthHDSNItemBase(ItemBase.Cast(target), value);
			}
		}
	}

	void SetTargetHealthNonItemBase(Object target, int value, int level = 0, int wall = 0)
	{
		//Implement your code here if you want custom functionality for this
	}

	int GetTargetHealth(Object target, int level = 0, int wall = 0)
	{
		if(target.IsInherited(ItemBase))
		{
			return GetTargetHealthItemBase(target, level, wall);
		}

		return GetTargetHealthNonItemBase();
	}

	int GetTargetHealthItemBase(Object target, int level = 0, int wall = 0)
	{
		int objHealth = -1;

		if(target.IsInherited(ItemBase))
		{
			if(target.GetType() == "Watchtower")
			{
				objHealth = GetHealthHDSNWatchtower(Watchtower.Cast(target), level, wall);
			}
			else
			{
				objHealth = GetHealthHDSNItemBase(ItemBase.Cast(target));
			}
		}

		return objHealth;
	}

	int GetHealthHDSNItemBase(ItemBase target)
	{
		return target.GetHealthHDSN();
	}

	int GetHealthHDSNWatchtower(Watchtower target, int level, int wall)
	{
		return target.GetWatchtowerWallHealthHDSN(level, wall);
	}

	void SetHealthHDSNItemBase(ItemBase target, int value)
	{
		target.SetHealthHDSN(value);
	}

	void SetHealthHDSNWatchtower(Watchtower target, int value, int level, int wall)
	{
		target.SetWatchtowerWallHealthHDSN(value, level, wall);
	}

	int GetTargetHealthNonItemBase()
	{
		//Implement your code here if you want custom functionality for this
		return -1;
	}

	void DestroyTarget(Object target, PlayerBase placingPlayer, HDSN_BreachingChargeBase charge)
	{
		if(GetGame().IsServer())
		{
			charge.OnTargetBeingDestroyed();
		}

		//if true just delete the object and return, can be useful for custom modded objects
		if(charge.GetDeleteObjectsDirectly())
		{
			GetGame().ObjectDelete(target);
			return;
		}

		if(charge.GetOnlyDestroyLocks())
		{
			DestroyLocks(target, placingPlayer);
			return;
		}

		if(target.IsInherited(ItemBase))
		{
			if(target.GetType() == "Fence" || target.GetType() == "Watchtower")
			{
				HandleDestructionFenceWatchtower(target, placingPlayer, charge);
			}
			else
			{
				//If destruction of the target can't be handeled at all, just delete it
				if(!HandleDestructionModdedObjects(target, placingPlayer, charge))
				{
					GetGame().ObjectDelete(target);
				}
			}
		}
		else
		{
			HandleDestructionNonItemBase(target, charge);
		}
	}

	void HandleDestructionFenceWatchtower(Object target, PlayerBase placingPlayer, HDSN_BreachingChargeBase charge)
	{
		string targetType = target.GetType();
		int levelIndex = 0;
		int wallIndex = 0;

		if(targetType == "Watchtower")
		{
			levelIndex = GetWatchtowerFloorIndex(target, charge.GetPosition());
			wallIndex = GetWatchtowerWallIndex(target, charge.GetPosition(), levelIndex);
		}

		if(IsWallBuild(target, levelIndex, wallIndex))
		{
			array<string> listOfConstructionParts = new array<string>;

			if(targetType == "Fence")
			{
				DestroyConstructedFenceParts(target, placingPlayer, charge);
			}
			else if(targetType == "Watchtower")
			{
				DestroyConstructedWatchtowerWallParts(target, placingPlayer, levelIndex, wallIndex);
			}
		}
		else
		{
			if(targetType == "Fence")
			{
				GetGame().ObjectDelete(target);
			}
			else if(targetType == "Watchtower")
			{
				if(!IsWatchtowerWallBuildOnLevel(target, levelIndex))
				{
					if(levelIndex > 1)
					{
						DestroyWatchtowerFloorAndAbove(target, placingPlayer, levelIndex);
					}
					else
					{
						GetGame().ObjectDelete(target);
					}
				}
			}
		}
	}

	//You can add your own logic for custom objects here if needed
	bool HandleDestructionInheritedFence(Object target, EntityAI placingPlayer, HDSN_BreachingChargeBase charge)
	{
		if(target.IsInherited(Fence))
		{
			if(charge.GetDestroyLocksFirst() && IsTargetLocked(target))
			{
				DestroyLocks(target, placingPlayer);
			}
			else
			{
				DestroyLocks(target, placingPlayer);
				DestroyAttachments(target);
				if(!TryToPartiallyDestructObject(target, Man.Cast(placingPlayer), charge))
				{
					GetGame().ObjectDelete(target);
				}
			}

			return true;
		}

		return false;
	}

	//You can add your own logic for custom objects here if needed
	bool HandleDestructionModdedObjects(Object target, EntityAI placingPlayer, HDSN_BreachingChargeBase charge)
	{
		if(HandleDestructionInheritedFence(target, placingPlayer, charge))
			return true;

		if(HandleDestructionExpansion(target, placingPlayer, charge))
			return true;

		//MuchStuffPack container compatibility
		if(target.IsKindOf("Msp_Openable_Base") || target.IsKindOf("Msp_Openable_Placeable_Base")  || target.IsKindOf("Msp_Greenhouse_Base"))
		{
			DestroyLocks(target, placingPlayer);
			return true;
		}

		return false;
	}

	bool HandleDestructionNonItemBase(Object target, HDSN_BreachingChargeBase charge)
	{
		GetGame().ObjectDelete(target);
		return true;
	}

	//Handles the destruction of objects from the expansion mod
	//Can't be implemented here because #ifdef depends on load-order of mods
	//and it can't be guaranteed that expansion gets loaded before this mod without setting it as a dependency
	//=> this function gets overridden in the compatibility mod and commented-out code gets implemented
	bool HandleDestructionExpansion(Object target, EntityAI placingPlayer, ItemBase charge)
	{
		return false;
	}

	//Destroys all attachments on a target (e.g. camonets, barbed wire, etc.)
	//Right now only setup for vanilla dayz's fences and watchtowers
	void DestroyAttachments(Object target, int level = 0, int wall = 0)
	{
		CamoNet camoNet;
		BarbedWire barbedWire1;
		BarbedWire barbedWire2;

		if(target.GetType() == "Fence")
		{
			Fence fence = Fence.Cast(target);
			camoNet = fence.GetCamoNet();
			barbedWire1 = fence.GetBarbedWire1();
			barbedWire2 = fence.GetBarbedWire2();

			if(camoNet)
			{
				GetGame().ObjectDelete(camoNet);
			}

			if(barbedWire1)
			{
				barbedWire1.SetMountedState(false);
				GetGame().ObjectDelete(barbedWire1);
			}

			if(barbedWire2)
			{
				barbedWire2.SetMountedState(false);
				GetGame().ObjectDelete(barbedWire2);
			}
		}
		else if(target.GetType() == "Watchtower")
		{
			Watchtower watchtower = Watchtower.Cast(target);
			camoNet = CamoNet.Cast(watchtower.FindAttachmentBySlotName("Level_" + level + "_Wall_" + wall + "_Camonet"));
			barbedWire1 = BarbedWire.Cast(watchtower.FindAttachmentBySlotName("level_" + level + "_wall_" + wall + "_barbedwire_1"));
			barbedWire2 = BarbedWire.Cast(watchtower.FindAttachmentBySlotName("level_" + level + "_wall_" + wall + "_barbedwire_2"));

			if(camoNet)
			{
				camoNet.Delete();
			}

			if(barbedWire1)
			{
				barbedWire1.SetMountedState(false);
				GetGame().ObjectDelete(barbedWire1);
			}

			if(barbedWire2)
			{
				barbedWire2.SetMountedState(false);
				GetGame().ObjectDelete(barbedWire2);
			}
		}
	}

	//Destroys Locks on target
	bool DestroyLocks(Object target, EntityAI placingPlayer)
	{
		bool handled = false;
		if(target.IsInherited(Fence))
		{
			DestroyLocksInheritedFence(target, placingPlayer);
			handled = true;
		}

		if(target.IsInherited(ItemBase))
		{
			DestroyCodelockOnTarget(target, placingPlayer);
			handled = true;
		}

		//Functions for different mods
		if(DestroyLocksExpansion(target, placingPlayer))
		{
			handled = true;
		}

		return handled;
	}

	bool DestroyLocksInheritedFence(Object target, EntityAI placingPlayer)
	{
		Fence fence = Fence.Cast(target);
		CombinationLock combinationLock = fence.GetCombinationLock();

		if(combinationLock)
		{
			//Without unlocking the lock first the object becomes broken and can not be locked again
			combinationLock.UnlockServer(placingPlayer, EntityAI.Cast(target));
			combinationLock.DestroyLock();
		}

		return true;
	}

	bool DestroyLocksExpansion(Object target, EntityAI placingPlayer)
	{
		return false;
	}

	bool DestroyCodelockOnTarget(Object target, EntityAI placingPlayer)
	{
		return false;
	}

	//Destroys all Fence parts that are constructed (i.e. everything but the frame/base)
	void DestroyConstructedFenceParts(Object target, Man placingPlayer, HDSN_BreachingChargeBase charge)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction constr = baseBuildingBase.GetConstruction();

			if(charge.GetDestroyLocksFirst() && IsTargetLocked(target))
			{
				DestroyLocks(target, placingPlayer);
			}
			else
			{
				DestroyLocks(target, placingPlayer);

				DestroyAttachments(target);

				array<string> listOfConstructionParts = FenceConstructionList;

				//check if the fence is open and can be closed, then close it
				Fence fence = Fence.Cast(baseBuildingBase);
				if(fence.CanCloseFence())
				{
					fence.CloseFence();
				}

				for(int i = 0; i < listOfConstructionParts.Count(); i++)
				{
					if(constr.IsPartConstructed(listOfConstructionParts[i]))
					{
						//constr.DestroyConnectedParts(listOfConstructionParts[i]);
						constr.DestroyPartServer(placingPlayer, listOfConstructionParts[i], AT_DESTROY_PART);
					}
				}
			}
		}
	}

	bool TryToPartiallyDestructObject(Object target, Man placingPlayer, HDSN_BreachingChargeBase charge)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(!baseBuildingBase) return false;

		Construction constr = baseBuildingBase.GetConstruction();

		if(!constr) return false;



		//Print("[Breachingcharge Debug] Charge position => " + charge.GetPosition());
		//Print("[Breachingcharge Debug] Target position => " + target.GetPosition());



		//check if the fence is open and can be closed, then close it
		Fence fence = Fence.Cast(baseBuildingBase);
		if(fence.CanCloseFence())
		{
			//Print("[Breachingcharge Debug] closing fence!");
			fence.CloseFence();
		}



		string type = target.GetType();
		if(	type == "BBP_BDoor" || type == "BBP_SDoor" || type == "BBP_Roof_Hatch" || type == "BBP_Floor_Hatch"/*|| type == "BBP_BWindow" || type == "BBP_SWindow"*/ )
		{
			bool partialDestructionHandledBBPNew = false;

			if(constr.IsPartConstructed("t3_door"))
			{
				constr.DestroyPartServer(placingPlayer, "t3_door", AT_DESTROY_PART);
				partialDestructionHandledBBPNew = true;
			}

			if(constr.IsPartConstructed("t2_door"))
			{
				constr.DestroyPartServer(placingPlayer, "t2_door", AT_DESTROY_PART);
				partialDestructionHandledBBPNew = true;
			}

			if(constr.IsPartConstructed("t1_door"))
			{
				constr.DestroyPartServer(placingPlayer, "t1_door", AT_DESTROY_PART);
				partialDestructionHandledBBPNew = true;
			}

			return partialDestructionHandledBBPNew;
		}

		bool partialDestructionHandled = true;
		string objectType = GetWallType(target);
		switch(objectType)
		{

			case "BBP_T1_Floor_Hatch":
			constr.DestroyPartServer(placingPlayer, "t1_hatch", AT_DESTROY_PART);
			break;

			case "BBP_T3_Floor_Hatch":
			constr.DestroyPartServer(placingPlayer, "t3_hatch", AT_DESTROY_PART);
			break;

			case "BBP_T1_Roof_Hatch":
			constr.DestroyPartServer(placingPlayer, "t1_hatch", AT_DESTROY_PART);
			break;

			case "BBP_T2_Roof_Hatch":
			constr.DestroyPartServer(placingPlayer, "t2_hatch", AT_DESTROY_PART);
			break;

			case "BBP_T3_Roof_Hatch":
			constr.DestroyPartServer(placingPlayer, "t3_hatch", AT_DESTROY_PART);
			break;

			default:
			partialDestructionHandled = false;
			break;
		}


		return partialDestructionHandled;
	}

	//Destroys all Watchtower-wall parts that are constructed (i.e. everything but the frame/base)
	void DestroyConstructedWatchtowerWallParts(Object target, Man placingPlayer, int level, int wall)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction constr = baseBuildingBase.GetConstruction();

			DestroyAttachments(target, level, wall);

			array<string> listOfConstructionParts = WatchtowerConstructionList;
			string prefixString = "level_" + level + "_wall_" + wall + "_";

			for(int j = 0; j < listOfConstructionParts.Count(); j++)
			{
				string partName = prefixString + listOfConstructionParts[j];
				if(constr.IsPartConstructed(partName))
				{
					//constr.DestroyConnectedParts(partName);
					constr.DestroyPartServer(placingPlayer, partName, AT_DESTROY_PART);
				}
			}
		}
	}

	//Destroys all watchtower floors from top to bottom (stopping at a given level)
	void DestroyWatchtowerFloorAndAbove(Object target, Man placingPlayer, int level)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction construction = baseBuildingBase.GetConstruction();

			for(int i = level; i <= 3; i++)
			{
				for(int j = 1; j <= 3; j++)
				{
					DestroyAttachments(target, i, j);
				}
			}

			for(int n = 3; n >= level; n--)
			{
				for(int k = 1; k <= 3; k++)
				{
					DestroyConstructedWatchtowerWallParts(target, placingPlayer, n, k);
				}

				if(construction.IsPartConstructed("level_" + n + "_roof"))
				{
					construction.DestroyPartServer(placingPlayer, "level_" + n + "_roof", AT_DESTROY_PART);
				}

				if(construction.IsPartConstructed("level_" + n + "_base"))
				{
					construction.DestroyPartServer(placingPlayer, "level_" + n + "_base", AT_DESTROY_PART);
				}

				if(n <= 2)
				{
					if(construction.IsPartConstructed("level_" + n + "_stairs"))
					{
						construction.DestroyPartServer(placingPlayer, "level_" + n + "_stairs", AT_DESTROY_PART);
					}
				}
			}
		}
	}

	//Checks if any wall is build on a given level of a watchtower
	bool IsWatchtowerWallBuildOnLevel(Object target, int level)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction construction = baseBuildingBase.GetConstruction();

			for(int i = 1; i <= 3; i++)
			{
				if(construction.IsPartConstructed("level_" + level + "_wall_" + i + "_wood_down") || construction.IsPartConstructed("level_" + level + "_wall_" + i + "_wood_up") || construction.IsPartConstructed("level_" + level + "_wall_" + i + "_metal_down") || construction.IsPartConstructed("level_" + level + "_wall_" + i + "_metal_up"))
				{
					return true;
				}
			}
		}

		return false;
	}

	//Checks if a base is build on a given level of a watchtower
	bool IsWatchtowerBaseBuildOnLevel(Object target, int level)
	{
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction construction = BaseBuildingBase.Cast(target).GetConstruction();

			if(construction.IsPartConstructed("level_" + level + "_base"))
			{
				return true;
			}
		}

		return false;
	}

	//Checks if a wall is build
	bool IsWallBuild(Object target, int level = 0, int wall = 0)
	{
		Construction construction = BaseBuildingBase.Cast(target).GetConstruction();

		if(target.GetType() == "Fence")
		{
			if(construction.IsPartConstructed("wall_base_down") || construction.IsPartConstructed("wall_base_up"))
			{
				return true;
			}
		}
		else if(target.GetType() == "Watchtower")
		{
			if(construction.IsPartConstructed("level_" + level + "_wall_" + wall + "_base_down") || construction.IsPartConstructed("level_" + level + "_wall_" + wall + "_base_up"))
			{
				return true;
			}
		}

		return false;
	}

	//Checks if the charge's target is locked or not
	bool IsTargetLocked(Object target)
	{
		if(IsTargetLockedInheritedFence(target))
		{
			return true;
		}

		if(IsTargetLockedExpansion(target))
		{
			return true;
		}

		return false;
	}


	//Checks if a DayZ-Fence (or inherited object) is locked
	bool IsTargetLockedInheritedFence(Object target)
	{
		if(target.IsInherited(Fence))
		{
			Fence fence = Fence.Cast(target);

			if(fence)
			{
				if(fence.IsLocked())
				{
					return true;
				}

				if(IsTargetLockedModdedLock(target))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsTargetLockedModdedLock(Object target)
	{
		if(IsTargetCodeLocked(target))
		{
			return true;
		}

		return false;
	}

	//Checks if a target is locked with a codelock from the codelock mod
	//implementation in the codelock compatibility mod for now since ifdef stuff doesn't work properly for this
	bool IsTargetCodeLocked(Object target)
	{
		return false;
	}

	//Checks if a Expansion-Object is locked
	//implementation in the codelock compatibility mod for now since ifdef stuff doesn't work properly for this
	bool IsTargetLockedExpansion(Object target)
	{
		return false;
	}

	//Returns the index of the floor the charge is being placed on depending on the charge's position
	int GetWatchtowerFloorIndex(Object target, vector position)
	{
		if(target.GetType() == "Watchtower")
		{
			Watchtower watchTower = Watchtower.Cast(target);

			float chargeHeight = watchTower.WorldToModel(position)[1];
			float floorHeight2 = 2.74971;
			float floorHeight3 = 5.50927;

			if(chargeHeight < floorHeight2)
			{
				return 1;
			}
			else if(chargeHeight < floorHeight3)
			{
				return 2;
			}
			else
			{
				return 3;
			}
		}

		return -1;
	}

	//Returns the index of the wall the charge is being placed on, depending on the position of the charge and the level of the watchtower
	//return values: -1 = not in any placement-zone, 0 = in placement-zone but wall not build, 1-3 = index of the wall
	int GetWatchtowerWallIndex(Object target, vector position, int level)
	{
		if(target.GetType() == "Watchtower")
		{
			Watchtower watchTower = Watchtower.Cast(target);

			vector chargePosLocal = watchTower.WorldToModel(position);
			//Height of the charge is not of interest
			chargePosLocal = Vector(chargePosLocal[0], 0.0, chargePosLocal[2]);

			vector wallZoneCenter1 = Vector(-2.08, 0.0, 0.0);
			vector wallZoneCenter2 = Vector(0.0, 0.0, 2.08);
			vector wallZoneCenter3 = Vector(2.08, 0.0, 0.0);

			//Depth and Width of the wall, only roughly accurate because of charge-placement-offset that is needed for modded objects
			float watchtowerWallDepth = 0.5;
			float watchtowerWallWidth = 4;

			//Rest of the function = Evaluation of which wall the charge is being placed on, depending on which rectangle the charge is colliding with
			bool isWall1 = HDSN_Functions.IsInRectangle2D(wallZoneCenter1, watchtowerWallDepth, watchtowerWallWidth, chargePosLocal);
			bool isWall2 = HDSN_Functions.IsInRectangle2D(wallZoneCenter2, watchtowerWallWidth, watchtowerWallDepth, chargePosLocal);
			bool isWall3 = HDSN_Functions.IsInRectangle2D(wallZoneCenter3, watchtowerWallDepth, watchtowerWallWidth, chargePosLocal);

			float distanceToWall1, distanceToWall2, distanceToWall3;
			bool isWall1Constructed, isWall2Constructed, isWall3Constructed;

			if(isWall1 && !isWall2 && !isWall3)
			{
				isWall1Constructed = IsWallBuild(target, level, 1);

				if(isWall1Constructed)
				{
					return 1;
				}

				return 0;
			}
			if(isWall2 && !isWall1 && !isWall3)
			{
				isWall2Constructed = IsWallBuild(target, level, 2);

				if(isWall2Constructed)
				{
					return 2;
				}

				return 0;
			}
			if(isWall3 && !isWall1 && !isWall2)
			{
				isWall3Constructed = IsWallBuild(target, level, 3);

				if(isWall3Constructed)
				{
					return 3;
				}

				return 0;
			}

			if(isWall1 && isWall2)
			{
				distanceToWall1 = vector.DistanceSq(chargePosLocal, wallZoneCenter1);
				distanceToWall2 = vector.DistanceSq(chargePosLocal, wallZoneCenter2);


				isWall1Constructed = IsWallBuild(target, level, 1);
				isWall2Constructed = IsWallBuild(target, level, 2);

				if(distanceToWall1 < distanceToWall2)
				{
					if(isWall1Constructed)
					{
						return 1;
					}

					if(isWall2Constructed)
					{
						return 2;
					}
				}
				else
				{
					if(isWall2Constructed)
					{
						return 2;
					}

					if(isWall1Constructed)
					{
						return 1;
					}
				}
			}

			if(isWall2 && isWall3)
			{
				distanceToWall2 = vector.DistanceSq(chargePosLocal, wallZoneCenter2);
				distanceToWall3 = vector.DistanceSq(chargePosLocal, wallZoneCenter3);

				isWall2Constructed = IsWallBuild(target, level, 2);
				isWall3Constructed = IsWallBuild(target, level, 3);

				if(distanceToWall3 < distanceToWall2)
				{
					if(isWall3Constructed)
					{
						return 3;
					}

					if(isWall2Constructed)
					{
						return 2;
					}
				}
				else
				{
					if(isWall2Constructed)
					{
						return 2;
					}

					if(isWall3Constructed)
					{
						return 3;
					}
				}
			}

			if(isWall1 || isWall2 || isWall3)
			{
				return 0;
			}
		}

		return -1;
	}

	bool GenerateIdentifierType(out string type, Construction constr, string partName, string str, bool replace = false)
	{
		if(!constr.IsPartConstructed(partName))
		{
			return false;
		}

		if(replace)
		{
			type = str;
			return true;
		}

		type += str;
		return true;
	}

	string GenerateCustomBaseBuildingBaseObjectType(Object target, string type, Construction constr, int level = 0, int wall = 0)
	{
		return type;
	}

	string GenerateCustomObjectType(Object target, string type)
	{
		return type;
	}

	//Returns the type an object has depending on which parts are constructed
	//Also returns "adjusted" type-names in some cases for easier readability in the json file (e.g. "ChopperWall3" => "SimpleBaseWall")
	//If no adjustments are made, function will just return the type (same as GetType())
	string GetWallType(Object target, int level = 0, int wall = 0)
	{
		string type = target.GetType();
		BaseBuildingBase baseBuildingBase = BaseBuildingBase.Cast(target);

		if(baseBuildingBase)
		{
			Construction wallConstruction = baseBuildingBase.GetConstruction();

			switch(type)
			{
				case "Fence":
					type = GenerateFenceType(type, wallConstruction);
				break;
				case "Watchtower":
					type = GenerateWatchtowerType(type, wallConstruction, level, wall);
				break;




				case "ExpansionWall6x3":
					type = GenerateExpansionWallType(type, wallConstruction);
				break;




				case "BBP_Bwall":
				case "BBP_Swall":
				case "BBP_BDoor":
				case "BBP_SDoor":
				case "BBP_Bfloor":
				case "BBP_Sfloor":
				case "BBP_Roof_Hatch":
				case "BBP_Bfoundation":
				case "BBP_Sfoundation":
				case "BBP_LGate":
				case "BBP_RGate":
				case "BBP_Half_Bwall":
				case "BBP_Half_Swall":
				case "BBP_Broof":
				case "BBP_Sroof":
				case "BBP_LHanger_Roof":
				case "BBP_RHanger_Roof":
				case "BBP_Slope_Roof":
				case "BBP_Left_Slope_Wall":
				case "BBP_Right_Slope_Wall":
				case "BBP_Triangle_Bfloor":
				case "BBP_Triangle_Broof":
				case "BBP_BWindow":
				case "BBP_SWindow":
				case "BBP_SGate":
				case "BBP_Floor_Hatch":
				case "BBP_Mesh_Fence":
				case "BBP_Triangle_Foundation":
					type = GenerateBBPType_New(type, wallConstruction);
				break;
				case "BBP_Bpillar":
				case "BBP_Spillar":
					type = GenerateBBPPillarType_New(type, wallConstruction);
				break;
				case "BBP_Mesh_Gate":
					type = GenerateBBPMeshType_New(type, wallConstruction);
				break;
				case "BBP_DGate":
					type = GenerateBBPDGateType_New(type, wallConstruction);
				break;





				case "BBP_T1_Wall":
					type = GenerateBBPT1WallType(type, wallConstruction);
				break;
				case "BBP_T2_Wall":
					type = GenerateBBPT2WallType(type, wallConstruction);
				break;
				case "BBP_T3_Wall":
					type = GenerateBBPT3WallType(type, wallConstruction);
				break;
				case "BBP_T1_Wall_Half":
					type = GenerateBBPT1WallHalfType(type, wallConstruction);
				break;
				case "BBP_T2_Wall_Half":
					type = GenerateBBPT2WallHalfType(type, wallConstruction);
				break;
				case "BBP_T3_Wall_Half":
					type = GenerateBBPT3WallHalfType(type, wallConstruction);
				break;
				case "BBP_T1_Window":
					type = GenerateBBPT1WindowType(type, wallConstruction);
				break;
				case "BBP_T2_Window":
					type = GenerateBBPT2WindowType(type, wallConstruction);
				break;
				case "BBP_T3_Window":
					type = GenerateBBPT3WindowType(type, wallConstruction);
				break;
				case "BBP_T1_Door":
					type = GenerateBBPT1DoorType(type, wallConstruction);
				break;
				case "BBP_T2_Door":
					type = GenerateBBPT2DoorType(type, wallConstruction);
				break;
				case "BBP_T3_Door":
					type = GenerateBBPT3DoorType(type, wallConstruction);
				break;
				case "BBP_T1_Roof":
					type = GenerateBBPT1RoofType(type, wallConstruction);
				break;
				case "BBP_T2_Roof":
					type = GenerateBBPT2RoofType(type, wallConstruction);
				break;
				case "BBP_T3_Roof":
					type = GenerateBBPT3RoofType(type, wallConstruction);
				break;
				case "BBP_T1_Floor":
					type = GenerateBBPT1FloorType(type, wallConstruction);
				break;
				case "BBP_T2_Floor":
					type = GenerateBBPT2FloorType(type, wallConstruction);
				break;
				case "BBP_T3_Floor":
					type = GenerateBBPT3FloorType(type, wallConstruction);
				break;
				case "BBP_T1_Stair":
					type = GenerateBBPT1StairType(type, wallConstruction);
				break;
				case "BBP_T2_Stair":
					type = GenerateBBPT2StairType(type, wallConstruction);
				break;
				case "BBP_T3_Stair":
					type = GenerateBBPT3StairType(type, wallConstruction);
				break;
				case "BBP_T1_Floor_Hatch":
					type = GenerateBBPT1FloorHatchType(type, wallConstruction);
				break;
				case "BBP_T3_Floor_Hatch":
					type = GenerateBBPT3FloorHatchType(type, wallConstruction);
				break;
				case "BBP_T1_Roof_Hatch":
					type = GenerateBBPT1RoofHatchType(type, wallConstruction);
				break;
				case "BBP_T2_Roof_Hatch":
					type = GenerateBBPT2RoofHatchType(type, wallConstruction);
				break;
				case "BBP_T3_Roof_Hatch":
					type = GenerateBBPT3RoofHatchType(type, wallConstruction);
				break;
				case "BBP_T1_Ramp":
					type = GenerateBBPT1RampType(type, wallConstruction);
				break;
				case "BBP_T1_Ramp_Wide":
					type = GenerateBBPT1RampWideType(type, wallConstruction);
				break;
				case "BBP_T1_Gate":
					type = GenerateBBPT1GateType(type, wallConstruction);
				break;
				case "BBP_T2_Gate":
					type = GenerateBBPT2GateType(type, wallConstruction);
				break;
				case "BBP_T3_Gate":
					type = GenerateBBPT3GateType(type, wallConstruction);
				break;
				case "BBP_T3_Door_Half":
					type = GenerateBBPT3DoorHalfType(type, wallConstruction);
				break;
				case "BBP_T1_Foundation":
					type = GenerateBBPT1FoundationType(type, wallConstruction);
				break;
				case "BBP_T2_Foundation":
					type = GenerateBBPT2FoundationType(type, wallConstruction);
				break;
				case "BBP_T3_Foundation":
					type = GenerateBBPT3FoundationType(type, wallConstruction);
				break;
				case "BBP_Pillar":
					type = GenerateBBPPillarType(type, wallConstruction);
				break;




				case "ChopperWall3":
					type = GenerateSimpleBaseWallType(type, wallConstruction);
				break;
				case "ChopperDoor2":
					type = GenerateSimpleBaseDoorType(type, wallConstruction);
				break;
				case "ChopperGate":
					type = GenerateSimpleBaseGateType(type, wallConstruction);
				break;
				case "ChopperWallHalf":
					type = GenerateSimpleBaseWallHalfType(type, wallConstruction);
				break;
				case "ChopperWindow":
					type = GenerateSimpleBaseWindowType(type, wallConstruction);
				break;
				case "ChopperRoof2":
					type = GenerateSimpleBaseRoofType(type, wallConstruction);
				break;
				case "ChopperFloor4":
					type = GenerateSimpleBaseFloorType(type, wallConstruction);
				break;
				case "ChopperStaircase":
					type = GenerateSimpleBaseStaircaseType(type, wallConstruction);
				break;
				case "ChopperPost2":
					type = GenerateSimpleBasePostType(type, wallConstruction);
				break;
				case "ChopperPostHalf":
					type = GenerateSimpleBasePostHalfType(type, wallConstruction);
				break;




				case "EvenMoreDoor":
					type = GenerateEvenMoreDoorType(type, wallConstruction);
				break;
				case "EvenMoreDoor_Double":
                   type = GenerateEvenMoreDoorDoubleType(type, wallConstruction);
                break;

				default:
					type = GenerateCustomBaseBuildingBaseObjectType(target, type, wallConstruction, level, wall);
				break;
			}
		}
		else
		{
			type = GenerateCustomObjectType(target, type);
		}


		//Print("Type ==> " + type);
		return type;
	}

	string GetBBPWallTier(Construction constr)
	{
		if(constr.IsPartConstructed("t3_wall_concrete_bricks") || constr.IsPartConstructed("t3_wall_concrete_bricks_2"))
			return "_T3";

		if(constr.IsPartConstructed("t2_metal") || constr.IsPartConstructed("t2_metal_2"))
			return "_T2";

		if(constr.IsPartConstructed("t1_wall_logs_horizontal") || constr.IsPartConstructed("t1_wall_planks"))
			return "_T1";

		return "_Frame";
	}

	string GetBBPDoorTier(Construction constr)
	{
		if(constr.IsPartConstructed("t3_door"))
			return "_Door_T3";

		if(constr.IsPartConstructed("t2_door"))
			return "_Door_T2";

		if(constr.IsPartConstructed("t1_door"))
			return "_Door_T1";

		return "";
	}

	string GenerateBBPDGateType_New(string type, Construction constr)
	{

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			string tmp = "_RightDoor_T3";

			if(constr.IsPartConstructed("t3_wall_concrete_bricks_2"))
				tmp = "_LeftDoor_T3" + tmp;

			return type + tmp;
		}


		string leftDoor = "";
		string rightDoor = "";

		if(constr.IsPartConstructed("t2_metal"))
		{
			leftDoor = "_LeftDoor_T2";
		}
		else if(constr.IsPartConstructed("t1_wall_logs_horizontal_2"))
		{
			leftDoor = "_LeftDoor_T1";
		}

		if(constr.IsPartConstructed("t2_metal_2"))
		{
			rightDoor = "_RightDoor_T2";
		}
		else if(constr.IsPartConstructed("t1_wall_logs_horizontal"))
		{
			rightDoor = "_RightDoor_T1";
		}


		return type + leftDoor + rightDoor;
	}

	string GenerateBBPMeshType_New(string type, Construction constr)
	{

		if(constr.IsPartConstructed("t2_metal"))
			type += "_LeftDoor_T2";


		if(constr.IsPartConstructed("t2_door"))
			type += "_RightDoor_T2";


		//if(constr.IsPartConstructed("t2_metal_2"))
		//	type += "_Barbwire";


		return type;
	}

	string GetBBPOtherPartsSuffix(Construction constr)
	{
		string tmpType = "";

		if(constr.IsPartConstructed("hdsometestpartn"))
			tmpType += "_TestWall";

		if(constr.IsPartConstructed("stair"))
			tmpType += "_Stairs";

		if(constr.IsPartConstructed("t1_trapdoor"))
			tmpType += "_TrapDoor_T1";

		return tmpType;
	}

	string GenerateBBPType_New(string type, Construction constr)
	{

		type += GetBBPWallTier(constr);
		type += GetBBPDoorTier(constr);
		type += GetBBPOtherPartsSuffix(constr);

		//if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		//{
		//	type +=  "_T3";
		//}
		//else if(constr.IsPartConstructed("t2_metal"))
		//{
		//	type += "_T2";
		//}
		//else if(constr.IsPartConstructed("t1_wall_logs_horizontal"))
		//{
		//	type += "_T1";
		//}
		//else
		//{
		//	type +="_Frame";
		//}

		//if(constr.IsPartConstructed("HDSometestpartN"))
		//{
		//	type += "Wall_";
		//}
		//
		//if(constr.IsPartConstructed("t1_trapdoor"))
		//{
		//	type += "_TD_T1";
		//}
		//
		//if(constr.IsPartConstructed("t3_door"))
		//{
		//	type += "_Door_T3";
		//}
		//else if(constr.IsPartConstructed("t1_door"))
		//{
		//	type += "_Door_T1";
		//}
		//
		//if(constr.IsPartConstructed("stair"))
		//{
		//	type += "_Stairs";
		//}

		return type;
	}

	string GenerateBBPPillarType_New(string type, Construction constr)
	{

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type += "_T3";
		}
		else
		{
			type += "_T1";
		}

		return type;
	}

	string GenerateFenceType(string type, Construction constr)
	{
		type = "Fence_Frame";

		int metalCount = 0;

		//wallConstruction.IsPartConstructed("wall_base_down") || wallConstruction.IsPartConstructed("wall_base_up");
		if(constr.IsPartConstructed("wall_base_down") || constr.IsPartConstructed("wall_base_up"))
		{
			type = "Fence";
		}

		if(constr.IsPartConstructed("wall_gate") )
		{
			type += "_Gate";
		}

		if(constr.IsPartConstructed("wall_metal_down"))
		{
			metalCount++;
		}

		if(constr.IsPartConstructed("wall_metal_up"))
		{
			metalCount++;
		}

		if(metalCount == 1)
		{
			type += "_HalfMetal";
		}
		else if(metalCount == 2)
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateWatchtowerType(string type, Construction constr, int level, int wall)
	{
		int metalCount = 0;

		if(constr.IsPartConstructed("level_" + level + "_wall_" + wall + "_metal_down"))
		{
			metalCount++;
		}

		if(constr.IsPartConstructed("level_" + level + "_wall_" + wall + "_metal_up"))
		{
			metalCount++;
		}

		if(metalCount == 1)
		{
			type += "_HalfMetal";
		}
		else if(metalCount == 2)
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateExpansionWallType(string type, Construction constr)
	{
		type = "ExpansionWall";

		if(constr.IsPartConstructed("wood_doorfinished"))
		{
			type += "_Door";
		}

		if(constr.IsPartConstructed("wood_gatefinished"))
		{
			type += "_Gate";
		}

		return type;
	}

	string GenerateBBPT1WallType(string type, Construction constr)
	{
		type = "BBP_T1_Wall_Hologram";

		if(constr.IsPartConstructed("t1_frame"))
		{
			type = "BBP_T1_Wall_Frame";
		}

		if(constr.IsPartConstructed("t1_wall_logs_horizontal") || constr.IsPartConstructed("t1_wall_planks_horizontal") || constr.IsPartConstructed("t1_wall_planks_vertical"))
		{
			type = "BBP_T1_Wall";
		}

		return type;
	}

	string GenerateBBPT2WallType(string type, Construction constr)
	{
		type = "BBP_T2_Wall_Hologram";

		if(constr.IsPartConstructed("t2_frame"))
		{
			type = "BBP_T2_Wall_Frame";
		}

		if(constr.IsPartConstructed("t2_wall_metal_panels"))
		{
			type = "BBP_T2_Wall";
		}

		if(constr.IsPartConstructed("t2_upgrade_metal"))
		{
			type += "_Rebar";
		}

		return type;
	}

	string GenerateBBPT3WallType(string type, Construction constr)
	{
		type = "BBP_T3_Wall_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Wall_Frame";
		}

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type = "BBP_T3_Wall";
		}

		if(constr.IsPartConstructed("t3_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateBBPT1WallHalfType(string type, Construction constr)
	{
		type = "BBP_T1_Wall_Half_Hologram";

		if(constr.IsPartConstructed("t1_frame"))
		{
			type = "BBP_T1_Wall_Half_Frame";
		}
		if(constr.IsPartConstructed("t1_wall_logs_horizontal") || constr.IsPartConstructed("t1_wall_planks_horizontal") || constr.IsPartConstructed("t1_wall_planks_vertical"))
		{
			type = "BBP_T1_Wall_Half";
		}

		return type;
	}

	string GenerateBBPT2WallHalfType(string type, Construction constr)
	{
		type = "BBP_T2_Wall_Half_Hologram";

		if(constr.IsPartConstructed("t2_frame"))
		{
			type = "BBP_T2_Wall_Half_Frame";
		}

		if(constr.IsPartConstructed("t2_wall_metal_panels"))
		{
			type = "BBP_T2_Wall_Half";
		}

		return type;
	}

	string GenerateBBPT3WallHalfType(string type, Construction constr)
	{
		type = "BBP_T3_Wall_Half_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Wall_Half_Frame";
		}

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type = "BBP_T3_Wall_Half";
		}

		return type;
	}

	string GenerateBBPT1WindowType(string type, Construction constr)
	{
		type = "BBP_T1_Window_Hologram";

		if(constr.IsPartConstructed("t1_frame"))
		{
			type = "BBP_T1_Window_Frame";
		}

		if(constr.IsPartConstructed("t1_wall_planks_vertical"))
		{
			type = "BBP_T1_Window";
		}

		if(constr.IsPartConstructed("t1_window"))
		{
			type += "_Shutter";
		}

		if(constr.IsPartConstructed("t1_window_upgrade_bars"))
		{
			type += "_Bars";
		}

		return type;
	}

	string GenerateBBPT2WindowType(string type, Construction constr)
	{
		type = "BBP_T2_Window_Hologram";

		if(constr.IsPartConstructed("t2_frame"))
		{
			type = "BBP_T2_Window_Frame";
		}

		if(constr.IsPartConstructed("t2_wall_metal_panels"))
		{
			type = "BBP_T2_Window";
		}

		if(constr.IsPartConstructed("t2_window"))
		{
			type += "_Shutter";
		}

		if(constr.IsPartConstructed("t2_window_upgrade_bars"))
		{
			type += "_Bars";
		}

		return type;
	}

	string GenerateBBPT3WindowType(string type, Construction constr)
	{
		type = "BBP_T3_Window_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Window_Frame";
		}

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type = "BBP_T3_Window";
		}

		if(constr.IsPartConstructed("t3_window"))
		{
			type += "_Shutter";
		}

		if(constr.IsPartConstructed("t3_window_upgrade_bars"))
		{
			type += "_Bars";
		}

		return type;
	}

	string GenerateBBPT1DoorType(string type, Construction constr)
	{
		type = "BBP_T1_Door_Hologram";

		if(constr.IsPartConstructed("t1_frame"))
		{
			type = "BBP_T1_Door_Frame";
		}

		if(constr.IsPartConstructed("t1_wall_planks_vertical") || constr.IsPartConstructed("t1_wall_logs_horizontal"))
		{
			type = "BBP_T1_Door";
		}

		if(constr.IsPartConstructed("t1_door"))
		{
			type += "_Door";
		}

		return type;
	}

	string GenerateBBPT2DoorType(string type, Construction constr)
	{
		type = "BBP_T2_Door_Hologram";

		if(constr.IsPartConstructed("t2_frame"))
		{
			type = "BBP_T2_Door_Frame";
		}

		if(constr.IsPartConstructed("t2_wall_metal_panels"))
		{
			type = "BBP_T2_Door";
		}

		if(constr.IsPartConstructed("t2_door"))
		{
			type += "_Door";
		}

		if(constr.IsPartConstructed("t2_door_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateBBPT3DoorType(string type, Construction constr)
	{
		type = "BBP_T3_Door_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Door_Frame";
		}

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type = "BBP_T3_Door";
		}

		if(constr.IsPartConstructed("t3_door"))
		{
			type += "_Door";
		}

		return type;
	}

	string GenerateBBPT1RoofType(string type, Construction constr)
	{
		type = "BBP_T1_Roof_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Roof";
		}

		return type;
	}

	string GenerateBBPT2RoofType(string type, Construction constr)
	{
		type = "BBP_T2_Roof_Hologram";

		if(constr.IsPartConstructed("t2_roof_floor"))
		{
			type = "BBP_T2_Roof";
		}

		return type;
	}

	string GenerateBBPT3RoofType(string type, Construction constr)
	{
		type = "BBP_T3_Roof_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Roof";
		}

		return type;
	}

	string GenerateBBPT1FloorType(string type, Construction constr)
	{
		type = "BBP_T1_Floor_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Floor";
		}

		return type;
	}

	string GenerateBBPT2FloorType(string type, Construction constr)
	{
		type = "BBP_T2_Floor_Hologram";

		if(constr.IsPartConstructed("t2_roof_floor"))
		{
			type = "BBP_T2_Floor";
		}

		return type;
	}

	string GenerateBBPT3FloorType(string type, Construction constr)
	{
		type = "BBP_T3_Floor_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Floor";
		}

		return type;
	}

	string GenerateBBPT1StairType(string type, Construction constr)
	{
		type = "BBP_T1_Stair_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Stair";
		}

		return type;
	}

	string GenerateBBPT2StairType(string type, Construction constr)
	{
		type = "BBP_T2_Stair_Hologram";

		if(constr.IsPartConstructed("t2_roof_floor"))
		{
			type = "BBP_T2_Stair";
		}

		return type;
	}

	string GenerateBBPT3StairType(string type, Construction constr)
	{
		type = "BBP_T3_Stair_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Stair";
		}

		return type;
	}

	string GenerateBBPT1FloorHatchType(string type, Construction constr)
	{
		type = "BBP_T1_Floor_Hatch_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Floor_Hatch_Frame";
		}

		if(constr.IsPartConstructed("t1_hatch"))
		{
			type = "BBP_T1_Floor_Hatch";
		}

		return type;
	}

	string GenerateBBPT3FloorHatchType(string type, Construction constr)
	{
		type = "BBP_T3_Floor_Hatch_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Floor_Hatch_Frame";
		}

		if(constr.IsPartConstructed("t3_hatch"))
		{
			type = "BBP_T3_Floor_Hatch";
		}

		return type;
	}

	string GenerateBBPT1RoofHatchType(string type, Construction constr)
	{
		type = "BBP_T1_Roof_Hatch_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Roof_Hatch_Frame";
		}

		if(constr.IsPartConstructed("t1_hatch"))
		{
			type = "BBP_T1_Roof_Hatch";
		}

		return type;
	}

	string GenerateBBPT2RoofHatchType(string type, Construction constr)
	{
		type = "BBP_T2_Roof_Hatch_Hologram";

		if(constr.IsPartConstructed("t2_roof_floor"))
		{
			type = "BBP_T2_Roof_Hatch_Frame";
		}

		if(constr.IsPartConstructed("t2_hatch"))
		{
			type = "BBP_T2_Roof_Hatch";
		}

		return type;
	}

	string GenerateBBPT3RoofHatchType(string type, Construction constr)
	{
		type = "BBP_T3_Roof_Hatch_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Roof_Hatch_Frame";
		}

		if(constr.IsPartConstructed("t3_hatch"))
		{
			type = "BBP_T3_Roof_Hatch";
		}

		return type;
	}

	string GenerateBBPT1RampType(string type, Construction constr)
	{
		type = "BBP_T1_Ramp_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Ramp";
		}

		return type;
	}

	string GenerateBBPT1RampWideType(string type, Construction constr)
	{
		type = "BBP_T1_Ramp_Wide_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Ramp_Wide";
		}

		return type;
	}

	string GenerateBBPT1GateType(string type, Construction constr)
	{
		type = "BBP_T1_Gate_Hologram";

		if(constr.IsPartConstructed("t1_frame"))
		{
			type = "BBP_T1_Gate_Frame";
		}

		if(constr.IsPartConstructed("t1_gate"))
		{
			type = "BBP_T1_Gate";
		}

		return type;
	}

	string GenerateBBPT2GateType(string type, Construction constr)
	{
		type = "BBP_T2_Gate_Hologram";

		if(constr.IsPartConstructed("t2_frame"))
		{
			type = "BBP_T2_Gate_Frame";
		}

		if(constr.IsPartConstructed("t2_gate") || constr.IsPartConstructed("t2_gate_2"))
		{
			type = "BBP_T2_Gate";
		}

		return type;
	}

	string GenerateBBPT3GateType(string type, Construction constr)
	{
		type = "BBP_T3_Gate_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Gate_Frame";
		}

		if(constr.IsPartConstructed("t3_gate") || constr.IsPartConstructed("t3_gate_2"))
		{
			type = "BBP_T3_Gate";
		}

		return type;
	}

	string GenerateBBPT3DoorHalfType(string type, Construction constr)
	{
		type = "BBP_T3_Door_Half_Hologram";

		if(constr.IsPartConstructed("t3_frame"))
		{
			type = "BBP_T3_Door_Half_Frame";
		}

		if(constr.IsPartConstructed("t3_wall_concrete_bricks"))
		{
			type = "BBP_T3_Door_Half";
		}

		if(constr.IsPartConstructed("t3_door"))
		{
			type += "_Door";
		}

		return type;
	}

	string GenerateBBPT1FoundationType(string type, Construction constr)
	{
		type = "BBP_T1_Foundation_Hologram";

		if(constr.IsPartConstructed("t1_roof_floor"))
		{
			type = "BBP_T1_Foundation";
		}

		return type;
	}

	string GenerateBBPT2FoundationType(string type, Construction constr)
	{
		type = "BBP_T2_Foundation_Hologram";

		if(constr.IsPartConstructed("t2_roof_floor"))
		{
			type = "BBP_T2_Foundation";
		}

		return type;
	}

	string GenerateBBPT3FoundationType(string type, Construction constr)
	{
		type = "BBP_T3_Foundation_Hologram";

		if(constr.IsPartConstructed("t3_roof_floor"))
		{
			type = "BBP_T3_Foundation";
		}


		return type;
	}

	string GenerateBBPPillarType(string type, Construction constr)
	{
		if(constr.IsPartConstructed("pillar_logs"))
		{
			type = "BBP_Pillar_Wood";
		}

		if(constr.IsPartConstructed("pillar_logs_half"))
		{
			type = "BBP_Pillar_WoodHalf";
		}

		if(constr.IsPartConstructed("pillar_metal"))
		{
			type = "BBP_Pillar_Metal";
		}

		if(constr.IsPartConstructed("pillar_metal_half"))
		{
			type = "BBP_Pillar_MetalHalf";
		}

		if(constr.IsPartConstructed("pillar_concrete"))
		{
			type = "BBP_Pillar_Concrete";
		}

		if(constr.IsPartConstructed("pillar_concrete_half"))
		{
			type = "BBP_Pillar_ConcreteHalf";
		}

		return type;
	}

	string GenerateSimpleBaseWallType(string type, Construction constr)
	{
		type = "SimpleBaseWall_Hologram";

		if(constr.IsPartConstructed("simple_wall"))
		{
			type = "SimpleBaseWall";
		}

		if(constr.IsPartConstructed("simple_wall_upgrade_logs"))
		{
			type += "_Logs";
		}

		if(constr.IsPartConstructed("simple_wall_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateSimpleBaseDoorType(string type, Construction constr)
	{
		type = "SimpleBaseDoor_Hologram";

		if(constr.IsPartConstructed("simple_door_frame"))
		{
			type = "SimpleBaseDoor_Frame";
		}

		if(constr.IsPartConstructed("simple_door"))
		{
			type = "SimpleBaseDoor";
		}

		if(constr.IsPartConstructed("simple_door_upgrade_logs"))
		{
			type += "_Logs";
		}

		if(constr.IsPartConstructed("simple_door_upgrade_metal"))
		{
			type += "_Metal";
		}

		if(constr.IsPartConstructed("simple_door_frame_upgrade_logs"))
		{
			type += "_LogsFrame";
		}

		if(constr.IsPartConstructed("simple_door_frame_upgrade_metal"))
		{
			type += "_MetalFrame";
		}

		return type;
	}

	string GenerateSimpleBaseGateType(string type, Construction constr)
	{
		type = "SimpleBaseGate_Hologram";

		if(constr.IsPartConstructed("simple_gate_frame"))
		{
			type = "SimpleBaseGate_Frame";
		}

		if(constr.IsPartConstructed("simple_gate"))
		{
			type = "SimpleBaseGate";
		}

		if(constr.IsPartConstructed("simple_gate_upgrade_logs"))
		{
			type += "_Logs";
		}

		if(constr.IsPartConstructed("simple_gate_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateSimpleBaseWallHalfType(string type, Construction constr)
	{
		type = "SimpleBaseHalfWall_Hologram";

		if(constr.IsPartConstructed("simple_half_wall"))
		{
			type = "SimpleBaseHalfWall";
		}

		if(constr.IsPartConstructed("simple_half_wall_upgrade_logs"))
		{
			type += "_Logs";
		}

		if(constr.IsPartConstructed("simple_half_wall_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateSimpleBaseWindowType(string type, Construction constr)
	{
		type = "SimpleBaseWindow_Hologram";

		if(constr.IsPartConstructed("simple_window_frame"))
		{
			type = "SimpleBaseWindow_Frame";
		}

		if(constr.IsPartConstructed("simple_window"))
		{
			type = "SimpleBaseWindow";
		}

		if(constr.IsPartConstructed("simple_window_upgrade_metal"))
		{
			type += "_Metal";
		}

		if(constr.IsPartConstructed("simple_window_frame_upgrade_logs"))
		{
			type += "_LogsFrame";
		}

		if(constr.IsPartConstructed("simple_window_frame_upgrade_metal"))
		{
			type += "_MetalFrame";
		}

		if(constr.IsPartConstructed("simple_window_upgrade_bars"))
		{
			type += "_Bars";
		}

		return type;
	}

	string GenerateSimpleBaseRoofType(string type, Construction constr)
	{
		type = "SimpleBaseRoof_Hologram";

		if(constr.IsPartConstructed("simple_roof"))
		{
			type = "SimpleBaseRoof";
		}

		if(constr.IsPartConstructed("simple_roof_upgrade_metal"))
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateSimpleBaseFloorType(string type, Construction constr)
	{
		type = "SimpleBaseFloor_Hologram";

		if(constr.IsPartConstructed("simple_floor"))
		{
			type = "SimpleBaseFloor";
		}
		return type;
	}

	string GenerateSimpleBaseStaircaseType(string type, Construction constr)
	{
		type = "SimpleBaseStaircase_Hologram";

		if(constr.IsPartConstructed("simple_staircase"))
		{
			type = "SimpleBaseStaircase";
		}

		return type;
	}

	string GenerateSimpleBasePostType(string type, Construction constr)
	{
		type = "SimpleBasePost_Hologram";

		if(constr.IsPartConstructed("simple_pillar"))
		{
			type = "SimpleBasePost";
		}

		return type;
	}

	string GenerateSimpleBasePostHalfType(string type, Construction constr)
	{
		type = "SimpleBasePostHalf_Hologram";

		if(constr.IsPartConstructed("simple_half_pillar"))
		{
			type = "SimpleBasePostHalf";
		}

		return type;
	}

	string GenerateEvenMoreDoorType(string type, Construction constr)
	{
		type = "EvenMoreDoor_Frame";

		int metalCount = 0;

		if(constr.IsPartConstructed("wall_base_up"))
		{
			type = "EvenMoreDoor";
		}

		if(constr.IsPartConstructed("wall_metal_up"))
		{
			metalCount++;
		}

		if(constr.IsPartConstructed("wall_gate"))
		{
			type += "_Door";
		}

		if(metalCount > 0)
		{
			type += "_Metal";
		}

		return type;
	}

	string GenerateEvenMoreDoorDoubleType(string type, Construction constr)
	{
		type = "EvenMoreDoor_Double_Frame";

		int metalCount = 0;

		if(constr.IsPartConstructed("wall_base_down") || constr.IsPartConstructed("wall_base_up"))
		{
			type = "EvenMoreDoor_Double";
		}
		if(constr.IsPartConstructed("wall_metal_down"))
		{
			metalCount++;
		}

		if(constr.IsPartConstructed("wall_metal_up"))
		{
			metalCount++;
		}

		if(constr.IsPartConstructed("wall_gate"))
		{
			type += "_Door";
		}

		if(metalCount == 1)
		{
			type += "_HalfMetal";
		}
		else if(metalCount == 2)
		{
			type += "_Metal";
		}

		return type;
	}
}
