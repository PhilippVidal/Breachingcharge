modded class ModItemRegisterCallbacks
{
	override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
		super.RegisterOneHanded(pType, pBehavior);

		pType.AddItemInHandsProfileIK("HDSN_BreachingCharge", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/child_g.anm");
		pType.AddItemInHandsProfileIK("HDSN_Explosive", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/child_g.anm");
		pType.AddItemInHandsProfileIK("HDSN_Gunpowder", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/food_can.anm");
		pType.AddItemInHandsProfileIK("HDSN_Charcoal", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/food_can.anm");
		pType.AddItemInHandsProfileIK("HDSN_Sulfur", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/food_can.anm");
	}
}
