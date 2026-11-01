modded class PluginRecipesManagerBase extends PluginBase
{
	override void RegisterRecipies()
	{
		super.RegisterRecipies();						// Calling base RegisterRecipies()
		RegisterRecipe(new HDSN_GunpowderRecipe);		// Registering Gunpowder Recipe
		RegisterRecipe(new HDSN_ExplosiveRecipe);		// Registering Explosive Recipe
		RegisterRecipe(new HDSN_BreachingChargeRecipe);	// Registering Breachingcharge Recipe
		RegisterRecipe(new HDSN_C4AndDetonatorRecipe);
		RegisterRecipe(new HDSN_C4WithWiringRecipe);
		RegisterRecipe(new HDSN_C4SticksRecipe);
	}
};
