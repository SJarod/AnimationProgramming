
#include "Maths.hpp"
#include "Constants.hpp"

#include <iostream>

// These are functions coming from raymath.h, mandatory for the collision/intersection functions

#pragma region PLANE CONSTRUCTORS
Maths::Plane::Plane(const Vector3 p1, const Vector3 p2, const Vector3 p3)
{
	Vector3 AB = p2 - p1;
	Vector3 AC = p3 - p1;

	normal = normalizeVector3(crossProductVector3(AB, AC));
	d = dotVector3(p1, normal);
}

Maths::Plane::Plane(const Vector3 inNormal, const Vector3 inPoint)
{
	normal = inNormal;
	d = dotVector3(inPoint, normal);
}
#pragma endregion

#pragma region INTERSECTION DETECTION
bool Maths::intersectSegmentSphere(const Vector3& p1, const Vector3& p2, const Sphere& sphere, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 AB = p2 - p1;
	Vector3 CenterA = p1 - sphere.center;

	float ABsquared = dotVector3(AB, AB);
	float dotCenterAB = dotVector3(CenterA, AB);

	float delta = (4 * (dotCenterAB * dotCenterAB)) - (4 * ABsquared * (dotVector3(CenterA, CenterA) - sphere.radius * sphere.radius));

	if (delta < 0)
		return false;

	float T = (-2 * dotCenterAB - sqrt(delta)) / (2 * ABsquared);

	if (T < 0 || T > 1)
		return false;

	intersectPt = p1 + AB * T;
	Vector3 normal = intersectPt - sphere.center;
	intersectNormal = normalizeVector3(normal);
	return true;
}

bool Maths::intersectLinePlane(const Vector3& p1, const Vector3& p2, const Plane& plane, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 AB = p2 - p1;
	float dotABNormal = dotVector3(AB, plane.normal);

	if (dotABNormal == 0)
		return false;

	Vector3 P = plane.normal * plane.d;
	float T = dotVector3(P - p1, plane.normal) / dotABNormal;

	intersectPt = p1 + AB * T;

	if (dotVector3(AB, plane.normal) <= 0)
		intersectNormal = plane.normal;
	else
		intersectNormal = -plane.normal;

	return true;
}

bool Maths::intersectSegmentPlane(const Vector3& p1, const Vector3& p2, const Plane& plane, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 AB = p2 - p1;
	float dotABNormal = dotVector3(AB, plane.normal);

	if (dotABNormal == 0)
		return false;

	Vector3 P = plane.normal * plane.d;
	float T = dotVector3(P - p1, plane.normal) / dotABNormal;

	if (T < 0 || T > 1)
		return false;

	intersectPt = p1 + AB * T;

	if (dotVector3(AB, plane.normal) <= 0)
		intersectNormal = plane.normal;
	else
		intersectNormal = -plane.normal;

	return true;
}

bool Maths::intersectSegmentCylinderInfinite(const Vector3& p1, const Vector3& p2, const Cylinder& cylinder, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 AB = p2 - p1;
	float ABsquared = dotVector3(AB, AB);

	if (ABsquared <= 0.0001f && ABsquared >= -0.0001f)
		return false;

	Vector3 PQ = cylinder.p2 - cylinder.p1;
	Vector3 PA = p1 - cylinder.p1;

	float PQsquared = dotVector3(PQ, PQ);
	float PQdotAB = dotVector3(PQ, AB);

	float a = PQsquared * ABsquared - PQdotAB * PQdotAB;

	if (a <= 0.0001f && a >= -0.0001f)
		return false;

	float PAsquared = dotVector3(PA, PA);
	float PAdotPQ = dotVector3(PA, PQ);

	float c = PQsquared * PAsquared - cylinder.radius * cylinder.radius * PQsquared - PAdotPQ * PAdotPQ;

	if (c < 0)
		return false;

	float b = PQsquared * dotVector3(PA, AB) - PQdotAB * PAdotPQ;
	float delta = (b * b - a * c);

	if (delta < 0)
		return false;

	float T = (-b - sqrt(b * b - a * c)) / a;

	if (T < 0 || T > 1)
		return false;

	intersectPt = p1 + AB * T;

	Plane intersectPlane(normalizeVector3(PQ), intersectPt);

	Vector3 O, placeholder; // This is the orthogonal projection of intersectPt on PQ
	intersectLinePlane(cylinder.p1, cylinder.p2, intersectPlane, O, placeholder);

	intersectNormal = normalizeVector3(intersectPt - O);

	return true;
}

