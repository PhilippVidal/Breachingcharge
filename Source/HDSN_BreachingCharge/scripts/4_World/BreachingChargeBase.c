enum HDSN_BreachingChargeStates
{
	NOT_PLACED,
	IS_PLACED,
	IS_ARMED,
	HAS_EXPLODED,
	IS_DEFUSED
};

class HDSN_BreachingChargeBase : ItemBase
{
	protected string m_ChargeTargetType;
	protected Object chargeTarget;
	protected PlayerBase m_PlacingPlayer;
	protected EffectSound m_BeepingSound;
	protected int m_CurrentState;
	protected int m_LastState;
	protected HDSN_ChargeParams m_ChargeParams;
	protected HDSN_BreachingChargeLight m_ChargeLight;
	protected Particle m_ChargeExplosionEffect;
	protected EffectSound chargeExplosionSound;
	ref array<Object> m_ObjectsInRadius;

	void HDSN_BreachingChargeBase()
	{
		RegisterNetSyncVariableInt("m_CurrentState");
		m_CurrentState = 0;
		m_LastState = m_CurrentState;
		m_ChargeTargetType = "";
	}

	void ~HDSN_BreachingChargeBase()
	{
		if(m_ChargeLight)
		{
			m_ChargeLight.FadeOut();
		}

		if(m_ChargeExplosionEffect)
		{
			m_ChargeExplosionEffect.Delete();
		}

		if(chargeExplosionSound)
		{
			delete chargeExplosionSound;
		}
	}


	HDSN_BreachingChargeConfigManager GetConfigManagerInstance()
	{
		return HDSN_BreachingChargeConfigManager.GetInstance();
	}

	HDSN_DestructionManager GetDestructionManager()
	{
		return HDSN_DestructionManager.GetInstance();
	}

	PlayerBase GetPlacingPlayer()
	{
		return m_PlacingPlayer;
	}

	void PlaceBreachingCharge(PlayerBase placingPlayer, Object targetobject)
	{
		SetTarget(targetobject);
		SetPlacingPlayer(placingPlayer);

		if(GetGame().IsServer())
		{
			SetChargeStateSynchronized(1);
			SetChargeTargetType();
			OnChargePlaced();
		}
	}

	void ArmBreachingCharge()
	{
		SetChargeStateSynchronized(2);
		GetCompEM().SwitchOn();
		SetAllowDamage(false);
		ResetTimer();
		OnChargeArmed();
	}

	void ExplodeBreachingCharge()
	{
		SetChargeStateSynchronized(3);

		GetCompEM().SwitchOff();

		if(GetGame().IsServer())
		{
			DamageEntitiesInRadius();

			if(chargeTarget)
			{
				OnChargeExploded();

				int targetHealth = GetDestructionManager().HandleHealth(chargeTarget, HDSN_BreachingChargeBase.Cast(this));
				if(m_CurrentState == 3 && targetHealth <= 0)
				{
					GetDestructionManager().DestroyTarget(chargeTarget, m_PlacingPlayer, HDSN_BreachingChargeBase.Cast(this));
				}
			}

			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Delete, 2000);
		}

		if(m_ChargeLight)
		{
			m_ChargeLight.FadeOut();
		}

