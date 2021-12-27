
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
	return (unsigned int)bones.size();
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
	globalBone.bindPos = parent.bindPos + RotateVectorByQuaternion(globalBone.bindPos, parent.bindRot);
	globalBone.pos = parent.pos + RotateVectorByQuaternion(globalBone.pos, parent.rot);
	globalBone.bindRot = QuaternionMultiply(parent.bindRot, globalBone.bindRot);
	globalBone.rot = QuaternionMultiply(parent.rot, globalBone.rot);

	return globalBone;
}

const char* SkeletalMesh::GetBoneNameFromIndex(const int index) const
{
	return bones[index].name.c_str();
}

void SkeletalMesh::UpdateSkeleton(float deltaTime)
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

Maths::mat4x4 SkeletalMesh::GetBoneMatrix(int index, bool getInRestSkeleton)
{
	Maths::mat4x4 matrix;

	Bone bone = GetGlobalBoneFromIndex(index);

	if (getInRestSkeleton)
		matrix = Maths::mat4::translate(bone.bindPos) * Maths::mat4::MakeRotationMatFromQuaternion(bone.bindRot);
	else
		matrix = Maths::mat4::translate(bone.pos) * Maths::mat4::MakeRotationMatFromQuaternion(bone.rot);

	return matrix;
}

void SkeletalMesh::GetSkeletonMatrixArray(Maths::mat4x4* matrix)
{
	for (unsigned int i = 0; i < 64; i++)
	{
		if (i < GetSkeletonSize())
		{
			matrix[i] = GetBoneMatrix(i);

			matrix[i] = matrix[i] * Maths::mat4::Invert(GetBoneMatrix(i, true));
		}
		else
			matrix[i] = Maths::mat4::identity();
	}
}

void SkeletalMesh::GetSkeletonMatrixFloat(float* fMatrix)
{
	mat4x4 matrix[64];
	GetSkeletonMatrixArray(matrix);

	unsigned int bonesSize = GetSkeletonSize();

	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			//fMatrix[i * 16 + j] = matrix[i].c[0].e[j];
			//fMatrix[i * 16 + j] = matrix[i].c[1].e[j];
			//fMatrix[i * 16 + j] = matrix[i].c[2].e[j];
			//fMatrix[i * 16 + j] = matrix[i].c[3].e[j];
			
			fMatrix[i * 16 + j] = matrix[i].e[j];
		}
	}
}