class HDSN_ActionDefuseBreachingChargeCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		float time = HDSN_BreachingChargeBase.Cast(m_ActionData.m_Target.GetObject()).GetTimeToDefuse();
		m_ActionData.m_ActionComponent = new CAContinuousTime(time);
	}
}

class HDSN_ActionDefuseBreachingCharge : ActionContinuousBase
{
	private string targetDisplayName;

	void HDSN_ActionDefuseBreachingCharge()
	{
		m_CallbackClass = HDSN_ActionDefuseBreachingChargeCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;//DayZPlayerConstants.CMD_ACTIONFB_ASSEMBLE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		targetDisplayName = "";
	}

	override string GetText()
	{
		return "#HDSN_TEXT_DEFUSE " + targetDisplayName;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTObject(2);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(target.GetObject());
		if(charge && charge.GetChargeState() == HDSN_BreachingChargeStates.IS_ARMED)
		{
			string itemName = GetItemName(item);

			if(itemName != "Unarmed")
			{
				if(item.IsDamageDestroyed())
				{
					return false;
				}
			}

			if(charge.GetChargeParams().CanBeDefusedBy(itemName))
			{
				targetDisplayName = charge.GetDisplayName();//charge.GetType();
				return true;
			}
		}

		return false;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		if(!action_data.m_Target)
		{
			return;
		}

		HDSN_BreachingChargeBase charge = HDSN_BreachingChargeBase.Cast(action_data.m_Target.GetObject());
		if(charge)
		{
			charge.DefuseBreachingCharge(action_data.m_Player);

			if(action_data.m_MainItem && GetItemName(action_data.m_MainItem) != "Unarmed")
			{
				action_data.m_MainItem.DecreaseHealth(charge.GetToolDamageOnDefuse(), false);
			}
		}
	}

	string GetItemName(ItemBase item)
	{
		if(!item)
		{
			return "Unarmed";
		}

		return item.GetType();
	}
}