		StopBeepingSound();
		CreateChargeExplosion();
		SetInvisible(true);
	}

	void DefuseBreachingCharge(PlayerBase player)
	{
		SetChargeStateSynchronized(4);
		GetCompEM().SwitchOff();
		OnChargeDefused(player);
	}

	void CreateChargeExplosion()
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			vector chargeOrientation = GetOrientation();
			vector wantedOrientationChange = Vector(0.0, -90.0, 0.0);
			vector chargeOrientationMat[3];
			chargeOrientation.RotationMatrixFromAngles(chargeOrientationMat);
			vector wantedOrientationChangeMat[3];
			wantedOrientationChange.RotationMatrixFromAngles(wantedOrientationChangeMat);
			vector res[3];
			Math3D.MatrixMultiply3(chargeOrientationMat, wantedOrientationChangeMat, res);
			vector orientation = Math3D.MatrixToAngles(res);
			m_ChargeExplosionEffect = GetChargeExplosionParticle();
			m_ChargeExplosionEffect.SetOrientation(orientation);
			m_ChargeExplosionEffect.PlayParticle();

			PlaySoundSet(chargeExplosionSound, GetExplosionSoundSetName(), 0.0, 0.0);

			SpawnChargeExplosionEffectOnPlayer();
		}
	}

	void SpawnChargeExplosionEffectOnPlayer()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		float distanceToPlayer = vector.Distance(GetPosition(), player.GetPosition());

		if(distanceToPlayer <= GetMaxDamageToPlayersRadius())
		{
			player.SpawnShockEffect(100);
			//player.OnPlayerReceiveFlashbangHitStart(false);
		}

		if(distanceToPlayer <= GetDamageToPlayersRadius())
		{
			player.GetCurrentCamera().SpawnCameraShake(20);
		}
	}

	void DamageEntitiesInRadius()
	{
		if(!GetGame().IsServer())
		{
			return;
		}

		float damageToDestroyableObjectsRadius = GetDamageToDestroyableObjectsRadius();
		float damageToPlayersRadius = GetDamageToPlayersRadius();
		float maxDamageToPlayersRadius = GetMaxDamageToPlayersRadius();
		float maxDamageToPlayers = GetMaxDamageToPlayers();
		float minDamageToPlayers = GetMinDamageToPlayers();

		int verticalDistanceModeObjects = GetVerticalDistanceModeObjects();
		float maxVerticalDistanceObjects = GetMaxVerticalDistanceObjects();
		float maxVerticalDistancePlayer = GetMaxVerticalDistancePlayers();

		bool destroyOtherCharges = GetDestroyOtherCharges();

		//if all values are (basically) 0, don't do anything
		//this prevents the server from getting all objects in the area if they aren't needed anyways
		if(damageToDestroyableObjectsRadius < 0.01 && (damageToPlayersRadius < 0.01 || maxDamageToPlayers < 0.01))
		{
			return;
		}

		if(damageToPlayersRadius < maxDamageToPlayersRadius)
		{
			maxDamageToPlayersRadius = damageToPlayersRadius;
		}

		if(maxDamageToPlayers < minDamageToPlayers)
		{
			minDamageToPlayers = maxDamageToPlayers;
		}

		FindObjectsInRadius(damageToDestroyableObjectsRadius, damageToPlayersRadius);

		for(int i = 0; i < m_ObjectsInRadius.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(m_ObjectsInRadius[i]);

			if(player)
			{

				if(damageToPlayersRadius < 0.01 || maxDamageToPlayers < 0.01)
				{
					continue;
				}

				float distancePlayerToCharge = vector.Distance(player.GetPosition(), GetPosition());

				//Check if outside defined radius
				if(distancePlayerToCharge > damageToPlayersRadius)
				{
					continue;
				}

				if(maxVerticalDistancePlayer > 0.01)
				{
					if(HDSN_Functions.GetDifference(GetPosition()[1], player.GetPosition()[1]) > maxVerticalDistancePlayer)
					{
						continue;
					}
				}

				float playerHealth = player.GetHealth();

				float scalingDistance = damageToPlayersRadius - maxDamageToPlayersRadius;
				float distanceToChargeWithOffset = distancePlayerToCharge - maxDamageToPlayersRadius;

				if(distancePlayerToCharge <= maxDamageToPlayersRadius)
				{
					playerHealth -= maxDamageToPlayers;
				}
				else
				{
					float distancePercentage;
					//prevents division by zero
					if(scalingDistance < 0.01)
					{
						distancePercentage = 0.0;
					}
					else
					{
						distancePercentage = distanceToChargeWithOffset / scalingDistance;
					}

					float damageToDeal = maxDamageToPlayers - (maxDamageToPlayers - minDamageToPlayers) * distancePercentage;
					playerHealth -= damageToDeal;
				}

				//if-check to prevent wrong input-values from healing the player
				if(playerHealth <= player.GetHealth())
				{
					player.SetHealth(playerHealth);
				}
			}
			else //Damage all objects that have an entry in the destroyable objects list of the json-file
			{
				if(damageToDestroyableObjectsRadius < 0.01)
				{
					continue;
				}

				//Don't damage/destroy the object the charge is planted on or the charge itself
				if(m_ObjectsInRadius[i] == chargeTarget || m_ObjectsInRadius[i] == this)
				{
					continue;
				}

				float destroyableObjectsRadiusSquared = damageToDestroyableObjectsRadius * damageToDestroyableObjectsRadius;
				vector objectPosition = m_ObjectsInRadius[i].GetPosition();
				float distanceToCharge = vector.DistanceSq(objectPosition, GetPosition());

				//Check if outside defined radius
				if(distanceToCharge > destroyableObjectsRadiusSquared)
				{
					continue;
				}

				//Not sure how to handle watchtowers with this yet, for now they are not affected by this at all
				if(m_ObjectsInRadius[i].GetType() == "Watchtower")
				{
					continue;
				}

				if(verticalDistanceModeObjects > 0)
				{
					float heightDifference;
					bool isBelowDistance = true;

					if(verticalDistanceModeObjects == 1)
					{
						heightDifference = HDSN_Functions.GetDifference(GetPosition()[1], objectPosition[1]);
						if(heightDifference > maxVerticalDistanceObjects)
						{
							isBelowDistance = false;
						}
					}
					else if(verticalDistanceModeObjects == 2)
					{
						heightDifference = HDSN_Functions.GetDifference(chargeTarget.GetPosition()[1], objectPosition[1]);
						if(heightDifference > maxVerticalDistanceObjects)
						{
							isBelowDistance = false;
						}
					}
					else
					{
						heightDifference = HDSN_Functions.GetDifference(GetPosition()[1], objectPosition[1]);
						if(heightDifference > maxVerticalDistanceObjects)
						{
							isBelowDistance = false;
						}

						heightDifference = HDSN_Functions.GetDifference(chargeTarget.GetPosition()[1], objectPosition[1]);

						if(heightDifference > maxVerticalDistanceObjects)
						{
							isBelowDistance = false;
						}
					}

					if(!isBelowDistance)
					{
						continue;
					}
				}

				if(destroyOtherCharges)
				{
					HDSN_BreachingChargeBase targetedCharge = HDSN_BreachingChargeBase.Cast(m_ObjectsInRadius[i]);
					if(targetedCharge)
					{
						if(targetedCharge.GetChargeState() == 1 || targetedCharge.GetChargeState() == 2)
						{
							targetedCharge.DefuseBreachingCharge(m_PlacingPlayer);
							GetGame().ObjectDelete(m_ObjectsInRadius[i]);
							continue;
						}

					}
				}

				if(GetDestructionManager().IsChargeAllowed(m_ObjectsInRadius[i], this))
				{
					int targetHealth = GetDestructionManager().HandleHealth(m_ObjectsInRadius[i], HDSN_BreachingChargeBase.Cast(this));

					if(m_CurrentState == 3 && targetHealth <= 0)
					{
						GetDestructionManager().DestroyTarget(m_ObjectsInRadius[i], m_PlacingPlayer, HDSN_BreachingChargeBase.Cast(this));
					}
				}
			}
		}
	}

	void FindObjectsInRadius(float radiusObjects, float radiusPlayers)
	{
		m_ObjectsInRadius = new array<Object>;
		array<CargoBase> proxyCargos = new array<CargoBase>;
		float radius = 0;

		//Choose the bigger radius so it only has to get the objects once
		if(radiusObjects < radiusPlayers)
		{
			radius = radiusPlayers;
		}
		else
		{
			radius = radiusObjects;
		}

		GetGame().GetObjectsAtPosition3D(GetPosition(), radius, m_ObjectsInRadius, proxyCargos);
	}



	void ResetTimer()
	{
		if(GetGame().IsServer())
		{
			GetCompEM().SetEnergyMaxPristine(GetTimeToExplode());
			GetCompEM().SetEnergy(GetCompEM().GetEnergyMaxPristine());
			GetCompEM().SetEnergyUsage(1.0);

			SetQuantityMax();
		}
	}

	void CreateChargeLight()
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			m_ChargeLight = HDSN_BreachingChargeLight.Cast(GetGame().CreateObject("HDSN_BreachingChargeLight", "0 0 0", true));
			m_ChargeLight.AttachOnMemoryPoint(this, "screenlight");
			m_ChargeLight.InitChargeLight();
		}
	}

	//Temporary fix (after DayZ 1.14 the beeping sound would only be played once)
	//The reason for this is that Event_OnSoundWaveEnded does not get called anymore after the sound has already played before, so the sound length is not determined automatically anymore
	//If you use a custom sound for the beeping of a charge just override this function to reflect the length of your custom sound file (in seconds)
	float GetBeepingSoundLength()
	{
		return 0.088;
	}

	void PlayBeepingSound()
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			if(m_CurrentState != 2)
			{
				StopBeepingSound();
				return;
			}

			if(m_BeepingSound)
			{
				//m_BeepingSound.Event_OnSoundWaveEnded.Remove(OnBeepingSoundEnded);
			}

			string soundSetName = GetBeepingSoundSet();
			if(soundSetName == "" || !PlaySoundSet(m_BeepingSound, soundSetName, 0.0, 0.0))
			{
				return;
			}

			//======= Temp =======
			int msTillSoundEnds = Math.Ceil(GetBeepingSoundLength() * 1000);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnBeepingSoundEnded, msTillSoundEnds, false, m_BeepingSound);
			//====================

			///m_BeepingSound.Event_OnSoundWaveEnded.Insert(OnBeepingSoundEnded);
		}
	}

	void StopBeepingSound()
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			if(m_ChargeLight)
			{
				m_ChargeLight.ToggleChargeLightOff();
			}

			if(m_BeepingSound)
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).RemoveByName(this, "OnBeepingSoundEnded");
				//m_BeepingSound.Event_OnSoundWaveEnded.Remove(OnBeepingSoundEnded);
				StopSoundSet(m_BeepingSound);
			}
		}
	}

	void OnBeepingSoundEnded(EffectSound beepingSound)
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			float timeLeft = GetChargeTimeLeft();
			//float beepingSoundLength = beepingSound.GetSoundWaveLenght();
			float beepingSoundLength = GetBeepingSoundLength();
			float beepingSoundEndTime = GetBeepingSoundEndTime();
			float beepingInterval = GetSwitchInterval();

			if(timeLeft <= beepingSoundLength)
			{
				StopBeepingSound();
				m_ChargeLight.ToggleChargeLightOff();
				return;
			}

			m_ChargeLight.ToggleChargeLight();

			int msTillNextBeep = CalculateTimeTillNextBeep(timeLeft, beepingSoundEndTime, beepingInterval, GetTimeToExplode());
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PlayBeepingSound, msTillNextBeep);
		}
	}

	int CalculateTimeTillNextBeep(float timeleft, float beepingSoundEndTime, float beepingInterval, float maxTime)
	{
		if(beepingSoundEndTime >= maxTime || beepingSoundEndTime > timeleft)
		{
			return 0;
		}

		float maxTimeWithOffset = maxTime - beepingSoundEndTime;
		float timeLeftWithOffset = timeleft - beepingSoundEndTime;
		float percentage = timeLeftWithOffset / maxTimeWithOffset;
		float msTillNextBeep = (beepingInterval * percentage) * 1000;
		return (int)msTillNextBeep;
	}

	float GetChargeTimeLeft()
	{
		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			return  GetTimeToExplode() * (GetQuantity() / (float)GetQuantityMax());
		}
		else
		{
			return  GetEnergy();
		}
	}

	// The charge quantity represents its countdown, not additional item mass.
	override protected float GetWeightSpecialized(bool forceRecalc = false)
	{
		return EntityAIType.Cast(GetEntityType()).m_ConfigWeight + GetInventoryAndCargoWeight(forceRecalc);
	}

	Particle GetChargeExplosionParticle()
	{
		return Particle.CreateInWorld(ParticleList.EXPLOSION_LANDMINE, GetPosition());
	}

