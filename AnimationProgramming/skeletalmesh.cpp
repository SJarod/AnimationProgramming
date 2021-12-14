#include "skeletalmesh.hpp"

void SkeletalMesh::AddBone(const std::string& name, const Vector3& localPos, const Quaternion& localRot, const int parent)
{
	Bone b;
	b.name = name;
	b.localPos = localPos;
	b.localRot = localRot;
	b.globalPos = localPos;
	b.globalRot = localRot;

	if (parent != -1 && (int)bones.size() > 0 && parent <= (int)bones.size())
	{
		b.globalPos = bones[parent].globalPos + RotateVectorByQuaternion(b.localPos, bones[parent].globalRot);
		b.globalRot = QuaternionMultiply(bones[parent].globalRot, b.globalRot);

		b.parent = parent;
	}

	bones.push_back(b);
}

unsigned int SkeletalMesh::GetSkeletonSize() const
{
	return (int)bones.size();
}

Bone SkeletalMesh::GetBoneFromIndex(const int index) const
{
	return bones[index];
}

const char* SkeletalMesh::GetBoneNameFromIndex(const int index) const
{
	return bones[index].name.c_str();
}