#include "skeletalmesh.hpp"

void SkeletalMesh::AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent)
{
	Bone b;
	b.name = name;
	b.pos = pos;
	b.rot = rot;
	b.parent = parent;

	bones.push_back(b);
}

unsigned int SkeletalMesh::GetSkeletonSize() const
{
	return (int)bones.size();
}

Bone SkeletalMesh::GetLocalBoneFromIndex(const int index) const
{
	return bones[index];
}

Bone SkeletalMesh::GetGlobalBoneFromIndex(const int index) const
{
	int parentIndex = bones[index].parent;
	if (parentIndex == -1)
		return bones[index];

	Bone parent = GetGlobalBoneFromIndex(parentIndex);

	Bone g = bones[index];
	g.pos = parent.pos + RotateVectorByQuaternion(g.pos, parent.rot);
	g.rot = QuaternionMultiply(parent.rot, g.rot);

	return g;
}

const char* SkeletalMesh::GetBoneNameFromIndex(const int index) const
{
	return bones[index].name.c_str();
}