/////////////////////////////////
//	Overridden DayZ Functions
/////////////////////////////////
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if(GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			if(m_LastState != m_CurrentState)
			{
				//Print("m_LastState = " + typename.EnumToString(HDSN_BreachingChargeStates, m_LastState));
				//Print("m_CurrentState = " + typename.EnumToString(HDSN_BreachingChargeStates, m_CurrentState));

				switch(m_CurrentState)
				{
					case HDSN_BreachingChargeStates.IS_ARMED:
						CreateChargeLight();
						PlayBeepingSound();
					break;

					case HDSN_BreachingChargeStates.HAS_EXPLODED:
						ExplodeBreachingCharge();
					break;

					case HDSN_BreachingChargeStates.IS_DEFUSED:
						if(m_ChargeLight)
						{
							m_ChargeLight.FadeOut();
						}

						StopBeepingSound();
					break;
				}

				m_LastState = m_CurrentState;
			}
		}
	}

	override void OnWorkStop()
	{
		super.OnWorkStop();

		if(GetGame().IsServer())
		{
			if(m_CurrentState == HDSN_BreachingChargeStates.IS_ARMED && GetChargeTimeLeft() <= 1)
			{
				SetAllowDamage(true);
				ExplodeBreachingCharge();
			}

			if(m_CurrentState == HDSN_BreachingChargeStates.IS_DEFUSED)
			{
				SetAllowDamage(true);
				ResetTimer();
				SetChargeStateSynchronized(HDSN_BreachingChargeStates.NOT_PLACED);
			}
		}
	}

	override void SetActions()
	{
		super.SetActions();

		AddAction(HDSN_ActionDeployBreachingCharge);
		AddAction(HDSN_ActionTogglePlantBreachingCharge);
		AddAction(HDSN_ActionArmBreachingCharge);

		RemoveAction(ActionTogglePlaceObject);
		RemoveAction(ActionPlaceObject);

		//removing and re-adding ActionTakeItem makes it so ActionArmBreachingCharge is shown by default
		//and is done to prevent accidentally picking up the charge once it's placed
		RemoveAction(ActionTakeItem);
		AddAction(ActionTakeItem);
		AddAction(HDSN_ActionDefuseBreachingCharge);
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		if(m_CurrentState == HDSN_BreachingChargeStates.IS_ARMED || m_CurrentState == HDSN_BreachingChargeStates.HAS_EXPLODED)
		{
			return false;
		}

		return super.CanPutInCargo(parent);
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		if(m_CurrentState == HDSN_BreachingChargeStates.IS_ARMED || m_CurrentState == HDSN_BreachingChargeStates.HAS_EXPLODED)
		{
			return false;
		}

		return super.CanPutIntoHands(parent);
	}

	override bool IsDeployable()
	{
		return true;
	}

	bool SetChargeParams()
	{
		if(GetConfigManagerInstance().FindChargeParams(GetType(), m_ChargeParams))
		{
			return true;
		}

		return false;
	}

	HDSN_ChargeParams GetChargeParams()
	{
		FindChargeParams();
		return m_ChargeParams;
	}

	bool FindChargeParams()
	{
		if(m_ChargeParams)
		{
			return true;
		}

		return SetChargeParams();
	}

	//Get charge target type based on GetWallType(), return value of get-method should correspond to destroyable-object names defined in the json-file
	//reason for use: in order to differentiate between (for example) a "Fence", "FenceHalfMetal" and a "FenceMetal"
	//GetType() returns just the object type => would be "Fence" regardless of how many metal pieces are actually build
	string SetChargeTargetType()
	{
		if(chargeTarget)
		{
			int level = 0;
			int wall = 0;

			if(chargeTarget.GetType() == "Watchtower")
			{
				level = GetDestructionManager().GetWatchtowerFloorIndex(chargeTarget, GetPosition());
				wall = GetDestructionManager().GetWatchtowerWallIndex(chargeTarget, GetPosition(), level);
			}

			m_ChargeTargetType = GetDestructionManager().GetWallType(chargeTarget, level, wall);
		}
		else
		{
			m_ChargeTargetType = "";
		}

		return m_ChargeTargetType;
	}

	string GetChargeTargetType()
	{
		if(m_ChargeTargetType == "")
		{
			return SetChargeTargetType();
		}

		return m_ChargeTargetType;
	}

	bool HasTarget()
	{
		if(chargeTarget)
		{
			return true;
		}

		return false;
	}

	Object GetTarget()
	{
		return chargeTarget;
	}

	void SetTarget(Object target)
	{
		chargeTarget = target;
	}

	void SetPlacingPlayer(PlayerBase player)
	{
		m_PlacingPlayer = player;
	}

	int GetChargeState()
	{
		return m_CurrentState;
	}

	void SetChargeState(int state)
	{
		m_CurrentState = state;
	}

	void SetChargeStateSynchronized(int state)
	{
		if ( GetGame().IsServer() )
		{
			m_CurrentState = state;
			SetSynchDirty();
		}
	}

	string GetExplosionSoundSetName()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.ExplosionSoundSet;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetExplosionSoundSetNameF for a charge-type without a json-entry: " + GetType());
		return "Landmine_Explosion_SoundSet";
	}

	int GetChargeDamage()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DamageToObjects;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetChargeDamageF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetDamageToDestroyableObjectsRadius()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DamageToDestroyableObjectsRadius;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetDamageToDestroyableObjectsRadiusF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetMaxDamageToPlayers()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MaxDamageToPlayers;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMaxDamageToPlayersF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetMinDamageToPlayers()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MinDamageToPlayers;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMinDamageToPlayersF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetDamageToPlayersRadius()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DamageToPlayersRadius;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetDamageToPlayersRadiusF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetMaxDamageToPlayersRadius()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MaxDamageToPlayersRadius;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMaxDamageToPlayersRadiusF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	bool GetOnlyDestroyLocks()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.OnlyDestroyLocks;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetOnlyDestroyLocksF for a charge-type without a json-entry: " + GetType());
		return false;
	}

	bool GetDestroyLocksFirst()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DestroyLocksFirst;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetDestroyLocksFirstF for a charge-type without a json-entry: " + GetType());
		return false;
	}

	bool GetDeleteObjectsDirectly()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DeleteObjectsDirectly;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetDeleteObjectsDirectlyF for a charge-type without a json-entry: " + GetType());
		return false;
	}

	float GetPlacementDistance()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.PlacementDistance;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetPlacementDistanceF for a charge-type without a json-entry: " + GetType());
		return 2.0;
	}

	float GetToolDamageOnDefuse()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.ToolDamageOnDefuse;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetToolDamageOnDefuseF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetTimeToPlant()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.TimeToPlant;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetTimeToPlantF for a charge-type without a json-entry: " + GetType());
		return 100000;
	}

	float GetTimeToExplode()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.TimeToExplode;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetTimeToExplodeF for a charge-type without a json-entry: " + GetType());
		return 100000;
	}

	float GetTimeToDefuse()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.TimeToDefuse;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetTimeToDefuseF for a charge-type without a json-entry: " + GetType());
		return 100000;
	}

	string GetBeepingSoundSet()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.BeepingSoundSet;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetBeepingSoundSetF for a charge-type without a json-entry: " + GetType());
		return "HDSN_SoundSet_Beeping";
	}

	float GetBeepingSoundEndTime()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.BeepingSoundEndTime;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetBeepingSoundEndTimeF for a charge-type without a json-entry: " + GetType());
		return 100000;
	}

	float GetSwitchInterval()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.SwitchInterval;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetSwitchIntervalF for a charge-type without a json-entry: " + GetType());
		return 100000;
	}

	int GetVerticalDistanceModeObjects()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.VerticalDistanceModeObjects;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetVerticalDistanceModeObjectsF for a charge-type without a json-entry: " + GetType());
		return 0;
	}

	float GetMaxVerticalDistanceObjects()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MaxVerticalDistanceObjects;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMaxVerticalDistanceObjectsF for a charge-type without a json-entry: " + GetType());
		return 0.0;
	}

	float GetMaxVerticalDistancePlayers()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MaxVerticalDistancePlayers;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMaxVerticalDistancePlayersF for a charge-type without a json-entry: " + GetType());
		return 0.0;
	}

	////////////////////////////////////////////////
	///////////////////////////////////////////////
	///////////////////////////////////////////////
	bool GetDestroyOtherCharges()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.DestroyOtherCharges;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetDestroyOtherChargesF for a charge-type without a json-entry: " + GetType());
		return false;
	}



