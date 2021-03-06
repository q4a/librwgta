#include "III.h"

#include "DebugDraw.h"


enum Direction
{
	DIR_X_POS,
	DIR_X_NEG,
	DIR_Y_POS,
	DIR_Y_NEG,
	DIR_Z_POS,
	DIR_Z_NEG,
};

eLevelName CCollision::ms_collisionInMemory;
CLinkList<CColModel*> CCollision::ms_colModelCache;

void
CCollision::Init(void)
{
	ms_colModelCache.Init(NUMCOLCACHELINKS);
	ms_collisionInMemory = LEVEL_NONE;
}

void
CCollision::Update(void)
{
	CVector pos = FindPlayerCoors();
	eLevelName level = CTheZones::m_CurrLevel;
	bool changeLevel = false;

	// hardcode a level if there are no zones
	if(level == LEVEL_NONE){
		if(CGame::currLevel == LEVEL_INDUSTRIAL &&
		   pos.x < 400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else if(CGame::currLevel == LEVEL_SUBURBAN &&
		         pos.x > -450.0f && pos.y < -1400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else{
			if(pos.x > 800.0f){
				level = LEVEL_INDUSTRIAL;
				changeLevel = true;
			}else if(pos.x < -800.0f){
				level = LEVEL_SUBURBAN;
				changeLevel = true;
			}
		}
	}
	if(level != LEVEL_NONE && level != CGame::currLevel){
		debug("changing level %d -> %d\n", CGame::currLevel, level);
		CGame::currLevel = level;
	}
	if(ms_collisionInMemory != CGame::currLevel)
		LoadCollisionWhenINeedIt(changeLevel);
	CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
}

void
CCollision::LoadCollisionWhenINeedIt(bool changeLevel)
{
	eLevelName level;
	level = LEVEL_NONE;
	if(!changeLevel){
		//assert(0 && "unimplemented");
	}

	if(level != CGame::currLevel || changeLevel){
		CTimer::Stop();
		CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
		CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
		CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
		ms_collisionInMemory = CGame::currLevel;
		CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		CStreaming::RequestBigBuildings(CGame::currLevel);
		CStreaming::LoadAllRequestedModels();
		CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
		CTimer::Update();
	}
}


//
// Test
//


bool
CCollision::TestSphereSphere(const CColSphere &s1, const CColSphere &s2)
{
	float d = s1.radius + s2.radius;
	return (s1.center - s2.center).MagnitudeSqr() < d*d;
}

bool
CCollision::TestSphereBox(const CColSphere &sph, const CColBox &box)
{
	if(sph.center.x + sph.radius < box.min.x) return false;
	if(sph.center.x - sph.radius > box.max.x) return false;
	if(sph.center.y + sph.radius < box.min.y) return false;
	if(sph.center.y - sph.radius > box.max.y) return false;
	if(sph.center.z + sph.radius < box.min.z) return false;
	if(sph.center.z - sph.radius > box.max.z) return false;
	return true;
}

bool
CCollision::TestLineBox(const CColLine &line, const CColBox &box)
{
	float t, x, y, z;
	// If either line point is in the box, we have a collision
	if(line.p0.x > box.min.x && line.p0.x < box.max.x &&
	   line.p0.y > box.min.y && line.p0.y < box.max.y &&
	   line.p0.z > box.min.z && line.p0.z < box.max.z)
		return true;
	if(line.p1.x > box.min.x && line.p1.x < box.max.x &&
	   line.p1.y > box.min.y && line.p1.y < box.max.y &&
	   line.p1.z > box.min.z && line.p1.z < box.max.z)
		return true;

	// check if points are on opposite sides of min x plane
	if((box.min.x - line.p1.x) * (box.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// same test with max x plane
	if((line.p1.x - box.max.x) * (line.p0.x - box.max.x) < 0.0f){
		t = (line.p0.x - box.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min y plne
	if((box.min.y - line.p0.y) * (box.min.y - line.p1.y) < 0.0f){
		t = (box.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// max y plane
	if((line.p0.y - box.max.y) * (line.p1.y - box.max.y) < 0.0f){
		t = (line.p0.y - box.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min z plne
	if((box.min.z - line.p0.z) * (box.min.z - line.p1.z) < 0.0f){
		t = (box.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}

	// max z plane
	if((line.p0.z - box.max.z) * (line.p1.z - box.max.z) < 0.0f){
		t = (line.p0.z - box.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}
	return false;
}

bool
CCollision::TestVerticalLineBox(const CColLine &line, const CColBox &box)
{
	if(line.p0.x <= box.min.x) return false;
	if(line.p0.y <= box.min.y) return false;
	if(line.p0.x >= box.max.x) return false;
	if(line.p0.y >= box.max.y) return false;
	if(line.p0.z < line.p1.z){
		if(line.p0.z > box.max.z) return false;
		if(line.p1.z < box.min.z) return false;
	}else{
		if(line.p1.z > box.max.z) return false;
		if(line.p0.z < box.min.z) return false;
	}
	return true;
}

bool
CCollision::TestLineTriangle(const CColLine &line, const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / DotProduct(line.p1 - line.p0, normal);
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];
	CVector2D vec1, vec2, vec3, vect;

	// We do the test in 2D. With the plane direction we
	// can figure out how to project the vectors.
	// normal = (c-a) x (b-a)
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	// This is our triangle:
	// 3-------2
	//  \  P  /
	//   \   /
	//    \ /
	//     1
	// We can use the "2d cross product" to check on which side
	// a vector is of another. Test is true if point is inside of all edges.
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	return true;
}

// Test if line segment intersects with sphere.
// If the first point is inside the sphere this test does not register a collision!
// The code is reversed from the original code and rather ugly, see Process for a clear version.
// TODO: actually rewrite this mess
bool
CCollision::TestLineSphere(const CColLine &line, const CColSphere &sph)
{
	CVector v01 = line.p1 - line.p0;	// vector from p0 to p1
	CVector v0c = sph.center - line.p0;	// vector from p0 to center
	float linesq = v01.MagnitudeSqr();
	// I leave in the strange -2 factors even though they serve no real purpose
	float projline = -2.0f * DotProduct(v01, v0c);	// project v0c onto line
	// Square of tangent from p0 multiplied by line length so we can compare with projline.
	// The length of the tangent would be this: sqrt((c-p0)^2 - r^2).
	// Negative if p0 is inside the sphere! This breaks the test!
	float tansq = 4.0f * linesq *
		(sph.center.MagnitudeSqr() - 2.0f*DotProduct(sph.center, line.p0) + line.p0.MagnitudeSqr() - sph.radius*sph.radius);
	float diffsq = projline*projline - tansq;
	// if diffsq < 0 that means the line is a passant, so no intersection
	if(diffsq < 0.0f)
		return false;
	// projline (negative in GTA for some reason) is the point on the line
	// in the middle of the two intersection points (startin from p0).
	// sqrt(diffsq) somehow works out to be the distance from that
	// midpoint to the intersection points.
	// So subtract that and get rid of the awkward scaling:
	float f = (-projline - sqrt(diffsq)) / (2.0f*linesq);
	// f should now be in range [0, 1] for [p0, p1]
	return f >= 0.0f && f <= 1.0f;
}

bool
CCollision::TestSphereTriangle(const CColSphere &sphere,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	// If sphere and plane don't intersect, no collision
	if(fabs(plane.CalcPoint(sphere.center)) > sphere.radius)
		return false;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// calculate two orthogonal basis vectors for the triangle
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector vec1 = CrossProduct(vec2, plane.normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = sphere.center - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	if(testcase == 1){
		// closest to a vertex
		if(insideAB) dist = (sphere.center - vc).Magnitude();
		else if(insideAC) dist = (sphere.center - vb).Magnitude();
		else if(insideBC) dist = (sphere.center - va).Magnitude();
		else assert(0);
	}else if(testcase == 2){
		// closest to an edge
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center);
		else assert(0);
	}else if(testcase == 3){
		// center is in triangle
		return true;
	}else
		assert(0);	// front fell off

	return dist < sphere.radius;
}

bool
CCollision::TestLineOfSight(CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSurf78)
{
	static CMatrix matTransform;
	CColLine newline;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	newline.Set(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSurf78 || model.spheres[i].surface != 7 && model.spheres[i].surface != 8)
			if(TestLineSphere(newline, model.spheres[i]))
				return true;

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSurf78 || model.boxes[i].surface != 7 && model.boxes[i].surface != 8)
			if(TestLineBox(newline, model.boxes[i]))
				return true;

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSurf78 || model.triangles[i].surface != 7 && model.triangles[i].surface != 8)
			if(TestLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i]))
				return true;

	return false;
}


//
// Process
//

// For Spheres mindist is the squared distance to its center
// For Lines mindist is between [0,1]

bool
CCollision::ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq)
{
	CVector dist = s1.center - s2.center;
	float d = dist.Magnitude() - s2.radius;	// distance from s1's center to s2
	float dc = d < 0.0f ? 0.0f : d;		// clamp to zero, i.e. if s1's center is inside s2
	// no collision if sphere is not close enough
	if(mindistsq <= dc*dc || s1.radius <= dc)
		return false;
	dist.Normalise();
	point.point = s1.center - dist*dc;
	point.normal = dist;
	point.surfaceA = s1.surface;
	point.pieceA = s1.piece;
	point.surfaceB = s2.surface;
	point.pieceB = s2.piece;
	point.depth = s1.radius - d;	// sphere overlap
	mindistsq = dc*dc;		// collision radius
	return true;
}

bool
CCollision::ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq)
{
	CVector p;
	CVector dist;

	// GTA's code is too complicated, uses a huge 3x3x3 if statement
	// we can simplify the structure a lot

	// first make sure we have a collision at all
	if(sph.center.x + sph.radius < box.min.x) return false;
	if(sph.center.x - sph.radius > box.max.x) return false;
	if(sph.center.y + sph.radius < box.min.y) return false;
	if(sph.center.y - sph.radius > box.max.y) return false;
	if(sph.center.z + sph.radius < box.min.z) return false;
	if(sph.center.z - sph.radius > box.max.z) return false;

	// Now find out where the sphere center lies in relation to all the sides
	int xpos = sph.center.x < box.min.x ? 1 :
	           sph.center.x > box.max.x ? 2 :
	           0;
	int ypos = sph.center.y < box.min.y ? 1 :
	           sph.center.y > box.max.y ? 2 :
	           0;
	int zpos = sph.center.z < box.min.z ? 1 :
	           sph.center.z > box.max.z ? 2 :
	           0;

	if(xpos == 0 && ypos == 0 && zpos == 0){
		// sphere is inside the box
		p = (box.min + box.max)*0.5f;

		dist = sph.center - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			point.normal = dist * (1.0f/sqrt(lensq));
			point.point = sph.center - point.normal;
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;

			// find absolute distance to the closer side in each dimension
			float dx = dist.x > 0.0f ?
				box.max.x - sph.center.x :
				sph.center.x - box.min.x;
			float dy = dist.y > 0.0f ?
				box.max.y - sph.center.y :
				sph.center.y - box.min.y;
			float dz = dist.z > 0.0f ?
				box.max.z - sph.center.z :
				sph.center.z - box.min.z;
			// collision depth is maximum of that:
			if(dx > dy && dx > dz)
				point.depth = dx;
			else if(dy > dz)
				point.depth = dy;
			else
				point.depth = dz;
			return true;
		}
	}else{
		// sphere is outside.
		// closest point on box:
		p.x = xpos == 1 ? box.min.x :
		      xpos == 2 ? box.max.x :
		      sph.center.x;
		p.y = ypos == 1 ? box.min.y :
		      ypos == 2 ? box.max.y :
		      sph.center.y;
		p.z = zpos == 1 ? box.min.z :
		      zpos == 2 ? box.max.z :
		      sph.center.z;

		dist = sph.center - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			float len = sqrt(lensq);
			point.point = p;
			point.normal = dist * (1.0f/len);
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
			point.depth = sph.radius - len;
			mindistsq = lensq;
			return true;
		}
	}
	return false;
}

bool
CCollision::ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist)
{
	float mint, t, x, y, z;
	CVector normal;
	CVector p;

	mint = 1.0f;
	// check if points are on opposite sides of min x plane
	if((box.min.x - line.p1.x) * (box.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.min.x, y, z);
					normal = CVector(-1.0f, 0.0f, 0.0f);
				}
		}
	}

	// max x plane
	if((line.p1.x - box.max.x) * (line.p0.x - box.max.x) < 0.0f){
		t = (line.p0.x - box.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.max.x, y, z);
					normal = CVector(1.0f, 0.0f, 0.0f);
				}
		}
	}

	// min y plne
	if((box.min.y - line.p0.y) * (box.min.y - line.p1.y) < 0.0f){
		t = (box.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.min.y, z);
					normal = CVector(0.0f, -1.0f, 0.0f);
				}
		}
	}

	// max y plane
	if((line.p0.y - box.max.y) * (line.p1.y - box.max.y) < 0.0f){
		t = (line.p0.y - box.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.max.y, z);
					normal = CVector(0.0f, 1.0f, 0.0f);
				}
		}
	}

	// min z plne
	if((box.min.z - line.p0.z) * (box.min.z - line.p1.z) < 0.0f){
		t = (box.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.min.z);
					normal = CVector(0.0f, 0.0f, -1.0f);
				}
		}
	}

	// max z plane
	if((line.p0.z - box.max.z) * (line.p1.z - box.max.z) < 0.0f){
		t = (line.p0.z - box.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.max.z);
					normal = CVector(0.0f, 0.0f, 1.0f);
				}
		}
	}

	if(mint >= mindist)
		return false;

	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = box.surface;
	point.pieceB = box.piece;
	mindist = mint;

	return true;
}

