class HDSN_PlaceBreachingChargeReceiveData : ActionReciveData
{
	vector position;
	vector orientation;
	Object targetObject;
}

class HDSN_PlaceBreachingChargeActionData : ActionData
{
	vector position;
	vector orientation;
	bool alreadyPlaced;
	Object targetObject;
}

class HDSN_ActionDeployBreachingChargeCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();
		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(m_ActionData.m_MainItem);
		m_ActionData.m_ActionComponent = new CAContinuousTime(charge.GetTimeToPlant());
	}
}

class HDSN_ActionDeployBreachingCharge : ActionContinuousBase
{
	string actionText;
	vector contactPos, contactDir;
	int contactComponent;
	float raycastDistance;
	Hologram chargeHologram;
	Object raycastTarget;


	void HDSN_ActionDeployBreachingCharge()
	{
		m_CallbackClass = HDSN_ActionDeployBreachingChargeCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_1HD;
		//m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override bool ActionConditionContinue(ActionData action_data)
	{
		return true;
	}

	override ActionData CreateActionData()
	{
		return new HDSN_PlaceBreachingChargeActionData;
	}

	override bool SetupAction(PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL)
	{
		if(super.SetupAction(player, target, item, action_data, extra_data))
		{
			HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
			pbcActionData.alreadyPlaced = false;

			if(GetGame().IsClient() || !GetGame().IsMultiplayer())
			{
				Hologram localHologram = player.GetHologramLocal();

				if(!localHologram)
				{
					return false;
				}

				pbcActionData.position = localHologram.GetProjectionPosition();
				pbcActionData.orientation = localHologram.GetProjectionOrientation();

				pbcActionData.m_Player.SetLocalProjectionPosition(pbcActionData.position);
				pbcActionData.m_Player.SetLocalProjectionOrientation(pbcActionData.orientation);
				pbcActionData.targetObject = raycastTarget;
			}

			if(!action_data.m_MainItem)
			{
				return false;
			}

			return true;
		}

		return false;
	}

	override void OnStartClient(ActionData action_data)
	{
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
		if(!pbcActionData)
		{
			return;
		}

		if(GetGame().IsMultiplayer())
		{
			pbcActionData.m_Player.PlacingCompleteLocal();
		}
	}

	override void OnStartServer(ActionData action_data)
	{
		if(GetGame().IsMultiplayer())
		{
			HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
			if(!pbcActionData)
			{
				return;
			}

			pbcActionData.m_Player.SetLocalProjectionPosition(pbcActionData.position);
			pbcActionData.m_Player.SetLocalProjectionOrientation(pbcActionData.orientation);

			if(pbcActionData.m_MainItem)
			{
				pbcActionData.m_Player.PlacingStartServer(pbcActionData.m_MainItem);
				GetGame().AddActionJuncture(action_data.m_Player, pbcActionData.m_MainItem, 10000);
				pbcActionData.m_MainItem.SetIsBeingPlaced(true);
			}
		}
		else
		{
			action_data.m_Player.PlacingStartServer(action_data.m_MainItem);
			action_data.m_Player.GetHologramLocal().SetUpdatePosition(false);
			action_data.m_MainItem.SetIsBeingPlaced(true);
		}
	}

	override void OnFinishProgressClient(ActionData action_data)
	{
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
		if(!pbcActionData)
		{
			return;
		}

		vector position = action_data.m_Player.GetLocalProjectionPosition();
		vector orientation = action_data.m_Player.GetLocalProjectionOrientation();

		pbcActionData.alreadyPlaced = true;

		pbcActionData.m_MainItem.OnPlacementComplete(action_data.m_Player, position, orientation);

		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(pbcActionData.m_MainItem);
		if(charge)
		{
			charge.PlaceBreachingCharge(pbcActionData.m_Player, pbcActionData.targetObject);
		}
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
		if(!pbcActionData || !action_data.m_MainItem)
		{
			return;
		}

		vector position = action_data.m_Player.GetLocalProjectionPosition();
		vector orientation = action_data.m_Player.GetLocalProjectionOrientation();

		EntityAI entityForPlacing = pbcActionData.m_MainItem;
		pbcActionData.m_Player.GetHologramServer().PlaceEntity(entityForPlacing);

		/*
		if(GetGame().IsMultiplayer())
		{
			pbcActionData.m_Player.GetHologramServer().CheckPowerSource();
		}
		*/

		pbcActionData.m_Player.PlacingCompleteServer();
		entityForPlacing.OnPlacementComplete(pbcActionData.m_Player, position, orientation);

		MoveEntityToFinalPosition(action_data, position, orientation);
		GetGame().ClearJuncture(action_data.m_Player, entityForPlacing);
		pbcActionData.m_MainItem.SetIsBeingPlaced(false);
		pbcActionData.alreadyPlaced = true;
		pbcActionData.m_MainItem.SoundSynchRemoteReset();

		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(pbcActionData.m_MainItem);

		if(!charge)
			return;

		charge.PlaceBreachingCharge(pbcActionData.m_Player, pbcActionData.targetObject);
		if(charge.GetAutomaticallyArm())
			charge.ArmBreachingCharge();
	}

	void MoveEntityToFinalPosition(ActionData action_data, vector position, vector orientation)
	{
		vector rotationMatrix[3];
		float direction[4];

		InventoryLocation source = new InventoryLocation;
		InventoryLocation destination = new InventoryLocation;

		Math3D.YawPitchRollMatrix(orientation, rotationMatrix);
		Math3D.MatrixToQuat(rotationMatrix, direction);

		EntityAI entityForPlacing = action_data.m_MainItem;
		if(entityForPlacing.GetInventory().GetCurrentInventoryLocation(source))
		{
			destination.SetGroundEx(entityForPlacing, position, direction);

			if(GetGame().IsMultiplayer())
			{
				action_data.m_Player.ServerTakeToDst(source, destination);
			}
			else
			{
				action_data.m_Player.GetInventory().TakeToDst(InventoryMode.LOCAL, source, destination);
			}
		}
	}

	override void OnEndClient(ActionData action_data)
	{
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
		if(!pbcActionData)
		{
			return;
		}

		if(!pbcActionData.alreadyPlaced)
		{
			pbcActionData.m_Player.PlacingCompleteLocal();
			pbcActionData.m_Player.PredictiveTakeEntityToHands(pbcActionData.m_MainItem);
			pbcActionData.m_Player.LockHandsUntilItemHeld();
		}
	}

	override void OnEndServer(ActionData action_data)
	{
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);
		if(!pbcActionData || !action_data.m_MainItem)
		{
			return;
		}

		if(!pbcActionData.alreadyPlaced)
		{
			GetGame().ClearJuncture(pbcActionData.m_Player, pbcActionData.m_MainItem);
			pbcActionData.m_MainItem.SetIsBeingPlaced(false);

			if(GetGame().IsMultiplayer())
			{
				pbcActionData.m_Player.PlacingCancelServer();
				pbcActionData.m_MainItem.SoundSynchRemoteReset();
			}
			else
			{
				pbcActionData.m_Player.PlacingCancelLocal();
				pbcActionData.m_Player.PlacingCancelServer();

				InventoryLocation source = new InventoryLocation;
				InventoryLocation destination = new InventoryLocation;

				if(pbcActionData.m_MainItem.GetInventory().GetCurrentInventoryLocation(source))
				{
					destination.SetHands(pbcActionData.m_Player, pbcActionData.m_MainItem);
					pbcActionData.m_Player.GetDayZPlayerInventory().RedirectToHandEvent(InventoryMode.LOCAL, source, destination);
				}
			}
		}
		else
		{
			pbcActionData.m_MainItem.SetIsDeploySound(false);
			pbcActionData.m_MainItem.SetIsPlaceSound(false);
			pbcActionData.m_MainItem.SoundSynchRemoteReset();
		}

		GetGame().ClearJuncture(pbcActionData.m_Player, pbcActionData.m_MainItem);
	}