bool Maths::intersectSegmentCylinderFinite(const Vector3& p1, const Vector3& p2, const Cylinder& cylinder, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 cylinderIntersection = {};
	if (!intersectSegmentCylinderInfinite(p1, p2, cylinder, cylinderIntersection, intersectNormal))
		cylinderIntersection = p1;

	Vector3 PQ = cylinder.p2 - cylinder.p1;
	Vector3 PM = cylinderIntersection - cylinder.p1;

	if (dotVector3(PM, PQ) < 0 || dotVector3(PM, PQ) > dotVector3(PQ, PQ))
	{
		Plane Qplane(normalizeVector3(PQ), cylinder.p2);
		Plane Pplane(normalizeVector3(-PQ), cylinder.p1);

		float lengthDiff = squaredLengthVector3(cylinder.p1 - p1) - squaredLengthVector3(cylinder.p2 - p1);

		if (lengthDiff > 0 && intersectSegmentPlane(p1, p2, Qplane, cylinderIntersection, intersectNormal))
		{
			float len = squaredLengthVector3(cylinder.p2 - cylinderIntersection);
			if (squaredLengthVector3(cylinder.p2 - cylinderIntersection) <= cylinder.radius * cylinder.radius)
			{
				intersectPt = cylinderIntersection;
				return true;
			}
		}
		else if (lengthDiff < 0 && intersectSegmentPlane(p1, p2, Pplane, cylinderIntersection, intersectNormal))
			if (squaredLengthVector3(cylinder.p1 - cylinderIntersection) <= cylinder.radius * cylinder.radius)
			{
				intersectPt = cylinderIntersection;
				return true;
			}
	}
	else
	{
		intersectPt = cylinderIntersection;
		return true;
	}
	return false;
}

bool Maths::intersectSegmentCylinder(const Vector3& p1, const Vector3& p2, const Cylinder& cylinder, Vector3& intersectPt, Vector3& intersectNormal)
{
	if (cylinder.infinite)
		return intersectSegmentCylinderInfinite(p1, p2, cylinder, intersectPt, intersectNormal);
	else
		return intersectSegmentCylinderFinite(p1, p2, cylinder, intersectPt, intersectNormal);
}

bool Maths::intersectSegmentCapsule(const Vector3& p1, const Vector3& p2, const Capsule& capsule, Vector3& intersectPt, Vector3& intersectNormal)
{
	Vector3 cylinderIntersection = {};
	Cylinder cylinder = { capsule.p1, capsule.p2, capsule.radius, false };
	Sphere sphere1 = { capsule.p1, capsule.radius };
	Sphere sphere2 = { capsule.p2, capsule.radius };
	bool intersectCyl = false;
	int intersectPlane = 0;

	if (!intersectSegmentCylinderInfinite(p1, p2, cylinder, cylinderIntersection, intersectNormal))
		cylinderIntersection = p1;

	Vector3 PQ = capsule.p2 - capsule.p1;
	Vector3 PM = cylinderIntersection - capsule.p1;
	Plane Qplane(normalizeVector3(PQ), capsule.p2);
	Plane Pplane(normalizeVector3(-PQ), capsule.p1);

	float lengthDiff = squaredLengthVector3(capsule.p1 - p1) - squaredLengthVector3(capsule.p2 - p1);

	if (dotVector3(PM, PQ) < 0 || dotVector3(PM, PQ) > dotVector3(PQ, PQ))
	{
		if (lengthDiff > 0 && intersectSegmentPlane(p1, p2, Qplane, cylinderIntersection, intersectNormal))
		{
			intersectPlane = 2;
			if (squaredLengthVector3(capsule.p2 - cylinderIntersection) <= capsule.radius * capsule.radius)
				intersectCyl = true;
		}
		else if (lengthDiff < 0 && intersectSegmentPlane(p1, p2, Pplane, cylinderIntersection, intersectNormal))
		{
			intersectPlane = 1;
			if (squaredLengthVector3(capsule.p1 - cylinderIntersection) <= capsule.radius * capsule.radius)
				intersectCyl = true;
		}
	}
	else
		intersectCyl = true;

	if (!intersectCyl)
	{
		if (intersectSegmentSphere(p1, p2, sphere1, intersectPt, intersectNormal) || intersectSegmentSphere(p1, p2, sphere2, intersectPt, intersectNormal))
			return true;
	}
	else
	{
		if (intersectPlane == 0)
		{
			intersectPt = cylinderIntersection;
			return true;
		}

		if (intersectPlane == 1)
		{
			Sphere capsEnd = { capsule.p1, capsule.radius };
			return (intersectSegmentSphere(p1, p2, capsEnd, intersectPt, intersectNormal));
		}
		if (intersectPlane == 2)
		{
			Sphere capsEnd = { capsule.p2, capsule.radius };
			return (intersectSegmentSphere(p1, p2, capsEnd, intersectPt, intersectNormal));
		}
	}

	return false;
}

