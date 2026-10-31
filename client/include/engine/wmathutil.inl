// ray against an axis aligned box grown by radius: fraction of the ray at the
// hit (1.0f = no hit); the axes are tested in x, z, y order
inline float __fastcall RayIntersectAABB(const Waabb& box, const WVector& start,
	const WVector& ray, WPlane* result, float radius)
{
	for (int i = 0; i < 3; i = (i == 0) ? 2 : ((i == 1) ? 3 : 1))
	{
		if (Wabs(ray.p[i]) < g_EPSILON)
			continue;

		float edge;
		if (ray.p[i] > 0.0f)
		{
			if (box.max.p[i] + radius < start.p[i])
				return 1.0f;
			edge = box.min.p[i] - radius;
			if (start.p[i] + ray.p[i] < edge)
				return 1.0f;
			if (start.p[i] > edge)
				continue;
		}
		else
		{
			if (box.min.p[i] - radius > start.p[i])
				return 1.0f;
			edge = box.max.p[i] + radius;
			if (start.p[i] + ray.p[i] > edge)
				return 1.0f;
			if (start.p[i] < edge)
				continue;
		}

		float t = (edge - start.p[i]) / ray.p[i];
		int j;
		for (j = 0; j < 3; ++j)
		{
			if (j == i)
				continue;
			float hit = start.p[j] + t * ray.p[j];
			if (hit < box.min.p[j] - radius || hit > box.max.p[j] + radius)
				break;
		}
		if (j == 3)
		{
			if (result)
			{
				for (int k = 0; k < 3; ++k)
				{
					if (k != i)
					{
						result->normal.p[k] = 0.0f;
					}
					else
					{
						result->normal.p[k] = ray.p[k] < 0.0f ? 1.0f : -1.0f;
						result->dis = -(
							(start.p[k] + t * ray.p[k]) * result->normal.p[k]);
					}
				}
			}
			return t;
		}
	}
	return 1.0f;
}
