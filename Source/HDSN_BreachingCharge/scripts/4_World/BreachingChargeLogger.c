enum HDSN_ChargeEventTypes
{
	PLACED,
	ARMED,
	DEFUSED,
	EXPLODED,
	TARGETDESTROYED
}

class HDSN_BreachingChargeLogger
{
	protected static ref HDSN_BreachingChargeLogger instance;

	protected string configPath = "$profile:/BreachingCharge/logs/BreachingChargeLog_";
	protected string configModDirectory = "$profile:/BreachingCharge";
	protected string configLogsDirectory = "$profile:/BreachingCharge/logs";

	protected string logFileName;
	protected string logFilePath;
	protected bool createLogs;

	void HDSN_BreachingChargeLogger()
	{
		Print("[Breachingcharge]	Breachingcharge-Logger created!");

		logFileName = GetCurrentDateAndTimePathFriendly() + ".txt";
		logFilePath = configPath + logFileName;
		createLogs = GetConfigManagerInstance().GetConfigData().CreateLogs;

		Print("[Breachingcharge]	Log-File for this session will be called: BreachingChargeLog_" + logFileName);
	}

	HDSN_BreachingChargeConfigManager GetConfigManagerInstance()
	{
		return HDSN_BreachingChargeConfigManager.GetInstance();
	}

	void LogMessage(string message)
	{
		CheckForDirectories();

		FileHandle fileHandle = OpenFile(logFilePath, FileMode.APPEND);

		if(fileHandle != 0)
		{
			FPrintln(fileHandle,"[" + GetCurrentDateAndTime() + "]	" + message);

			CloseFile(fileHandle);
		}
	}

	void LogEvent(int eventType, PlayerBase placingPlayer, Object charge, string targetType, PlayerBase defusingPlayer = null)
	{
		if(createLogs)
		{
			string dateAndTime = "CAN'T GET TIME";
			string placingPlayerName = "CAN'T GET PLAYER NAME";
			string placingPlayerSteam64 = "CAN'T GET PLAYER STEAM64";
			string chargeClassName = "CAN'T GET CHARGE TYPE";
			string chargePosition = "CAN'T GET CHARGE POSITION";
			string target = "CAN'T GET TARGET TYPE";
			string defusingPlayerName = "CAN'T GET PLAYER NAME";
			string defusingPlayerSteam64 = "CAN'T GET PLAYER STEAM64";

			dateAndTime = GetCurrentDateAndTime();
			target = targetType;

			if(placingPlayer)
			{
				placingPlayerName = placingPlayer.GetIdentity().GetName();
				placingPlayerSteam64 = placingPlayer.GetIdentity().GetPlainId();
			}

			if(charge)
			{
				chargeClassName = charge.ClassName();
				chargePosition = charge.GetPosition().ToString();
			}

			if(defusingPlayer)
			{
				defusingPlayerName = defusingPlayer.GetIdentity().GetName();
				defusingPlayerSteam64 = defusingPlayer.GetIdentity().GetPlainId();
			}

			string logMessage;
			string eventDescription;
			string chargeInfo = chargeClassName + ", Position: " + chargePosition + ", Target: " + target;
			string placingPlayerInfo = "Player: " + placingPlayerName + " [" + placingPlayerSteam64 + "]";
			string spacer = " | ";

			if(eventType == HDSN_ChargeEventTypes.PLACED)
			{
				eventDescription = "[PLACED]	[" + dateAndTime + "]	" + "A charge has been placed!	";
				logMessage = eventDescription + spacer + chargeInfo + spacer + placingPlayerInfo;
				LogMessage(logMessage);
			}
			else if(eventType == HDSN_ChargeEventTypes.ARMED)
			{
				eventDescription = "[ARMED]		[" + dateAndTime + "]	" + "A charge has been armed!	";
				logMessage = eventDescription + spacer + chargeInfo + spacer + placingPlayerInfo;
				LogMessage(logMessage);
			}
			else if(eventType == HDSN_ChargeEventTypes.DEFUSED)
			{
				eventDescription = "[DEFUSED]	[" + dateAndTime + "]	" + "A charge has been defused!	";
				string defusingPlayerInfo = "Defusing Player: " + defusingPlayerName + " [" + defusingPlayerSteam64 + "]";
				logMessage = eventDescription + spacer + chargeInfo + spacer + defusingPlayerInfo + spacer + placingPlayerInfo;
				LogMessage(logMessage);
			}
			else if(eventType == HDSN_ChargeEventTypes.EXPLODED)
			{
				chargeInfo += ", Damage done: " + HDSN_BreachingChargeBase.Cast(charge).GetChargeDamage().ToString();
				eventDescription = "[EXPLODED]	[" + dateAndTime + "]	" + "A charge has exploded!		";
				logMessage = eventDescription + spacer + chargeInfo + spacer + placingPlayerInfo;
				LogMessage(logMessage);
			}
			else if(eventType == HDSN_ChargeEventTypes.TARGETDESTROYED)
			{
				eventDescription = "[DESTROYED]	[" + dateAndTime + "]	" + "A target was destroyed!	";
				logMessage = eventDescription + spacer + chargeInfo + spacer + placingPlayerInfo;
				LogMessage(logMessage);
			}
		}
	}

	void CheckForDirectories()
	{
		if(!FileExist(configLogsDirectory))
		{
			if(!FileExist(configModDirectory))
			{
				MakeDirectory(configModDirectory);
			}

			MakeDirectory(configLogsDirectory);
		}
	}

	string GetCurrentTime()
	{
		int hours, minutes, seconds;
		string h, m, s;
		GetHourMinuteSecond(hours, minutes, seconds);

		h = hours.ToString();
		m = minutes.ToString();
		s = seconds.ToString();
		if(hours < 10)
		{
			h = "0" + h;
		}

		if(minutes < 10)
		{
			m = "0" + m;
		}

		if(seconds < 10)
		{
			s = "0" + s;
		}

		return h + "-" + m + "-" + s;
	}

	string GetCurrentDate()
	{
		int days, months, years;
		string d, m, y;
		GetYearMonthDay(years, months, days);

		d = days.ToString();
		m = months.ToString();
		y = years.ToString();
		if(days < 10)
		{
			d = "0" + d;
		}

		if(months < 10)
		{
			m = "0" + m;
		}

		if(years < 10)
		{
			y = "0" + y;
		}

		return d + "-" + m + "-" + y;
	}

	string GetCurrentDateAndTimePathFriendly()
	{
		return GetCurrentDate() + "_" + GetCurrentTime();
	}

	string GetCurrentDateAndTime()
	{
		string date = GetCurrentDate();
		string time = GetCurrentTime();
		date.Replace("-", "/");
		time.Replace("-", ":");

		return date + "-" + time;
	}

	static HDSN_BreachingChargeLogger GetInstance()
	{
		return instance;
	}

	static void ClearInstance()
	{
		delete instance;
	}

	static void CreateInstance()
	{
		ClearInstance();
		instance = new HDSN_BreachingChargeLogger;
	}
}
