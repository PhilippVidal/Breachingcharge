class HDSN_BreachingChargeLight : PointLightBase
{

	protected float lightRadius;
	protected float lightBrightness;
	protected vector colorStart;
	protected vector colorHalfway;
	protected vector colorEnd;
	protected bool isOn;
	protected HDSN_BreachingChargeBase parentCharge;
	protected bool isInitialized;

	void HDSN_BreachingChargeLight()
	{
		isInitialized = false;
	}

	void InitChargeLight()
	{
		parentCharge = HDSN_BreachingChargeBase.Cast(GetParent());
		if(parentCharge)
		{
			lightRadius = parentCharge.GetChargeParams().LightRadius;
			lightBrightness = parentCharge.GetChargeParams().LightBrightness;
			colorStart = parentCharge.GetChargeParams().LightColorStart;
			colorHalfway = parentCharge.GetChargeParams().LightColorHalfway;
			colorEnd = parentCharge.GetChargeParams().LightColorEnd;

			SetVisibleDuringDaylight(true);
			SetFlareVisible(true);
			FadeIn(0);
			SetFadeOutTime(0);

			isInitialized = true;

			ToggleChargeLightOn();
		}
		else
		{
			Print("[Breachingcharge ERROR]		Trying to attach a HDSN_BreachingChargeLightC to an object that is not inherited from HDSN_BreachingChargeBase! Can't initialize charge-light properly!");
		}
	}

	void ToggleChargeLight()
	{
		if(isOn)
		{
			ToggleChargeLightOff();
		}
		else
		{
			ToggleChargeLightOn();
		}
	}

	void ToggleChargeLightOn()
	{
		if(!isInitialized)
		{
			InitChargeLight();
		}

		isOn = true;
		SetRadiusTo(lightRadius);
		SetBrightnessTo(lightBrightness);

		vector lightColor = GetColorToSet();

		SetDiffuseColor(lightColor[0], lightColor[1], lightColor[2]);
		SetAmbientColor(lightColor[0], lightColor[1], lightColor[2]);
	}

	void ToggleChargeLightOff()
	{
		if(!isInitialized)
		{
			InitChargeLight();
		}

		isOn = false;
		SetRadiusTo(0);
		SetBrightnessTo(0);
	}

	vector GetColorToSet()
	{
		float timeLeft = parentCharge.GetChargeTimeLeft();
		float endTime = parentCharge.GetBeepingSoundEndTime();

		if(timeLeft <= endTime)
		{
			return colorEnd;
		}

		float percentage = timeLeft / parentCharge.GetTimeToExplode();

		if(percentage <= 0.50)
		{
			return colorHalfway;
		}

		return colorStart;
	}

	bool IsChargeLightOn()
	{
		return isOn;
	}
}