	override void OnExecuteServer(ActionData action_data)
	{
		action_data.m_MainItem.SoundSynchRemote();
	}

	override void WriteToContext(ParamsWriteContext ctx, ActionData action_data)
	{
		super.WriteToContext(ctx, action_data);

		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);

		ctx.Write(pbcActionData.position);
		ctx.Write(pbcActionData.orientation);
		ctx.Write(pbcActionData.targetObject);
	}

	override bool ReadFromContext(ParamsReadContext ctx, out ActionReciveData action_recive_data)
	{
		if(!action_recive_data)
		{
			action_recive_data = new HDSN_PlaceBreachingChargeReceiveData;
		}

		super.ReadFromContext(ctx, action_recive_data);

		HDSN_PlaceBreachingChargeReceiveData pbcReceiveActionData = HDSN_PlaceBreachingChargeReceiveData.Cast(action_recive_data);

		vector entityPosition = "0 0 0";
		vector entityOrientation = "0 0 0";
		Object entityTarget = null;

		if(!ctx.Read(entityPosition))
		{
			return false;
		}

		if(!ctx.Read(entityOrientation))
		{
			return false;
		}

		if(!ctx.Read(entityTarget))
		{
			return false;
		}

		pbcReceiveActionData.position = entityPosition;
		pbcReceiveActionData.orientation = entityOrientation;
		pbcReceiveActionData.targetObject = entityTarget;

		return true;
	}

	override void HandleReciveData(ActionReciveData action_recive_data, ActionData action_data)
	{
		super.HandleReciveData(action_recive_data, action_data);

		HDSN_PlaceBreachingChargeReceiveData pbcReceiveActionData = HDSN_PlaceBreachingChargeReceiveData.Cast(action_recive_data);
		HDSN_PlaceBreachingChargeActionData pbcActionData = HDSN_PlaceBreachingChargeActionData.Cast(action_data);

		pbcActionData.position = pbcReceiveActionData.position;
		pbcActionData.orientation = pbcReceiveActionData.orientation;
		pbcActionData.targetObject = pbcReceiveActionData.targetObject;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if(!HDSN_DestructionManager.GetInstance().IsRaidingAllowed())
		{
			return false;
		}

		if(!GetGame().IsClient())
		{
			return true;
		}

		if(!player.IsPlacingLocal())
		{
			return false;
		}

		player.GetHologramLocal().SetActionDeployBreachingCharge(this);

		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(item);
		if(charge)
		{
			raycastDistance = charge.GetPlacementDistance();
		}

		vector playerHeadPos;
		MiscGameplayFunctions.GetHeadBonePos(player, playerHeadPos);
		float cameraToPlayerDistance = vector.Distance(GetGame().GetCurrentCameraPosition(), playerHeadPos);
		vector startPosition = GetGame().GetCurrentCameraPosition();
		vector endPosition = startPosition + (GetGame().GetCurrentCameraDirection() * (raycastDistance + cameraToPlayerDistance));

		vector hitPosition;
		vector hitDirection;
		int hitComponent;
		Object hitObject;

		bool raycastHit = false;

		if(HDSN_BreachingChargeConfigManager.GetConfig().EnableExperimentalPlacing)
		{
			float dist2ndRC = HDSN_BreachingChargeConfigManager.GetConfig().ExperimentalPlacingRaycastDistance;
			raycastHit = HDSN_Functions.CustomRaycastExp(startPosition, endPosition, hitPosition, hitDirection, hitComponent, hitObject, player, player.GetHologramLocal().GetProjectionEntity(), dist2ndRC);
		}
		else
		{
			raycastHit = HDSN_Functions.CustomRaycast(startPosition, endPosition, hitPosition, hitDirection, hitComponent, hitObject, player, player.GetHologramLocal().GetProjectionEntity());
		}

		if(raycastHit)
		{
			if(hitObject)
			{

				//check if far enough away -> setting in the json
				vector positionPlayer = playerHeadPos;
				positionPlayer[1] = 1;

				vector posHit2D = hitPosition;
				posHit2D[1] = 1;

				float distanceHeadToHitSq = vector.DistanceSq(positionPlayer, posHit2D);
				float minDist = charge.GetMinPlacementDistance();
				if(distanceHeadToHitSq < minDist * minDist)
				{
					return false;
				}

				//slight offset along the mesh-normal of the hit object -> charges tend to clip inside some models a tiny bit too much
				//this tries to prevent the charges from being completely hidden inside an object
				hitPosition += hitDirection.Normalized() * GetOffsetDistance(hitObject, item, hitComponent);

				int levelIndex = 0;
				int wallIndex = 0;

				if(hitObject.GetType() == "Watchtower")
				{
					levelIndex = HDSN_DestructionManager.GetInstance().GetWatchtowerFloorIndex(hitObject, hitPosition);
					wallIndex = HDSN_DestructionManager.GetInstance().GetWatchtowerWallIndex(hitObject, hitPosition, levelIndex);
				}

				if(HDSN_DestructionManager.GetInstance().IsChargeAllowed(hitObject, item, hitComponent, levelIndex, wallIndex))
				{
					bool isTargetValid = true;

					if(hitObject.GetType() == "Watchtower")
					{
						if(HDSN_DestructionManager.GetInstance().IsWatchtowerWallBuildOnLevel(hitObject, levelIndex))
						{
							//Charge not in valid area or specified wall isn't build
							if(wallIndex == -1 || !HDSN_DestructionManager.GetInstance().IsWallBuild(hitObject, levelIndex, wallIndex))
							{
								isTargetValid = false;
							}
						}
						else
						{
							//Charge not in valid area or base on floor is not build
							if(wallIndex == -1 || !HDSN_DestructionManager.GetInstance().IsWatchtowerBaseBuildOnLevel(hitObject, levelIndex))
							{
								isTargetValid = false;
							}

							if(wallIndex > 0)
							{
								if(!HDSN_DestructionManager.GetInstance().IsWallBuild(hitObject, levelIndex, wallIndex))
								{
									isTargetValid = false;
								}
							}
						}
					}

					if(isTargetValid)
					{
						raycastTarget = hitObject;
						HDSN_BreachingChargeBase.Cast(item).SetTarget(raycastTarget);
						actionText = GetCustomActionTextPlanting(raycastTarget.GetDisplayName(), item.GetDisplayName());
						contactPos = hitPosition;
						contactDir = hitDirection;
						return true;
					}
				}
			}
		}

		contactPos = vector.Zero;
		contactDir = vector.Zero;

		raycastTarget = null;
		HDSN_BreachingChargeBase.Cast(item).SetTarget(null);
		actionText = "No target";

		return false;
	}

	override string GetText()
	{
		return actionText;
	}


	override bool HasTarget()
	{
		return false;
	}

	override bool HasProgress()
	{
		return true;
	}

	override bool HasAlternativeInterrupt()
	{
		return true;
	}

	string GetCustomActionTextPlanting(string objectType, string itemDisplayName)
	{
		return "#HDSN_TEXT_PLANT " + itemDisplayName + " #HDSN_TEXT_ON " + objectType;
	}

	//Override this if you want charges to have a different offset along the raycast's hit-normal (for example to prevent clipping)
	//you can use the parameters target and charge to set it differently only in specific situations
	float GetOffsetDistance(Object target, ItemBase charge, int component)
	{
		string targetType = target.GetType();
		string chargeType = charge.GetType();

		if(chargeType == "HDSN_BreachingCharge")
		{
			switch(targetType)
			{
				case "BBP_T1_Gate":
					return 0.2;
				case "BBP_T3_Gate":
					return 0.2;
				case "BBP_T3_Door_Half":
					return 0.2;
				case "ExpansionWall6x3":
					return 0.2;
				case "BBP_Pillar":
					return 0.2;
			}
		}
		else if(chargeType == "HDSN_BreachingChargeHeavy")
		{
			switch(targetType)
			{
				case "BBP_T1_Gate":
					return 0.25;
				case "BBP_T3_Gate":
					return 0.25;
				case "BBP_T2_Door":
					return 0.2;
				case "BBP_T3_Door":
					return 0.2;
				case "BBP_T3_Door_Half":
					return 0.25;
				case "ExpansionWall6x3":
					return 0.3;
				case "BBP_Pillar":
					return 0.2;
				case "EvenMoreDoor":
					return 0.2;
				case "EvenMoreDoor_Double":
					return 0.2;
			}
		}

		return 0.1;
	}

	vector GetContactPos()
	{
		return contactPos;
	}

	vector GetContactDir()
	{
		return contactDir;
	}

	Object GetRaycastTarget()
	{
		return raycastTarget;
	}
}
