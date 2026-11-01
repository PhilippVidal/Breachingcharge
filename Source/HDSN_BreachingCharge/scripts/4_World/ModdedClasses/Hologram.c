modded class Hologram
{
	protected HDSN_ActionDeployBreachingCharge hdsn_actionDeployCharge;

	override void UpdateHologram(float timeslice)
	{
		if(m_Projection.IsInherited(HDSN_BreachingChargeBase))
		{
			if ( !m_Parent )
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(m_Player.TogglePlacingLocal);

				return;
			}

			if ( m_Player.IsSwimming() || m_Player.IsClimbingLadder() || m_Player.IsRaised() )
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(m_Player.TogglePlacingLocal);

				return;
			}

			EvaluateCollision();
			RefreshTrigger();
			CheckPowerSource();
			RefreshVisual();

			if (!GetUpdatePosition())
			{
				return;
			}

			if(HDSN_BreachingChargeBase.Cast(m_Parent).HasTarget())
			{
				if(hdsn_actionDeployCharge)
				{
					m_Projection.SetPosition(hdsn_actionDeployCharge.GetContactPos());

					vector chargeDirection = hdsn_actionDeployCharge.GetContactDir().Normalized();
					float angle = HDSN_Functions.ViewDirectionToChargeZAngle(GetGame().GetCurrentCameraDirection());

					if(HDSN_Functions.IsRoughlySameVector(chargeDirection, Vector(0.0, 1.0, 0.0)))
					{
						m_Projection.SetOrientation(Vector(0.00, 90.00, angle));
					}
					else if(HDSN_Functions.IsRoughlySameVector(chargeDirection, Vector(0.0, -1.0, 0.0)))
					{
						m_Projection.SetOrientation(Vector(0.00, -90.0, -angle));
					}
					else
					{
						m_Projection.SetOrientation(hdsn_actionDeployCharge.GetContactDir().VectorToAngles());
					}
				}
			}
			else
			{
				SetProjectionPosition(vector.Zero);
			}
		}
		else
		{
			super.UpdateHologram(timeslice);
		}
	}

	void SetActionDeployBreachingCharge(HDSN_ActionDeployBreachingCharge action)
	{
		hdsn_actionDeployCharge = action;
	}
}