bool Maths::intersectSegmentQuad(const Vector3& p1, const Vector3& p2, const Quad& quad, Vector3& intersectPt, Vector3& intersectNormal)
{
	Quaternion quaternion = QuaternionFromEuler(quad.rotation.x, quad.rotation.y, quad.rotation.z);
	Vector3 normal = normalizeVector3(RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion));

	Plane plane(normal, quad.center);

	Vector3 planeIntersection = {};
	if (!intersectSegmentPlane(p1, p2, plane, planeIntersection, intersectNormal))
		return false;

	Vector3 i = normalizeVector3(RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion));
	Vector3 j = normalizeVector3(RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion));
	Vector3 vect = planeIntersection - quad.center;

	if (fabs(dotVector3(j, vect)) > quad.width / 2 || fabs(dotVector3(i, vect)) > quad.length / 2)
		return false;

	intersectPt = planeIntersection;
	return true;
}

Vector3 VecFromPoints(const Vector3& A, const Vector3& B)
{
	Vector3 AB;
	AB.x = B.x - A.x;
	AB.y = B.y - A.y;
	AB.z = B.z - A.z;
	return AB;
}

//bool IsInLocalRange3D(const Vector3& referencial, const Vector3& accuratePos, float rangeX, float rangeY, float rangeZ)
//{
//	Vector3 vecAccurateInLocalCoords = VecFromPoints(referencial, accuratePos);
//	if (vecAccurateInLocalCoords.x <= rangeX && vecAccurateInLocalCoords.x >= (-rangeX) && vecAccurateInLocalCoords.y <= rangeY && vecAccurateInLocalCoords.y >= (-rangeY) && vecAccurateInLocalCoords.z <= rangeZ && vecAccurateInLocalCoords.z >= (-rangeZ))
//		return true;
//	else
//		return false;
//}

bool Maths::intersectSegmentBox(const Vector3& p1, const Vector3& p2, const Box& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	intersectPt = { BOX_MAX_RANGE, 0, 0 };
	Quaternion quaternion = QuaternionFromEuler(box.rotation.x, box.rotation.y, box.rotation.z);

	Vector3 i = RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion);
	Vector3 j = RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion);
	Vector3 k = RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion);

	Vector3 intersectPtTests = {};
	Vector3 intersectNormalTests = {};

	Quaternion quadRotation = QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 0.0f, 0.0f, 1.0f }, M_PI / 2.0f));

	Quad quad = { box.dimensions.y, box.dimensions.z , { box.center + i * box.dimensions.x / 2 }, QuaternionToEulerRadians(quadRotation) };

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - i * box.dimensions.x / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center + j * box.dimensions.y / 2;
	quad.width = box.dimensions.x;
	quad.length = box.dimensions.z;
	quad.rotation = QuaternionToEulerRadians(QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 0.0f, 1.0f, 0.0f }, M_PI)));

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - j * box.dimensions.y / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center + k * box.dimensions.z / 2;
	quad.width = box.dimensions.x;
	quad.length = box.dimensions.y;
	quad.rotation = QuaternionToEulerRadians(QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 1.0f, 0.0f, 0.0f }, M_PI / 2.0f)));

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - k * box.dimensions.z / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (lengthVector3(intersectPtTests - p1) < lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	if (intersectPt.x == BOX_MAX_RANGE)
		return false;
	else
		return true;
}

void intersectSegmentRBoxFaces(const Vector3& p1, const Vector3& p2, const Maths::RoundedBox& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	Quaternion quaternion = QuaternionFromEuler(box.rotation.x, box.rotation.y, box.rotation.z);

	Vector3 i = RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion);
	Vector3 j = RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion);
	Vector3 k = RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion);

	Vector3 intersectPtTests = { BOX_MAX_RANGE, 0, 0 };
	Vector3 intersectNormalTests = {};

	Quaternion quadRotation = QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 0.0f, 0.0f, 1.0f }, M_PI / 2.0f));

	Maths::Quad quad = { box.dimensions.y, box.dimensions.z , { box.center + i * (box.dimensions.x + box.radius) / 2 }, QuaternionToEulerRadians(quadRotation) };

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - i * (box.dimensions.x + box.radius) / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center + j * (box.dimensions.y + box.radius) / 2;
	quad.width = box.dimensions.x;
	quad.length = box.dimensions.z;
	quad.rotation = QuaternionToEulerRadians(QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 0.0f, 1.0f, 0.0f }, M_PI)));

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - j * (box.dimensions.y + box.radius) / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center + k * (box.dimensions.z + box.radius) / 2;
	quad.width = box.dimensions.x;
	quad.length = box.dimensions.y;
	quad.rotation = QuaternionToEulerRadians(QuaternionMultiply(quaternion, QuaternionFromAxisAngle({ 1.0f, 0.0f, 0.0f }, M_PI / 2.0f)));

	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	quad.center = box.center - k * (box.dimensions.z + box.radius) / 2;
	if (intersectSegmentQuad(p1, p2, quad, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1))
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}
}

