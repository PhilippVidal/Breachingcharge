class HDSN_ActionTogglePlantBreachingCharge : ActionSingleUseBase
{
	string actionText;

	void HDSN_ActionTogglePlantBreachingCharge()
	{
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool IsLocal()
	{
		return true;
	}

	override bool IsInstant()
	{
		return true;
	}

	override bool RemoveForceTargetAfterUse()
	{
		return false;
	}

	override string GetText()
	{
		return actionText;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if(player.IsPlacingLocal())
		{
			actionText = "#HDSN_TEXT_SELECT_TARGET";
		}
		else
		{
			actionText = "#toggle_placing";
		}

		return true;
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);
		action_data.m_Player.TogglePlacingLocal();
	}
}