// If line.p0 lies inside sphere, no collision is registered.
bool
CCollision::ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist)
{
	CVector v01 = line.p1 - line.p0;
	CVector v0c = sphere.center - line.p0;
	float linesq = v01.MagnitudeSqr();
	// project v0c onto v01, scaled by |v01| this is the midpoint of the two intersections
	float projline = DotProduct(v01, v0c);
	// tangent of p0 to sphere, scaled by linesq just like projline^2
	float tansq = (v0c.MagnitudeSqr() - sphere.radius*sphere.radius) * linesq;
	// this works out to be the square of the distance between the midpoint and the intersections
	float diffsq = projline*projline - tansq;
	// no intersection
	if(diffsq < 0.0f)
		return false;
	// point of first intersection, in range [0,1] between p0 and p1
	float t = (projline - sqrt(diffsq)) / linesq;
	// if not on line or beyond mindist, no intersection
	if(t < 0.0f || t > 1.0f || t >= mindist)
		return false;
	point.point = line.p0 + v01*t;
	point.normal = point.point - sphere.center;
	point.normal.Normalise();
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = sphere.surface;
	point.pieceB = sphere.piece;
	mindist = t;
	return true;
}

bool
CCollision::ProcessVerticalLineTriangle(const CColLine &line,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist, CStoredCollPoly *poly)
{
	float t;
	CVector normal;

	const CVector &p0 = line.p0;
	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// early out bound rect test
	if(p0.x < va.x && p0.x < vb.x && p0.x < vc.x) return false;
	if(p0.x > va.x && p0.x > vb.x && p0.x > vc.x) return false;
	if(p0.y < va.y && p0.y < vb.y && p0.y < vc.y) return false;
	if(p0.y > va.y && p0.y > vb.y && p0.y > vc.y) return false;

	plane.GetNormal(normal);
	// if points are on the same side, no collision
	if(plane.CalcPoint(p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	float h = (line.p1 - p0).z;
	t = -plane.CalcPoint(p0) / (h * normal.z);
	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	CVector p(p0.x, p0.y, p0.z + h*t);

	CVector2D vec1, vec2, vec3, vect;
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	if(poly){
		poly->verts[0] = va;
		poly->verts[1] = vb;
		poly->verts[2] = vc;
		poly->valid = true;
	}
	mindist = t;
	return true;
}

bool
CCollision::ProcessLineTriangle(const CColLine &line ,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist)
{
	float t;
	CVector normal;
	plane.GetNormal(normal);

	// if points are on the same side, no collision
	if(plane.CalcPoint(line.p0) * plane.CalcPoint(line.p1) > 0.0f)
		return false;

	// intersection parameter on line
	t = -plane.CalcPoint(line.p0) / DotProduct(line.p1 - line.p0, normal);
	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	// find point of intersection
	CVector p = line.p0 + (line.p1-line.p0)*t;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];
	CVector2D vec1, vec2, vec3, vect;

	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	point.point = p;
	point.normal = normal;
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	mindist = t;
	return true;
}

bool
CCollision::ProcessSphereTriangle(const CColSphere &sphere,
	const CVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindistsq)
{
	// If sphere and plane don't intersect, no collision
	float planedist = plane.CalcPoint(sphere.center);
	float distsq = planedist*planedist;
	if(fabs(planedist) > sphere.radius || distsq > mindistsq)
		return false;

	const CVector &va = verts[tri.a];
	const CVector &vb = verts[tri.b];
	const CVector &vc = verts[tri.c];

	// calculate two orthogonal basis vectors for the triangle
	CVector normal;
	plane.GetNormal(normal);
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector vec1 = CrossProduct(vec2, normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = sphere.center - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	CVector p;
	if(testcase == 1){
		// closest to a vertex
		if(insideAB) p = vc;
		else if(insideAC) p = vb;
		else if(insideBC) p = va;
		else assert(0);
		dist = (sphere.center - p).Magnitude();
	}else if(testcase == 2){
		// closest to an edge
		if(!insideAB) dist = DistToLine(&va, &vb, &sphere.center, p);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphere.center, p);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphere.center, p);
		else assert(0);
	}else if(testcase == 3){
		// center is in triangle
		dist = fabs(planedist);
		p = sphere.center - normal*planedist;
	}else
		assert(0);	// front fell off

	if(dist >= sphere.radius || dist*dist >= mindistsq)
		return false;

	point.point = p;
	point.normal = sphere.center - p;
	point.normal.Normalise();
	point.surfaceA = sphere.surface;
	point.pieceA = sphere.piece;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	point.depth = sphere.radius - dist;
	mindistsq = dist*dist;
	return true;
}

