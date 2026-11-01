class HDSN_Functions
{
	//Two raycasts because RayCastBullet is broken -> object being hit is always null (at least at the time of writing this (march 2021), once fixed only a single raycast should be enough)
	//RaycastRV checks for hit object type, RaycastBullet gives position and mesh-normal
	//required because RaycastRV's contact direction always points towards the player and RayCastBullet's contact position is more accurate
	static bool CustomRaycast(vector startPosition, vector endPosition, out vector contactPosition, out vector contactDirection, out int contactComponent, out Object hitObject, Object with, Object ignore)
	{
		contactPosition = vector.Zero;
		contactDirection = vector.Zero;
		contactComponent = 0;
		hitObject = null;

		set<Object> results = new set<Object>();
		if(DayZPhysics.RaycastRV(startPosition, endPosition, contactPosition, contactDirection, contactComponent, results, with, ignore, true))
		{
			hitObject = results[0];
			PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.BUILDING|PhxInteractionLayers.ITEM_LARGE|PhxInteractionLayers.FENCE;
			Object hitObj;
			vector hitPos, hitDir;
			float hitFraction;

			if(DayZPhysics.RayCastBullet(startPosition, endPosition, collisionLayerMask, ignore, hitObj, hitPos, hitDir, hitFraction))
			{
				contactPosition = hitPos;
				contactDirection = hitDir;
			}


			//Fix for charges sometimes getting placed at the origin position of target objects
			//example => BBP walls
			//Temporary fix
			if(hitObject.GetPosition() == contactPosition)
			{
				return false;
			}

			return true;
		}

		return false;
	}

	static bool CustomRaycastExp(vector startPosition, vector endPosition, out vector contactPosition, out vector contactDirection, out int contactComponent, out Object hitObject, Object with, Object ignore, float secondRayDistance = 0.1)
	{
		contactPosition = vector.Zero;
		contactDirection = vector.Zero;

		PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.BUILDING | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE;
		Object hitObj;
		vector hitPos, hitDir;
		float hitFraction;

		if(!DayZPhysics.RayCastBullet(startPosition, endPosition, collisionLayerMask, ignore, hitObject, hitPos, hitDir, hitFraction))
			return false;

		contactPosition = hitPos;
		contactDirection = hitDir;

		contactComponent = 0;
		set<Object> results = new set<Object>();
		if(!DayZPhysics.RaycastRV(contactPosition, contactPosition + contactDirection * -secondRayDistance, hitPos, hitDir, contactComponent, results, with, ignore, true))
			return false;

		hitObj = results[0];
		if(!hitObj)
			return false;

		hitObject = hitObj;
		return true;
	}

	static float ViewDirectionToChargeZAngle(vector viewDirection)
	{
		float aX = viewDirection[0];
		float aY = viewDirection[2];
		vector defaultVector = Vector(0, 0, -1);
		float bX = defaultVector[0];
		float bY = defaultVector[2];
		float rad = Math.Atan2(aY, aX) - Math.Atan2(bY, bX);
		return rad * (180.0 / Math.PI);
	}

	static vector RotateAroundAxis(vector pos, vector axis, float angle)
	{
		float cosAngle = Math.Cos(angle);
		float sinAngle = Math.Sin(angle);

		return (pos * cosAngle) + ((axis * pos) * sinAngle) + (axis * vector.Dot(axis, pos)) * (1 - cosAngle);
	}

	static bool IsInRectangle2D(vector center, float widthX, float widthY, vector targetPosition)
	{
		Vector2 centerPoint = new Vector2(center[0], center[2]);
		Vector2 position = new Vector2(targetPosition[0], targetPosition[2]);

		bool isInsideX = position.x >= (centerPoint.x - widthX) && position.x <= (centerPoint.x + widthX);
		bool isInsideY = position.y >= (centerPoint.y - widthY) && position.y <= (centerPoint.y + widthY);

		if(isInsideX && isInsideY)
		{
			return true;
		}

		return false;
	}

	static vector CrossProduct(vector a, vector b)
	{
		float x = a[1] * b[2] - a[2] * b[1];
		float y = a[2] * b[0] - a[0] * b[2];
		float z = a[0] * b[1] - a[1] * b[0];

		return Vector(x, y, z);
	}

	static float AngleBetweenVectors(vector u, vector v)
	{
		float a = u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
		float u_Squared = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		float v_Squared = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		float b = Math.Sqrt(u_Squared * v_Squared);

		return Math.Acos(a/b) * (360.0 / (2 * Math.PI));
	}

	static float AnglesBetweenVectorsAlongAxis(vector u, vector v, vector axis)
	{
		vector uProjection = u - vector.Dot(u, axis) * axis;
		vector vProjection = v - vector.Dot(v, axis) * axis;

		return AngleBetweenVectors(uProjection, vProjection);
	}

	static bool IsRoughlyEqual(float a, float b)
	{
		float c = a - b;

		if(c < 0.0)
		{
			c *= -1.0;
		}

		if(c <= 0.01)
		{
			return true;
		}

		return false;
	}

	static bool IsRoughlySameVector(vector a, vector b)
	{
		bool x = IsRoughlyEqual(a[0], b[0]);
		bool y = IsRoughlyEqual(a[1], b[1]);
		bool z = IsRoughlyEqual(a[2], b[2]);

		if(x && y && z)
		{
			return true;
		}

		return false;
	}

	static bool IsObjectFacingPlayer(PlayerBase player, Object object)
	{
		vector playerDirection = player.GetDirection().Normalized();
		playerDirection[1] = 0;

		vector objectDirection = object.GetDirection().Normalized();
		objectDirection[1] = 0;

		if(objectDirection.Length() != 0)
		{
			float angle = Math.Acos(playerDirection * objectDirection);

			if(angle >= 1.5708)
			{
				return true;
			}

		}

		return false;
	}

	static float GetDifference(float a, float b)
	{
		float c = a - b;

		if(c < 0)
		{
			c *= -1.0;
		}

		return c;
	}
}