void intersectSegmentRBoxCapsules(const Vector3& p1, const Vector3& p2, const Maths::RoundedBox& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	Quaternion quaternion = QuaternionFromEuler(box.rotation.x, box.rotation.y, box.rotation.z);

	Vector3 i = RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion);
	Vector3 j = RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion);
	Vector3 k = RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion);

	Vector3 intersectPtTests = { BOX_MAX_RANGE, 0, 0 };
	Vector3 intersectNormalTests = {};

	Maths::Capsule capsule = { { box.center + i * box.dimensions.x / 2 + k * box.dimensions.z / 2 },
		{ box.center + i * box.dimensions.x / 2 + k * box.dimensions.z / 2 }, box.radius / 2 };

	capsule.p1 = capsule.p1 + j * box.dimensions.y / 2;
	capsule.p2 = capsule.p2 - j * box.dimensions.y / 2;
	if (intersectSegmentCapsule(p1, p2, capsule, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	capsule.p1 = capsule.p1 - i * box.dimensions.x;
	capsule.p2 = capsule.p2 - i * box.dimensions.x;
	if (intersectSegmentCapsule(p1, p2, capsule, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	capsule.p2 = capsule.p2 - k * box.dimensions.z;
	capsule.p1 = capsule.p1 - k * box.dimensions.z;
	if (intersectSegmentCapsule(p1, p2, capsule, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	capsule.p1 = capsule.p1 + i * box.dimensions.x;
	capsule.p2 = capsule.p2 + i * box.dimensions.x;
	if (intersectSegmentCapsule(p1, p2, capsule, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}
}

void intersectSegmentRBoxCylinders(const Vector3& p1, const Vector3& p2, const Maths::RoundedBox& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	Quaternion quaternion = QuaternionFromEuler(box.rotation.x, box.rotation.y, box.rotation.z);

	Vector3 i = RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion);
	Vector3 j = RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion);
	Vector3 k = RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion);

	Vector3 intersectPtTests = { BOX_MAX_RANGE, 0, 0 };
	Vector3 intersectNormalTests = {};

	Maths::Cylinder cylinder = { { box.center + i * box.dimensions.x / 2 + j * box.dimensions.y / 2 },
		{ box.center + i * box.dimensions.x / 2 + j * box.dimensions.y / 2 }, box.radius / 2, false };

	cylinder.p1 = cylinder.p1 + k * box.dimensions.z / 2;
	cylinder.p2 = cylinder.p2 - k * box.dimensions.z / 2;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 - i * box.dimensions.x;
	cylinder.p2 = cylinder.p2 - i * box.dimensions.x;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 - j * box.dimensions.y;
	cylinder.p2 = cylinder.p2 - j * box.dimensions.y;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 + i * box.dimensions.x;
	cylinder.p2 = cylinder.p2 + i * box.dimensions.x;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = box.center + k * box.dimensions.z / 2 + j * box.dimensions.y / 2 + i * box.dimensions.x / 2;
	cylinder.p2 = box.center + k * box.dimensions.z / 2 + j * box.dimensions.y / 2 - i * box.dimensions.x / 2;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 - j * box.dimensions.y;
	cylinder.p2 = cylinder.p2 - j * box.dimensions.y;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 - k * box.dimensions.z;
	cylinder.p2 = cylinder.p2 - k * box.dimensions.z;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}

	cylinder.p1 = cylinder.p1 + j * box.dimensions.y;
	cylinder.p2 = cylinder.p2 + j * box.dimensions.y;
	if (intersectSegmentCylinder(p1, p2, cylinder, intersectPtTests, intersectNormalTests))
		if (Maths::lengthVector3(intersectPtTests - p1) < Maths::lengthVector3(intersectPt - p1) && intersectPtTests != p1)
		{
			intersectPt = intersectPtTests;
			intersectNormal = intersectNormalTests;
		}
}

bool Maths::intersectSegmentRoundedBox(const Vector3& p1, const Vector3& p2, const RoundedBox& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	intersectPt = { BOX_MAX_RANGE, 0, 0 };

	Vector3 intersectPtTests = { BOX_MAX_RANGE, 0, 0 };
	Vector3 intersectNormalTests = {};

	intersectSegmentRBoxFaces(p1, p2, box, intersectPtTests, intersectNormalTests);
	intersectSegmentRBoxCylinders(p1, p2, box, intersectPtTests, intersectNormalTests);
	intersectSegmentRBoxCapsules(p1, p2, box, intersectPtTests, intersectNormalTests);

	if (intersectPtTests.x != BOX_MAX_RANGE)
	{
		intersectPt = intersectPtTests;
		intersectNormal = intersectNormalTests;
	}

	if (intersectPt.x == BOX_MAX_RANGE)
		return false;
	else
		return true;
}

bool Maths::intersectSphereBox(const Vector3& velocity, const Sphere& sphere, const Box& box, Vector3& intersectPt, Vector3& intersectNormal)
{
	RoundedBox rBox = {box.dimensions, box.center, box.rotation, sphere.radius};

	return (intersectSegmentRoundedBox(sphere.center, sphere.center + velocity, rBox, intersectPt, intersectNormal));
}

#pragma endregion

void globToLocPos(const referential3D& local, Vector3& pt)
{
	Vector3 OP = { pt.x - local.origin.x, pt.y - local.origin.y,  pt.z - local.origin.z };

	pt.x = OP.x;
	pt.y = OP.y;
	pt.z = OP.z;
}

#pragma region COLLISION FUNCTIONS
bool Maths::collisionSphereBox(const Sphere& sphere, const Box& box)
{
	/*Quaternion quaternion = QuaternionFromEuler(box.rotation.x, box.rotation.y, box.rotation.z);

	Vector3 i = RotateVectorByQuaternion({ 1.0f, 0.0f, 0.0f }, quaternion);
	Vector3 j = RotateVectorByQuaternion({ 0.0f, 1.0f, 0.0f }, quaternion);
	Vector3 k = RotateVectorByQuaternion({ 0.0f, 0.0f, 1.0f }, quaternion);

	Vector3 boxToSphere = sphere.center - box.center;
	Sphere localSphere = { {dotVector3(boxToSphere, i), dotVector3(boxToSphere, j), dotVector3(boxToSphere, k)}, sphere.radius };

	if (!(localSphere.center.x > (box.dimensions.x / 2) + localSphere.radius)
		|| !(localSphere.center.y > (box.dimensions.y / 2) + localSphere.radius)
		|| !(localSphere.center.z > (box.dimensions.z / 2) + localSphere.radius))
		return false;

	Vector3 closestPoint = { clamp(localSphere.center.x, -box.dimensions.x / 2, box.dimensions.x / 2),
		clamp(localSphere.center.y, -box.dimensions.y / 2, box.dimensions.y / 2),
		clamp(localSphere.center.z, -box.dimensions.z / 2, box.dimensions.z / 2) };

	boxToSphere = localSphere.center - closestPoint;

	if (squaredLengthVector3(boxToSphere) > sphere.radius * sphere.radius)
		return false;

	return true;*/

	

	referential3D boxReferencial;
	boxReferencial.origin = box.center;

	Vector3 spherePos = sphere.center;

	globToLocPos(boxReferencial, spherePos);

	if (spherePos.x > (box.dimensions.x + sphere.radius) || spherePos.y > (box.dimensions.y + sphere.radius) || spherePos.z > (box.dimensions.z + sphere.radius))
		return false;

	if (spherePos.x < -(box.dimensions.x + sphere.radius) || spherePos.y < -(box.dimensions.y + sphere.radius) || spherePos.z < -(box.dimensions.z + sphere.radius))
		return false;

	Vector3 P;
	P.x = Maths::clamp(spherePos.x, -1 * box.dimensions.x, box.dimensions.x);
	P.y = Maths::clamp(spherePos.y, -1 * box.dimensions.y, box.dimensions.y);
	P.z = Maths::clamp(spherePos.z, -1 * box.dimensions.z, box.dimensions.z);

	if (Maths::lengthVector3(VecFromPoints(P, spherePos)) < sphere.radius)
		return true;
	else
		return false;
}

bool Maths::collisionBoxBox(const Box& box1, const Box& box2)
{

	return true;
}


bool Maths::collisionSphereSphere(const Sphere& sphere1, const Sphere& sphere2)
{
	if (lengthVector3(sphere2.center - sphere1.center) < sphere1.radius)
		return true;
	else
		return false;
}

#pragma endregion

#pragma region VEC2
int Maths::compareVector2(const Vector2& vector1, const Vector2& vector2, float epsilon)
{
    if (vector1.x <= (vector2.x + epsilon) && vector1.x >= (vector2.x - epsilon))
        if (vector1.y <= (vector2.y + epsilon) && vector1.y >= (vector2.y - epsilon))
            return 0;
        else
            return 1;
    else
        return 1;
}

float Maths::lengthVector2(const Vector2& vector)
{
    float len = sqrtf((vector.x * vector.x) + (vector.y * vector.y));

    return len;
}

float Maths::squaredLengthVector2(const Vector2& vector)
{
    return (vector.x * vector.x) + (vector.y * vector.y);
}

float Maths::pointsDistance(const Vector2& p1, const Vector2& p2)
{
    Vector2 vector = { p2.x - p1.x, p2.y - p1.y };

    return lengthVector2(vector);
}

float Maths::dotProductVector2(const Vector2& vector1, const Vector2& vector2)
{
    return ((vector1.x * vector2.x) + (vector1.y * vector2.y));
}

float Maths::angleVector2(const Vector2& vector1, const Vector2& vector2)
{
    if ((vector1.x == 0 && vector1.y == 0) || (vector2.x == 0 && vector2.y == 0))
        return 0;

    float dot = dotProductVector2(vector1, vector2);
    float num = (vector1.x * vector2.y) - (vector1.y * vector2.x);
    return atan2(num, dot);
}

float Maths::angleDegVector2(const Vector2& vector1, const Vector2& vector2)
{
    float angle = angleVector2(vector1, vector2);
    return (float)(angle * 180 / 3.1415f);
}

Maths::Vector2 Maths::additionVector2(const Vector2& vector1, const Vector2& vector2)
{
    Vector2 resVect = {};
    resVect.x = vector1.x + vector2.x;
    resVect.y = vector1.y + vector2.y;
    return resVect;
}

Maths::Vector2 Maths::negateVector2(const Vector2& vector)
{
    Vector2 resVect;
    resVect.x = -vector.x;
    resVect.y = -vector.y;

    return resVect;
}

Maths::Vector2 Maths::unitVector2(Vector2& vector)
{
    float len = lengthVector2(vector);
    vector.y /= len;
    vector.x /= len;
    return vector;
}

Maths::Vector2 Maths::substractVector2(const Vector2& vector1, const Vector2& vector2)
{
    Vector2 tempVect = negateVector2(vector2);

    Vector2 resVect = additionVector2(vector1, tempVect);

    return resVect;
}

Maths::Vector2 Maths::scaleVector2(const Vector2& vector, float scale)
{
    Vector2 resVect;
    resVect.x = scale * vector.x;
    resVect.y = scale * vector.y;

    return resVect;
}

void Maths::vectRotate(Vector2& vector, float angle)
{
    float x = vector.x;

    vector.x = x * cosf(angle) - vector.y * sinf(angle);
    vector.y = x * sinf(angle) + vector.y * cosf(angle);
}

void Maths::pointRotate(const Vector2& origin, Vector2& point, float angle)
{
    Vector2 vect = { point.x - origin.x, point.y - origin.y };
    vectRotate(vect, angle);

    point.x = vect.x + origin.x;
    point.y = vect.y + origin.y;
}

void Maths::vectRotate90(Vector2& vector)
{
    float x = vector.x;

    vector.x = -vector.y;
    vector.y = x;
}

Maths::Vector2 Maths::normalVector2(const Vector2& vector)
{
    Vector2 retVect = { vector.x, vector.y };
    vectRotate90(retVect);
    return retVect;
}
#pragma endregion

#pragma region VEC4
#pragma endregion

#pragma region MAT4
mat4x4 mat4::perspective(float fovY, float aspect, float near, float far)
{
    float top = near * tanf((fovY * 3.14159f / 180.f) / 2);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    return frustum(left, right, bottom, top, near, far);
    /*
    mat4x4 projection;
    float f = 1 / tan((fovY / 180.f * 3.14159f) / 2);

    projection = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, -(far + near) / (far - near), -1,
        0, 0, -(2 * far * near) / (far - near), 0
    };

    return projection;*/
}

mat4x4 mat4::orthographic(float fovY, float aspect, float near, float far)
{
	float top = 10;
	float bottom = -10;
	float right = top * aspect;
	float left = -right;
	near;

	// Pre-compute divisions
	float OneOverTopMinusBottom = 1.f / (top - bottom);
	float OneOverFarMinusNear = 1.f / (far - near);
	float OneOverRightMinusLeft = 1.f / (right - left);

	return {
		2.f * OneOverRightMinusLeft,           0.f,                                    0.f,                               0.f,
		0.f,                                   2.f * OneOverTopMinusBottom,            0.f,                               0.f,
		0.f,                                   0.f,                                   -2.f * OneOverFarMinusNear,         0.f,
	  -(right + left) * OneOverRightMinusLeft,-(top + bottom) * OneOverTopMinusBottom,-(far + near)* OneOverFarMinusNear, 1.f
	};
}

mat4x4 mat4::frustum(float left, float right, float bottom, float top, float near, float far)
{
    /*
    return {
            2 * near / (right - left),  0.f,                        0.f,                            -near * (right + left) / (right - left),
            0.f,                        2 * near / (top - bottom),  0.f,                            -near * (top + bottom) / (top - bottom),
            0.f,                        0.f,                        -(far + near) / (far - near),   2 * far * near / (near - far),
            0.f,                        0.f,                        -1.f,                           0.f,
    };*/
    return {
            2 * near / (right - left),       0.f,                              0.f,                             0.f,
            0.f,                             2 * near / (top - bottom),        0.f,                             0.f,
            (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near),    -1.f,
            0.f,                             0.f,                             -(2 * far * near) / (far - near), 0.f,
    };
}

mat4x4 mat4::Invert(mat4x4& mat)
{
	mat4x4 result = {};

	// Cache the matrix values (speed optimization)
	float a00 = mat.e[0], a01 = mat.e[1], a02 = mat.e[2], a03 = mat.e[3];
	float a10 = mat.e[4], a11 = mat.e[5], a12 = mat.e[6], a13 = mat.e[7];
	float a20 = mat.e[8], a21 = mat.e[9], a22 = mat.e[10], a23 = mat.e[11];
	float a30 = mat.e[12], a31 = mat.e[13], a32 = mat.e[14], a33 = mat.e[15];

	float b00 = a00 * a11 - a01 * a10;
	float b01 = a00 * a12 - a02 * a10;
	float b02 = a00 * a13 - a03 * a10;
	float b03 = a01 * a12 - a02 * a11;
	float b04 = a01 * a13 - a03 * a11;
	float b05 = a02 * a13 - a03 * a12;
	float b06 = a20 * a31 - a21 * a30;
	float b07 = a20 * a32 - a22 * a30;
	float b08 = a20 * a33 - a23 * a30;
	float b09 = a21 * a32 - a22 * a31;
	float b10 = a21 * a33 - a23 * a31;
	float b11 = a22 * a33 - a23 * a32;

	// Calculate the invert determinant (inlined to avoid double-caching)
	float invDet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

	result.e[0] = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
	result.e[1] = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
	result.e[2] = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
	result.e[3] = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
	result.e[4] = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
	result.e[5] = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
	result.e[6] = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
	result.e[7] = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
	result.e[8] = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
	result.e[9] = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
	result.e[10] = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
	result.e[11] = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
	result.e[12] = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
	result.e[13] = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
	result.e[14] = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
	result.e[15] = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

	return result;
}
#pragma endregion

#pragma region QUATERNIONS
Vector3 Maths::QuaternionToEulerRadians(Quaternion q)
{
    Vector3 result = {};

    // roll (x-axis rotation)
    float x0 = 2.0f * (q.w * q.x + q.y * q.z);
    float x1 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    result.x = atan2f(x0, x1);

    // pitch (y-axis rotation)
    float y0 = 2.0f * (q.w * q.y - q.z * q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    result.y = asinf(y0);

    // yaw (z-axis rotation)
    float z0 = 2.0f * (q.w * q.z + q.x * q.y);
    float z1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    result.z = atan2f(z0, z1);

    return result;
}

Quaternion Maths::QuaternionFromEuler(float roll, float pitch, float yaw)
{
	Quaternion q = {};

	float x0 = cosf(roll * 0.5f);
	float x1 = sinf(roll * 0.5f);
	float y0 = cosf(pitch * 0.5f);
	float y1 = sinf(pitch * 0.5f);
	float z0 = cosf(yaw * 0.5f);
	float z1 = sinf(yaw * 0.5f);

	q.x = x1 * y0 * z0 - x0 * y1 * z1;
	q.y = x0 * y1 * z0 + x1 * y0 * z1;
	q.z = x0 * y0 * z1 - x1 * y1 * z0;
	q.w = x0 * y0 * z0 + x1 * y1 * z1;

	return q;
}

Vector3 Maths::RotateVectorByQuaternion(Vector3 v, Quaternion q)
{
	Vector3 result = {};

	result.x = v.x * (q.x * q.x + q.w * q.w - q.y * q.y - q.z * q.z) 
		+ v.y * (2 * q.x * q.y - 2 * q.w * q.z) 
		+ v.z * (2 * q.x * q.z + 2 * q.w * q.y);
	result.y = v.x * (2 * q.w * q.z + 2 * q.x * q.y) 
		+ v.y * (q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z) 
		+ v.z * (-2 * q.w * q.x + 2 * q.y * q.z);
	result.z = v.x * (-2 * q.w * q.y + 2 * q.x * q.z) 
		+ v.y * (2 * q.w * q.x + 2 * q.y * q.z) 
		+ v.z * (q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);

	return result;
}

Quaternion Maths::QuaternionMultiply(Quaternion q1, Quaternion q2)
{
	Quaternion result = { 0 };

	result.x = q1.x * q2.w + q1.w * q2.x + q1.y * q2.z - q1.z * q2.y;
	result.y = q1.y * q2.w + q1.w * q2.y + q1.z * q2.x - q1.x * q2.z;
	result.z = q1.z * q2.w + q1.w * q2.z + q1.x * q2.y - q1.y * q2.x;
	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

	return result;
}

float Maths::QuaternionLength(Quaternion q)
{
	return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

Quaternion Maths::QuaternionNormalize(Quaternion q)
{
	Quaternion result = { 0 };

	float length;
	length = QuaternionLength(q);
	if (length == 0.0f) return q;

	result.x = q.x / length;
	result.y = q.y / length;
	result.z = q.z / length;
	result.w = q.w / length;

	return result;
}

Quaternion Maths::QuaternionFromAxisAngle(Vector3 axis, float angle)
{
	Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (lengthVector3(axis) != 0.0f)

		angle *= 0.5f;

	axis = normalizeVector3(axis);

	float sinres = sinf(angle);
	float cosres = cosf(angle);

	result.x = axis.x * sinres;
	result.y = axis.y * sinres;
	result.z = axis.z * sinres;
	result.w = cosres;

	result = QuaternionNormalize(result);

	return result;
}

#pragma endregion

#pragma region OTHER
float Maths::getFloatsMin4(float num1, float num2, float num3, float num4)
{
    if (num1 <= num2 && num1 <= num3 && num1 <= num4)
        return num1;
    else if (num2 <= num1 && num2 <= num3 && num2 <= num4)
        return num2;
    else if (num3 <= num2 && num3 <= num1 && num3 <= num4)
        return num3;
    else
        return num4;
}

float Maths::getFloatsMin4(Vector4 vec)
{
    if (vec.x <= vec.y && vec.x <= vec.z && vec.x <= vec.w)
        return vec.x;
    else if (vec.y <= vec.x && vec.y <= vec.z && vec.y <= vec.w)
        return vec.y;
    else if (vec.z <= vec.y && vec.z <= vec.x && vec.z <= vec.w)
        return vec.z;
    else
        return vec.w;
}

float Maths::getFloatsMax4(float num1, float num2, float num3, float num4)
{
    if (num1 >= num2 && num1 >= num3 && num1 >= num4)
        return num1;
    else if (num2 >= num1 && num2 >= num3 && num2 >= num4)
        return num2;
    else if (num3 >= num2 && num3 >= num1 && num3 >= num4)
        return num3;
    else
        return num4;
}

float Maths::clamp(float toClamp, float min, float max)
{
    if (toClamp > max) return max;
    if (toClamp < min) return min;
    return toClamp;
}

int Maths::clamp(int toClamp, int min, int max)
{
    if (toClamp > max) return max;
    if (toClamp < min) return min;
    return toClamp;
}

int Maths::remap(int val, int min1, int max1, int min2, int max2)
{
	return min2 + (val - min1) * (max2 - min2) / (max1 - min1);
}

float Maths::lerp(const float& from, const float& to, const float& t)
{
	return (1 - t) * from + t * to;
}

Vector3 Maths::lerp(const Vector3& from, const Vector3& to, const float& t)
{
	Vector3 lerped;
	lerped.x = lerp(from.x, to.x, t);
	lerped.y = lerp(from.y, to.y, t);
	lerped.z = lerp(from.z, to.z, t);
	return lerped;
}

Quaternion Maths::nlerp(const Quaternion& from, const Quaternion& to, const float& t)
{
	Quaternion lerped;
	lerped.w = lerp(from.w, to.w, t);
	lerped.x = lerp(from.x, to.x, t);
	lerped.y = lerp(from.y, to.y, t);
	lerped.z = lerp(from.z, to.z, t);

	float mag = sqrtf(lerped.w * lerped.w + lerped.x * lerped.x + lerped.y * lerped.y + lerped.z * lerped.z);

	return Quaternion{ lerped.x / mag, lerped.y / mag, lerped.z / mag, lerped.w / mag };
}

Quaternion Maths::slerp(const Quaternion& from, const Quaternion& to, const float& t)
{
	//based on Wikipedia and Raylib

	float cosHalfOmega = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	Quaternion _to = to;
	if (cosHalfOmega < 0)
	{
		_to = _to * -1.f;
		cosHalfOmega *= -1.f;
	}

	if (fabs(cosHalfOmega) >= 1.0f)
	{
		return from;
	}
	else if (cosHalfOmega > 0.95f)
	{
		return nlerp(from, _to, t);
	}
	else
	{
		float halfOmega = acosf(cosHalfOmega);
		float sinHalfOmega = sqrtf(1.f - cosHalfOmega * cosHalfOmega);

		return from * (sinf((1 - t) * halfOmega) / sinHalfOmega) + _to * (sinf(t * halfOmega) / sinHalfOmega);
	}
}

#pragma endregion