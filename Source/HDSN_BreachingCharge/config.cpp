#define _ARMA_

class CfgPatches
{
	class HDSN_BreachingCharge
	{
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"JM_CF_Scripts"};
		ammo[] = {"BreachingchargeExplosion"};
	};
};
class CfgMods
{
	class HDSN_BreachingCharge
	{
		dir = "HDSN_BreachingCharge";
		name = "BreachingCharge";
		credits = "HandsomeNipple";
		author = "HandsomeNipple";
		authorID = "71122";
		version = "0.1";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		extra = 0;
		type = "mod";
		dependencies[] = {"World","Mission"};
		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"HDSN_BreachingCharge/scripts/Common","HDSN_BreachingCharge/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"HDSN_BreachingCharge/scripts/Common","HDSN_BreachingCharge/scripts/5_Mission"};
			};
		};
	};
};
class CfgVehicles
{
	class Inventory_Base;
	class HDSN_BreachingChargeBase: Inventory_Base
	{
		scope = 0;
		overrideDrawArea = "3.0";
		forceFarBubble = "true";
		canBeSplit = 0;
		stackedUnit = "l";
		quantityBar = 1;
		varQuantityInit = 100000;
		varQuantityMin = 0;
		varQuantityMax = 100000;
		varQuantityDestroyOnMin = 0;
		class EnergyManager
		{
			energyAtSpawn = 1;
			energyUsagePerSecond = 0;
			updateInterval = 1;
			convertEnergyToQuantity = 1;
		};
	};
	class HDSN_BreachingCharge: HDSN_BreachingChargeBase
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_HOMEMADE_BREACHINGCHARGE";
		descriptionShort = "#HDSN_DESC_HOMEMADE_BREACHINGCHARGE";
		model = "HDSN_BreachingCharge\data\models\BreachingCharge.p3d";
		weight = 5000;
		itemSize[] = {4,4};
		itemBehaviour = 1;
		rotationFlags = 1;
		memoryPoints[] = {"screenlight"};
	};
	class HDSN_BreachingChargeHeavy: HDSN_BreachingChargeBase
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_HEAVY_BREACHINGCHARGE";
		descriptionShort = "#HDSN_DESC_HEAVY_BREACHINGCHARGE";
		model = "HDSN_BreachingCharge\data\models\HeavyCharge.p3d";
		weight = 2000;
		itemSize[] = {3,2};
		itemBehaviour = 1;
		rotationFlags = 1;
		memoryPoints[] = {"screenlight"};
	};
	class HDSN_Detonator: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_DETONATOR";
		descriptionShort = "#HDSN_DESC_DETONATOR";
		model = "HDSN_BreachingCharge\data\models\Detonator.p3d";
		rotationFlags = 1;
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {2,1};
		weight = 500;
	};
	class HDSN_C4Sticks: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_C4STICKS";
		descriptionShort = "#HDSN_DESC_C4STICKS";
		model = "HDSN_BreachingCharge\data\models\C4_Sticks.p3d";
		rotationFlags = 1;
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {3,2};
		weight = 1400;
	};
	class HDSN_C4Stick: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_C4STICK";
		descriptionShort = "#HDSN_DESC_C4STICK";
		model = "HDSN_BreachingCharge\data\models\C4_Stick.p3d";
		rotationFlags = 1;
		itemBehaviour = 1;
		canBeSplit = 1;
		itemSize[] = {3,1};
		weight = 700;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 2;
		varQuantityDestroyOnMin = 1;
	};
	class HDSN_C4NoWiring: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_C4_NOWIRING";
		descriptionShort = "#HDSN_DESC_C4_NOWIRING";
		model = "HDSN_BreachingCharge\data\models\HeavyChargeWOWire.p3d";
		rotationFlags = 1;
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {3,2};
		weight = 1900;
	};
	class HDSN_Explosive: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_Explosive";
		descriptionShort = "#HDSN_DESC_Explosive";
		model = "HDSN_BreachingCharge\data\models\Explosive.p3d";
		rotationFlags = 1;
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {4,4};
		weight = 4000;
	};
	class HDSN_Charcoal: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_CHARCOALPOWDER";
		descriptionShort = "#HDSN_DESC_CHARCOALPOWDER";
		model = "HDSN_BreachingCharge\data\models\charcoal.p3d";
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {2,2};
		weight = 200;
		class AnimEvents
		{
			class SoundWeapon
			{
				class VitaminBottle_open
				{
					soundSet = "VitaminBottle_open_SoundSet";
					id = 201;
				};
				class VitaminBottle_shake
				{
					soundSet = "";
					id = 202;
				};
				class VitaminBottle_close
				{
					soundSet = "VitaminBottle_close_SoundSet";
					id = 203;
				};
				class pickUpItem
				{
					soundSet = "vitaminbottle_pickup_SoundSet";
					id = 797;
				};
			};
		};
	};
	class HDSN_Sulfur: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_SULFURPOWDER";
		descriptionShort = "#HDSN_DESC_SULFURPOWDER";
		model = "HDSN_BreachingCharge\data\models\sulfur.p3d";
		itemBehaviour = 1;
		canBeSplit = 0;
		itemSize[] = {2,2};
		weight = 200;
		class AnimEvents
		{
			class SoundWeapon
			{
				class VitaminBottle_open
				{
					soundSet = "VitaminBottle_open_SoundSet";
					id = 201;
				};
				class VitaminBottle_shake
				{
					soundSet = "";
					id = 202;
				};
				class VitaminBottle_close
				{
					soundSet = "VitaminBottle_close_SoundSet";
					id = 203;
				};
				class pickUpItem
				{
					soundSet = "vitaminbottle_pickup_SoundSet";
					id = 797;
				};
			};
		};
	};
	class HDSN_Gunpowder: Inventory_Base
	{
		scope = 2;
		displayName = "#HDSN_DISPLAYNAME_GUNPOWDER";
		descriptionShort = "#HDSN_DESC_GUNPOWDER";
		model = "HDSN_BreachingCharge\data\models\gunpowder.p3d";
		itemBehaviour = 1;
		weight = 500;
		canBeSplit = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 5;
		varQuantityDestroyOnMin = 1;
		itemSize[] = {2,3};
		class AnimEvents
		{
			class SoundWeapon
			{
				class Stitch
				{
					soundSet = "";
					id = 201;
				};
				class pickup
				{
					soundSet = "sewingkit_pickup_SoundSet";
					id = 797;
				};
				class drop
				{
					soundset = "sewingkit_drop_SoundSet";
					id = 898;
				};
			};
		};
	};
};
class cfgSoundSets
{
	class HDSN_SoundSet_Beeping
	{
		soundShaders[] = {"HDSN_SoundShader_Beeping"};
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class HDSN_SoundSet_Beeping_Fast
	{
		soundShaders[] = {"HDSN_SoundShader_Beeping_Fast"};
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class HDSN_SoundSet_Beeping_Medium_Fast
	{
		soundShaders[] = {"HDSN_SoundShader_Beeping_Medium_Fast"};
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class HDSN_SoundSet_Beeping_Medium_Slow
	{
		soundShaders[] = {"HDSN_SoundShader_Beeping_Medium_Slow"};
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class HDSN_SoundSet_Beeping_Slow
	{
		soundShaders[] = {"HDSN_SoundShader_Beeping_Slow"};
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
};
class cfgSoundShaders
{
	class HDSN_SoundShader_Beeping
	{
		samples[] = {{"HDSN_BreachingCharge\data\sounds\Beeping",1}};
		volume = 1;
		range = 100;
		limitation = 0;
	};
	class HDSN_SoundShader_Beeping_Fast
	{
		samples[] = {{"HDSN_BreachingCharge\data\sounds\Beeping_Fast",1}};
		volume = 1;
		range = 100;
		limitation = 0;
	};
	class HDSN_SoundShader_Beeping_Medium_Fast
	{
		samples[] = {{"HDSN_BreachingCharge\data\sounds\Beeping_Medium_Fast",1}};
		volume = 1;
		range = 100;
		limitation = 0;
	};
	class HDSN_SoundShader_Beeping_Medium_Slow
	{
		samples[] = {{"HDSN_BreachingCharge\data\sounds\Beeping_Medium_Slow",1}};
		volume = 1;
		range = 100;
		limitation = 0;
	};
	class HDSN_SoundShader_Beeping_Slow
	{
		samples[] = {{"HDSN_BreachingCharge\data\sounds\Beeping_Slow",1}};
		volume = 1;
		range = 100;
		limitation = 0;
	};
};
class CfgAmmo
{
	class DefaultAmmo;
	class BreachingchargeExplosion: DefaultAmmo
	{
		hit = 0.1;
		indirectHit = 1;
		indirectHitRange = 1.5;
		particle = "explosion_landmine_01";
		explosive = 1;
		typicalSpeed = 3;
		initSpeed = 3;
		fuseDistance = 1;
		simulation = "shotShell";
		simulationStep = 0.05;
		soundSetExplosion[] = {"Landmine_Explosion_SoundSet","Landmine_Explosion_Tail_SoundSet"};
		soundHit[] = {"DZ\sounds\weapons\grenades\hand_grenade_1",10,1};
		soundFly[] = {"dz\sounds\Weapons\explosions\noise",3.162277e-05,1};
		soundEngine[] = {"",9.999998e-05,4};
		class DamageApplied
		{
			type = "Projectile";
			class Health
			{
				damage = 99;
			};
			class Blood
			{
				damage = 500;
			};
			class Shock
			{
				damage = 500;
			};
		};
		class NoiseExplosion
		{
			strength = 100;
			type = "sound";
		};
	};
};
class CfgModels
{
	class BreachingCharge
	{
		sections[] = {"camo"};
	};
	class Explosive
	{
		sections[] = {"all"};
	};
	class HeavyCharge
	{
		sections[] = {"all"};
	};
	class HeavyChargeWOWire
	{
		sections[] = {"all"};
	};
	class C4_Stick
	{
		sections[] = {"all"};
	};
	class C4_Sticks
	{
		sections[] = {"all"};
	};
	class charcoal
	{
		sections[] = {"all"};
	};
	class Detonator
	{
		sections[] = {"all"};
	};
	class gunpowder
	{
		sections[] = {"all"};
	};
	class sulfur
	{
		sections[] = {"all"};
	};
};