bool
CCollision::ProcessLineOfSight(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSurf78)
{
	static CMatrix matTransform;
	CColLine newline;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	newline.Set(matTransform * line.p0, matTransform * line.p1);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSurf78 || model.spheres[i].surface != 7 && model.spheres[i].surface != 8)
			ProcessLineSphere(newline, model.spheres[i], point, coldist);

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSurf78 || model.boxes[i].surface != 7 && model.boxes[i].surface != 8)
			ProcessLineBox(newline, model.boxes[i], point, coldist);

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSurf78 || model.triangles[i].surface != 7 && model.triangles[i].surface != 8)
			ProcessLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist);

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		mindist = coldist;
		return true;
	}
	return false;
}

bool
CCollision::ProcessVerticalLine(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSurf78, CStoredCollPoly *poly)
{
	static CStoredCollPoly TempStoredPoly;
	CColLine newline;
	int i;

	// transform line to model space
	// Why does the game seem to do this differently than above?
	newline.Set(MultiplyInverse(matrix, line.p0), MultiplyInverse(matrix, line.p1));
	newline.p1.x = newline.p0.x;
	newline.p1.y = newline.p0.y;

	if(!TestVerticalLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++)
		if(!ignoreSurf78 || model.spheres[i].surface != 7 && model.spheres[i].surface != 8)
			ProcessLineSphere(newline, model.spheres[i], point, coldist);

	for(i = 0; i < model.numBoxes; i++)
		if(!ignoreSurf78 || model.boxes[i].surface != 7 && model.boxes[i].surface != 8)
			ProcessLineBox(newline, model.boxes[i], point, coldist);

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	for(i = 0; i < model.numTriangles; i++)
		if(!ignoreSurf78 || model.triangles[i].surface != 7 && model.triangles[i].surface != 8)
			ProcessVerticalLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist, &TempStoredPoly);

	if(coldist < mindist){
		point.point = matrix * point.point;
		point.normal = Multiply3x3(matrix, point.normal);
		if(poly && TempStoredPoly.valid){
			*poly = TempStoredPoly;
			poly->verts[0] = matrix * poly->verts[0];
			poly->verts[1] = matrix * poly->verts[1];
			poly->verts[2] = matrix * poly->verts[2];
		}
		mindist = coldist;
		return true;
	}
	return false;
}

