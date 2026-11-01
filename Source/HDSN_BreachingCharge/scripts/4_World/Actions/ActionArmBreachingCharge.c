class HDSN_ActionArmBreachingCharge : ActionInteractBase
{
	void HDSN_ActionArmBreachingCharge()
	{
		m_CommandUID        = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask        = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
	}

	override string GetText()
	{
		return "#HDSN_TEXT_ARM";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNonRuined(UAMaxDistances.DEFAULT);
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		Object targetObj = target.GetObject();
		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(targetObj);

		if(!charge)
		{
			return false;
		}

		if(GetGame().IsServer() && !charge.HasTarget())
		{
			return false;
		}

		if(charge.GetChargeState() == HDSN_BreachingChargeStates.IS_PLACED)
		{
			return true;
		}

		return false;
	}

	override void OnEndServer( ActionData action_data )
	{
		Object targetObj = action_data.m_Target.GetObject();
		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(targetObj);

		if(charge)
		{
			charge.ArmBreachingCharge();
		}
	}
}
