
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
	/*
	if (parentIndex >= 0)
	{
		Bone parent;
		GetSkeletonBoneLocalBindTransform(parentIndex, parent.pos, parent.rot);
		matrix = matrix * parent.GetBoneMatrix(GetSkeletonBoneParentIndex(parentIndex));
	}
	*/
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
	b.pos = pos;
	b.rot = rot;
	b.parent = parent;

	bones.push_back(b);
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

void SkeletalMesh::UpdateSkeleton(float deltaTime)
{
	/*
	static int currentKeyFrame = 0;
	static float timer = 0.f;
	size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");

	for (unsigned int i = 0; i < GetSkeletonSize(); ++i)
	{
		Vector3 keyFramePos;
		Quaternion keyFrameQ;
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", i, currentKeyFrame, keyFramePos, keyFrameQ);

		Vector3 tPosePos;
		Quaternion tPoseQ;
		GetSkeletonBoneLocalBindTransform(i, tPosePos, tPoseQ);

		SetLocalBoneFromIndex(i, tPosePos + keyFramePos, QuaternionMultiply(tPoseQ, keyFrameQ));
	}

	timer += 0.1f;
	if (timer >= 1.f)
	{
		++currentKeyFrame;
		timer = 0.f;
	}
	if (currentKeyFrame >= (int)keyCount)
		currentKeyFrame = 0;
	*/
	
	//bones[1].localPos.z = sinf(movement) * 100.f;
	bones[1].localPos.y = cosf(movement) * 10.f;
	bones[2].localRot = Maths::QuaternionFromAxisAngle({ 1.0f, 0.f, 0.f }, sinf(movement / 40) * 6.f);
	//bones[56].localRot = Maths::QuaternionFromAxisAngle({0.0f, 1.f, 0.f}, cosf(movement / 24) * 4.f);

	movement += deltaTime;
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

	for (int i = 0; i < 64 && i < 64; i++)
	{
		if (i < bones.size())
		{
			matrix[i] = bones[i].GetBoneMatrix(bones[i].parent);

			matrix[i] = matrix[i] * Maths::mat4::Invert(restBones[i].GetBoneMatrix(restBones[i].parent));
		}
		else
			matrix[i] = Maths::mat4::identity();
	}

	return matrix;
}

float* SkeletalMesh::GetSkeletonMatrixFloat()
{
	mat4x4* matrix = GetSkeletonMatrixArray();

	unsigned int bonesSize = GetSkeletonSize();
	float fMatrix[64 * 16];

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

	return fMatrix;
}