// This checks model A's spheres and lines against model B's spheres, boxes and triangles.
// Returns the number of A's spheres that collide.
// Returned ColPoints are in world space.
// NB: lines do not seem to be supported very well, use with caution
int32
CCollision::ProcessColModels(const CMatrix &matrixA, CColModel &modelA,
	const CMatrix &matrixB, CColModel &modelB,
	CColPoint *spherepoints, CColPoint *linepoints, float *linedists)
{
	static int aSphereIndicesA[128];
	static int aLineIndicesA[16];
	static int aSphereIndicesB[128];
	static int aBoxIndicesB[32];
	static int aTriangleIndicesB[600];
	static bool aCollided[16];
	static CColSphere aSpheresA[128];
	static CColLine aLinesA[16];
	static CMatrix matAB, matBA;
	CColSphere s;
	int i, j;

	assert(modelA.numSpheres <= 128);
	assert(modelA.numLines <= 16);
	assert(modelB.numSpheres <= 128);
	assert(modelB.numBoxes <= 32);
	assert(modelB.numTriangles <= 600);

	// From model A space to model B space
	matAB = Invert(matrixB, matAB) * matrixA;

	CColSphere bsphereAB;	// bounding sphere of A in B space
	bsphereAB.Set(modelA.boundingSphere.radius, matAB * modelA.boundingSphere.center);
	if(!TestSphereBox(bsphereAB, modelB.boundingBox))
		return 0;
	// B to A space
	matBA = Invert(matrixA, matBA) * matrixB;

	// transform modelA's spheres and lines to B space
	for(i = 0; i < modelA.numSpheres; i++){
		CColSphere &s = modelA.spheres[i];
		aSpheresA[i].Set(s.radius, matAB * s.center, s.surface, s.piece);
	}
	for(i = 0; i < modelA.numLines; i++)
		aLinesA[i].Set(matAB * modelA.lines[i].p0, matAB * modelA.lines[i].p1);

	// Test them against model B's bounding volumes
	int numSpheresA = 0;
	int numLinesA = 0;
	for(i = 0; i < modelA.numSpheres; i++)
		if(TestSphereBox(aSpheresA[i], modelB.boundingBox))
			aSphereIndicesA[numSpheresA++] = i;
	// no actual check???
	for(i = 0; i < modelA.numLines; i++)
		aLineIndicesA[numLinesA++] = i;
	// No collision
	if(numSpheresA == 0 && numLinesA == 0)
		return 0;

	// Check model B against A's bounding volumes
	int numSpheresB = 0;
	int numBoxesB = 0;
	int numTrianglesB = 0;
	for(i = 0; i < modelB.numSpheres; i++){
		s.Set(modelB.spheres[i].radius, matBA * modelB.spheres[i].center);
		if(TestSphereBox(s, modelA.boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	}
	for(i = 0; i < modelB.numBoxes; i++)
		if(TestSphereBox(bsphereAB, modelB.boxes[i]))
			aBoxIndicesB[numBoxesB++] = i;
	CalculateTrianglePlanes(&modelB);
	for(i = 0; i < modelB.numTriangles; i++)
		if(TestSphereTriangle(bsphereAB, modelB.vertices, modelB.triangles[i], modelB.trianglePlanes[i]))
			aTriangleIndicesB[numTrianglesB++] = i;
	// No collision
	if(numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// We now have the collision volumes in A and B that are worth processing.

	// Process A's spheres against B's collision volumes
	int numCollisions = 0;
	for(i = 0; i < numSpheresA; i++){
		float coldist = 1.0e24f;
		bool hasCollided = false;

		for(j = 0; j < numSpheresB; j++)
			hasCollided |= ProcessSphereSphere(
				aSpheresA[aSphereIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numBoxesB; j++)
			hasCollided |= ProcessSphereBox(
				aSpheresA[aSphereIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numTrianglesB; j++)
			hasCollided |= ProcessSphereTriangle(
				aSpheresA[aSphereIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				spherepoints[numCollisions], coldist);
		if(hasCollided)
			numCollisions++;
	}
	for(i = 0; i < numCollisions; i++){
		spherepoints[i].point = matrixB * spherepoints[i].point;
		spherepoints[i].normal = Multiply3x3(matrixB, spherepoints[i].normal);
	}

	// And the same thing for the lines in A
	for(i = 0; i < numLinesA; i++){
		aCollided[i] = false;

		for(j = 0; j < numSpheresB; j++)
			aCollided[i] |= ProcessLineSphere(
				aLinesA[aLineIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numBoxesB; j++)
			aCollided[i] |= ProcessLineBox(
				aLinesA[aLineIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numTrianglesB; j++)
			aCollided[i] |= ProcessLineTriangle(
				aLinesA[aLineIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
	}
	for(i = 0; i < numLinesA; i++)
		if(aCollided[i]){
			j = aLineIndicesA[i];
			linepoints[j].point = matrixB * linepoints[j].point;
			linepoints[j].normal = Multiply3x3(matrixB, linepoints[j].normal);
		}

	return numCollisions;	// sphere collisions
}


//
// Misc
//

float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// Between 0 and len we're above the line.
	// if not, calculate distance to endpoint
	if(dot <= 0.0f)
		return (*point - *l0).Magnitude();
	if(dot >= lensq)
		return (*point - *l1).Magnitude();
	// distance to line
	return sqrt((*point - *l0).MagnitudeSqr() - dot*dot/lensq);
}

// same as above but also return the point on the line
float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// find out which point we're closest to
	if(dot <= 0.0f)
		closest = *l0;
	else if(dot >= lensq)
		closest = *l1;
	else
		closest = *l0 + (*l1 - *l0)*(dot/lensq);
	// this is the distance
	return (*point - closest).Magnitude();
}

void
CCollision::CalculateTrianglePlanes(CColModel *model)
{
	if(model->numTriangles == 0)
		return;

	CLink<CColModel*> *lptr;
	if(model->trianglePlanes){
		// re-insert at front so it's not removed again soon
		lptr = model->GetLinkPtr();
		lptr->Remove();
		ms_colModelCache.head.Insert(lptr);
	}else{
		lptr = ms_colModelCache.Insert(model);
		if(lptr == nil){
			// make room if we have to, remove last in list
			lptr = ms_colModelCache.tail.prev;
			lptr->item->RemoveTrianglePlanes();
			ms_colModelCache.Remove(lptr);
			// now this cannot fail
			lptr = ms_colModelCache.Insert(model);
			assert(lptr);
		}
		model->CalculateTrianglePlanes();
		model->SetLinkPtr(lptr);
	}
}

void
CCollision::DrawColModel(const CMatrix &mat, const CColModel &colModel)
{
	static rw::RGBA red = { 255, 0, 0, 255 };
	static rw::RGBA green = { 0, 255, 0, 255 };
	static rw::RGBA magenta = { 255, 0, 255, 255 };
	static rw::RGBA cyan = { 0, 255, 255, 255 };
	static rw::RGBA white = { 255, 255, 255, 255 };
	int i;
	CVector verts[3];

	rw::SetRenderState(rw::ZWRITEENABLE, 1);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);

	CDebugDraw::RenderWireBox(mat, colModel.boundingBox.min, colModel.boundingBox.max, red);

	for(i = 0; i < colModel.numSpheres; i++){
		CVector center = mat * colModel.spheres[i].center;
		CDebugDraw::RenderWireSphere(center, colModel.spheres[i].radius, magenta);
	}

	for(i = 0; i < colModel.numBoxes; i++)
		CDebugDraw::RenderWireBox(mat, colModel.boxes[i].min, colModel.boxes[i].max, white);

	for(i = 0; i < colModel.numLines; i++){
		verts[0] = colModel.lines[i].p0;
		verts[1] = colModel.lines[i].p1;
		rw::V3d::transformPoints((rw::V3d*)verts, (rw::V3d*)verts, 2, &mat.m_matrix);
		CDebugDraw::RenderLine(verts[0], verts[1], cyan, cyan);
	}

	for(i = 0; i < colModel.numTriangles; i++){
		verts[0] = colModel.vertices[colModel.triangles[i].a];
		verts[1] = colModel.vertices[colModel.triangles[i].b];
		verts[2] = colModel.vertices[colModel.triangles[i].c];
		rw::V3d::transformPoints((rw::V3d*)verts, (rw::V3d*)verts, 3, &mat.m_matrix);
		CDebugDraw::RenderWireTri(verts, green);
	}

	CDebugDraw::RenderAndEmptyRenderBuffer();

	rw::SetRenderState(rw::ZWRITEENABLE, 1);
	rw::SetRenderState(rw::ZTESTENABLE, 1);
	rw::SetRenderState(rw::VERTEXALPHA, 0);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
}

/*
 * ColModel code
 */

void
CColSphere::Set(float radius, const CVector &center, uint8 surf, uint8 piece)
{
	this->radius = radius;
	this->center = center;
	this->surface = surf;
	this->piece = piece;
}

void
CColSphere::Set(float radius, const CVector &center)
{
	this->radius = radius;
	this->center = center;
}

void
CColBox::Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece)
{
	this->min = min;
	this->max = max;
	this->surface = surf;
	this->piece = piece;
}

void
CColLine::Set(const CVector &p0, const CVector &p1)
{
	this->p0 = p0;
	this->p1 = p1;
}

void
CColTriangle::Set(int a, int b, int c, uint8 surf)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->surface = surf;
}

void
CColTrianglePlane::Set(const CVector *v, CColTriangle &tri)
{
	const CVector &va = v[tri.a];
	const CVector &vb = v[tri.b];
	const CVector &vc = v[tri.c];

	normal = CrossProduct(vc-va, vb-va);
	normal.Normalise();
	dist = DotProduct(normal, va);
	CVector an(fabs(normal.x), fabs(normal.y), fabs(normal.z));
	// find out largest component and its direction
	if(an.x > an.y && an.x > an.z)
		dir = normal.x < 0.0f ? DIR_X_NEG : DIR_X_POS;
	else if(an.y > an.z)
		dir = normal.y < 0.0f ? DIR_Y_NEG : DIR_Y_POS;
	else
		dir = normal.z < 0.0f ? DIR_Z_NEG : DIR_Z_POS;
}

CColModel::CColModel(void)
{
	numSpheres = 0;
	spheres = nil;
	numLines = 0;
	lines = nil;
	numBoxes = 0;
	boxes = nil;
	numTriangles = 0;
	vertices = nil;
	triangles = nil;
	trianglePlanes = nil;
	level = CGame::currLevel;
	ownsCollisionVolumes = true;
}

CColModel::~CColModel(void)
{
	RemoveCollisionVolumes();
	RemoveTrianglePlanes();
}

void
CColModel::RemoveCollisionVolumes(void)
{
	if(ownsCollisionVolumes){
		rwFree(spheres);
		rwFree(lines);
		rwFree(boxes);
		rwFree(vertices);
		rwFree(triangles);
	}
	numSpheres = 0;
	numLines = 0;
	numBoxes = 0;
	numTriangles = 0;
	spheres = nil;
	lines = nil;
	boxes = nil;
	vertices = nil;
	triangles = nil;
}

void
CColModel::CalculateTrianglePlanes(void)
{
	// HACK: allocate space for one more element to stuff the link pointer into
	trianglePlanes = rwNewT(CColTrianglePlane, numTriangles+1, 0);
	for(int i = 0; i < numTriangles; i++)
		trianglePlanes[i].Set(vertices, triangles[i]);
}

void
CColModel::RemoveTrianglePlanes(void)
{
	rwFree(trianglePlanes);
	trianglePlanes = nil;
}

CLink<CColModel*>*
CColModel::GetLinkPtr(void)
{
	assert(trianglePlanes);
	return *(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]);
}

void
CColModel::SetLinkPtr(CLink<CColModel*> *lptr)
{
	assert(trianglePlanes);
	*(CLink<CColModel*>**)ALIGNPTR(&trianglePlanes[numTriangles]) = lptr;
}
