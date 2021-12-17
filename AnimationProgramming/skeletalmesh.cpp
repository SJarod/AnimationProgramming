
#include <chrono>
#include "skeletalmesh.hpp"
#include "Engine.h"
#include "EngineOverloads.h"

void Bone::DrawBoneNode(const float size, const Maths::Vector3 color)
{
	DrawLine(pos + Maths::Vector3{ -size, 0.f, size }, pos - Maths::Vector3{ -size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(pos + Maths::Vector3{ size, 0.f, size }, pos - Maths::Vector3{ size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(pos + Maths::Vector3{ 0.f, -size, size }, pos - Maths::Vector3{ 0.f, -size, size }, color.r, color.g, color.b);
	DrawLine(pos + Maths::Vector3{ 0.f, size, size }, pos - Maths::Vector3{ 0.f, size, size }, color.r, color.g, color.b);
}

Maths::mat4x4 Bone::GetBoneMatrix(int parentIndex)
{
	Maths::mat4x4 matrix = Maths::mat4::identity();

	if (parentIndex >= 0)
	{
		Bone parent;
		GetSkeletonBoneLocalBindTransform(parentIndex, parent.pos, parent.rot);
		matrix = matrix * parent.GetBoneMatrix(GetSkeletonBoneParentIndex(parentIndex));
	}

	matrix = matrix * Maths::mat4::translate(pos) * Maths::mat4::MakeRotationMatFromQuaternion(rot);

	return matrix;
}

void SkeletalMesh::AddBone(const Bone& bone)
{
	bones.push_back(bone);
}

void SkeletalMesh::AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent)
{
	Bone b;
	b.name = name;

	b.bindPos = pos;
	b.pos = pos;

	b.bindRot = rot;
	b.rot = rot;

	b.parent = parent;

	bones.push_back(b);
}

void SkeletalMesh::PlayAnimation(const Animation& anim, const float& playSpeed)
{
	static float timer = 0.f;
	static int currentKeyFrame = 0;

	for (unsigned int i = 0; i < GetSkeletonSize(); ++i)
	{
		KeyFrameBone kfb = anim.GetBoneFromKeyFrame(currentKeyFrame, i);
		KeyFrameBone nextKfb = anim.GetBoneFromKeyFrame(kfb.nextKF, i);

		Vector3 newPos = lerp(bones[i].bindPos + kfb.pos, bones[i].bindPos + nextKfb.pos, timer);
		Quaternion newRot = slerp(QuaternionMultiply(bones[i].bindRot, kfb.rot),
								  QuaternionMultiply(bones[i].bindRot, nextKfb.rot), timer);

		SetLocalBoneFromIndex(i, newPos, newRot);
	}

	timer += playSpeed;
	if (timer >= 1.f)
	{
		timer = 0.f;
		++currentKeyFrame;
		if (currentKeyFrame >= anim.GetAnimationSize())
			currentKeyFrame = 0;
	}
}

unsigned int SkeletalMesh::GetSkeletonSize() const
{
	return (int)bones.size();
}

void SkeletalMesh::SetLocalBoneFromIndex(const int index, const Vector3& pos, const Quaternion& rot)
{
	bones[index].pos = pos;
	bones[index].rot = rot;
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

	Bone globalBone = bones[index];
	globalBone.pos = parent.pos + RotateVectorByQuaternion(globalBone.pos, parent.rot);
	globalBone.rot = QuaternionMultiply(parent.rot, globalBone.rot);

	return globalBone;
}

const char* SkeletalMesh::GetBoneNameFromIndex(const int index) const
{
	return bones[index].name.c_str();
}

void SkeletalMesh::UpdateSkeleton(const float& deltaTime)
{
}

void SkeletalMesh::DrawSkeleton(const Maths::Vector3& skeletonDrawOffset)
{
	for (unsigned int i = 0; i < GetSkeletonSize(); i++)
	{
		std::string boneName = bones[i].name;
		if (boneName.substr(0, 3) == "ik_")
			continue;

		int parentIndex = bones[i].parent;
		if (parentIndex >= 0)
		{
			Bone bone = GetGlobalBoneFromIndex(i);
			Bone parent = GetGlobalBoneFromIndex(parentIndex);

			bone.pos = bone.pos + skeletonDrawOffset;
			parent.pos = parent.pos + skeletonDrawOffset;

			DrawLine(bone.pos, parent.pos, 0.f, 0.8f, 1.f);
			bone.DrawBoneNode(0.5f, { 1.f, 0.6f, 0.f });
			continue;
		}

		Bone bone = bones[i];
		bone.pos = bone.pos + skeletonDrawOffset;
		bone.DrawBoneNode(1.f, { 1.f, 0.f, 0.f });
	}
}

Maths::mat4x4* SkeletalMesh::GetSkeletonMatrixArray()
{
	Maths::mat4x4 matrix[64];
	for (unsigned int i = 0; i < GetSkeletonSize() && i < 64; i++)
	{
		matrix[i] = bones[i].GetBoneMatrix(GetSkeletonBoneParentIndex(i));
	}

	return matrix;
}

float* SkeletalMesh::GetSkeletonMatrixFloat()
{
	mat4x4* matrix = GetSkeletonMatrixArray();

	unsigned int bonesSize = GetSkeletonSize();
	float fMatrix[64 * 16];

	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 16; j++)
			fMatrix[i * 16 + j] = matrix[i].e[j];

	return fMatrix;
}