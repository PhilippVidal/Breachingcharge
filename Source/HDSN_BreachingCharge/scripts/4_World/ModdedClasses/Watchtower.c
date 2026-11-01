modded class Watchtower
{
	int hdsn_watchtowerWallHealth[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

	void SetWatchtowerWallHealthHDSN(int value, int level, int wall)
	{
		int index = (level - 1) * 3 + wall - 1;

		if(index < 9 && index >= 0)
		{
			hdsn_watchtowerWallHealth[index] = value;
		}
	}

	int GetWatchtowerWallHealthHDSN(int level, int wall)
	{
		int index = (level - 1) * 3 + wall - 1;

		if(index < 9 && index >= 0)
		{
			return hdsn_watchtowerWallHealth[index];
		}

		return 0;
	}
}