//Charge "Events", these functions can be modified to do specific custom behaviour (e.g. logging to your own logging-system, calling other functions, etc.)
//without having to worry about breaking anything major if it's not done properly (in the worst case the mod's default logger will not print any logs anymore)
//These functions only get called on the server's side

	//Gets called once the charge has been placed, mainly used as a kind of event to log stuff
	//and to enable custom behaviour through overrides
	void OnChargePlaced()
	{
		if(GetGame().IsServer())
		{
			HDSN_BreachingChargeLogger.GetInstance().LogEvent(HDSN_ChargeEventTypes.PLACED, m_PlacingPlayer, this, GetChargeTargetType());
		}
	}

	//Gets called once the charge has been armed, mainly used as a kind of event to log stuff
	//and to enable custom behaviour through overrides
	void OnChargeArmed()
	{
		if(GetGame().IsServer())
		{
			HDSN_BreachingChargeLogger.GetInstance().LogEvent(HDSN_ChargeEventTypes.ARMED, m_PlacingPlayer, this, GetChargeTargetType());
		}
	}

	//Gets called once the charge has been defused, mainly used as a kind of event to log stuff
	//and to enable custom behaviour through overrides
	void OnChargeDefused(PlayerBase defusingPlayer)
	{
		if(GetGame().IsServer())
		{
			if(defusingPlayer)
			{
				HDSN_BreachingChargeLogger.GetInstance().LogEvent(HDSN_ChargeEventTypes.DEFUSED, m_PlacingPlayer, this, GetChargeTargetType(), defusingPlayer);
			}
		}
	}

	//Gets called once the charge has been exploded, mainly used as a kind of event to log stuff
	//and to enable custom behaviour through overrides
	void OnChargeExploded()
	{
		if(GetGame().IsServer())
		{
			HDSN_BreachingChargeLogger.GetInstance().LogEvent(HDSN_ChargeEventTypes.EXPLODED, m_PlacingPlayer, this, GetChargeTargetType());
		}
	}

	//Gets called once the target of a charge has been destroyed (a charge exploding != target destroyed => therefore a separate function)
	//mainly used as a kind of event to log stuff
	//and to enable custom behaviour through overrides
	void OnTargetBeingDestroyed()
	{
		if(GetGame().IsServer())
		{
			HDSN_BreachingChargeLogger.GetInstance().LogEvent(HDSN_ChargeEventTypes.TARGETDESTROYED, m_PlacingPlayer, this, GetChargeTargetType());
		}
	}

	float GetMinPlacementDistance()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.MinPlacementDistance;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetMinPlacementDistanceF for a charge-type without a json-entry: " + GetType());
		return 0.0;
	}

	bool GetAutomaticallyArm()
	{
		if(FindChargeParams())
		{
			return m_ChargeParams.AutomaticallyArm;
		}

		Print("[Breachingcharge]	ERROR: Trying to call GetAutomaticallyArmF for a charge-type without a json-entry: " + GetType());
		return false;
	}
}
