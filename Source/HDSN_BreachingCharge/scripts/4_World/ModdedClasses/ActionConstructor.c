modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);

		actions.Insert(HDSN_ActionTogglePlantBreachingCharge);
		actions.Insert(HDSN_ActionDeployBreachingCharge);
		actions.Insert(HDSN_ActionDefuseBreachingCharge);
		actions.Insert(HDSN_ActionArmBreachingCharge);
    }
}
