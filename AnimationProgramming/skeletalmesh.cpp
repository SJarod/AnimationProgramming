
#include <chrono>
#include "skeletalmesh.hpp"
#include "Engine.h"
#include "EngineOverloads.h"

void Bone::DrawBoneNode(const float size, const Maths::Vector3 color)
{
	DrawLine(localPos + Maths::Vector3{ -size, 0.f, size }, localPos - Maths::Vector3{ -size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(localPos + Maths::Vector3{ size, 0.f, size }, localPos - Maths::Vector3{ size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(localPos + Maths::Vector3{ 0.f, -size, size }, localPos - Maths::Vector3{ 0.f, -size, size }, color.r, color.g, color.b);
	DrawLine(localPos + Maths::Vector3{ 0.f, size, size }, localPos - Maths::Vector3{ 0.f, size, size }, color.r, color.g, color.b);
}

Maths::mat4x4 Bone::GetBoneMatrix(int parentIndex)
{
	Maths::mat4x4 matrix = Maths::mat4::identity();
	/*
	if (parentIndex >= 0)
	{
		Bone parent;
		GetSkeletonBoneLocalBindTransform(parentIndex, parent.localPos, parent.localRot);
		matrix = parent.GetBoneMatrix(GetSkeletonBoneParentIndex(parentIndex)) * matrix;
	}
	*/
	matrix =  Maths::mat4::MakeRotationMatFromQuaternion(globalRot) * Maths::mat4::translate(globalPos) * matrix;
	
	return matrix;
}

Bone SkeletalMesh::GetParentRelativeBone(int parentIndex, Bone& bone)
{
	if (parentIndex <= -1) return {};
	Bone parent;
	GetSkeletonBoneLocalBindTransform(parentIndex, parent.localPos, parent.localRot);

	int parentsParent = GetSkeletonBoneParentIndex(parentIndex);
	 
	if (parentsParent <= -1)
	{
		Bone result;
		result.name = GetSkeletonBoneName(parentIndex);
		result.localPos = parent.localPos + Maths::RotateVectorByQuaternion(bone.localPos, parent.localRot);
		result.localRot = QuaternionMultiply(parent.localRot, bone.localRot);
		return result;
	}

	parent = GetParentRelativeBone(parentsParent, parent);
	Bone result;
	result.name = GetSkeletonBoneName(parentIndex);
	result.localPos = parent.localPos + Maths::RotateVectorByQuaternion(bone.localPos, parent.localRot);
	result.localRot = QuaternionMultiply(parent.localRot, bone.localRot);
	return result;
}

Bone SkeletalMesh::GetGlobalBoneFromIndex(const int index) const
{
	int parentIndex = bones[index].parent;
	if (parentIndex == -1)
		return bones[index];

	Bone parent = GetGlobalBoneFromIndex(parentIndex);

	Bone globalBone = bones[index];
	globalBone.localPos = parent.localPos + RotateVectorByQuaternion(globalBone.localPos, parent.localRot);
	globalBone.localRot = QuaternionMultiply(parent.localRot, globalBone.localRot);

	return globalBone;
}

void SkeletalMesh::AddBone(const Bone& bone)
{
	bones.push_back(bone);
	
	if (bone.parent != -1 && (int)bones.size() > 0 && bone.parent <= (int)bones.size())
	{
		bones.back().globalPos = bones[bone.parent].globalPos + RotateVectorByQuaternion(bone.localPos, bones[bone.parent].globalRot);
		bones.back().globalRot = QuaternionMultiply(bones[bone.parent].globalRot, bones.back().localRot);
	}

}

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
		b.globalRot = QuaternionMultiply(bones[parent].globalRot, b.localRot);

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

void SkeletalMesh::UpdateSkeleton(float deltaTime)
{
	static float movement = 0.f;

	//bones[1].localPos.z = sinf(movement) * 100.f;
	bones[1].localPos.y = cosf(movement) * 10.f;
	bones[2].localRot = Maths::QuaternionFromAxisAngle({ 1.0f, 0.f, 0.f }, sinf(movement / 40) * 6.f);
	//bones[56].localRot = Maths::QuaternionFromAxisAngle({0.0f, 1.f, 0.f}, cosf(movement / 24) * 4.f);

	movement += deltaTime;
}

void SkeletalMesh::DrawSkeleton(const Maths::Vector3& skeletonDrawOffset)
{
	for (int i = 0; i < bones.size(); i++)
	{
		std::string boneName = bones[i].name;
		if (boneName.substr(0, 3) == "ik_")
			continue;

		int parentIndex = bones[i].parent;
		if (parentIndex >= 0)
		{
			Bone bone = GetGlobalBoneFromIndex(i);
			Bone parent = GetGlobalBoneFromIndex(parentIndex);
			//Bone bone = GetParentRelativeBone(parentIndex, bones[i]);
			//Bone parent = GetParentRelativeBone(GetSkeletonBoneParentIndex(parentIndex), bones[parentIndex]);
			bone.localPos = bone.localPos + skeletonDrawOffset;
			parent.localPos = parent.localPos + skeletonDrawOffset;

			DrawLine(bone.localPos, parent.localPos, 0.f, 0.8f, 1.f);
			bone.DrawBoneNode(0.5f, { 1.f, 0.6f, 0.f });
			continue;
		}
		Bone bone = bones[i];
		bone.localPos = bone.localPos + skeletonDrawOffset;
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

	int bonesSize = (int)bones.size();
	//float* fMatrix = new float[64 * 16];
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
