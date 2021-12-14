// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Engine.h"
#include "Simulation.h"
#include "Maths.hpp"

//#include <cstdlib>

struct Bone
{
	Maths::Vector3 position = {};
	Maths::Quaternion rotation = {};
};

#pragma region ENGINE TYPED FUNCTIONS
void GetSkeletonBoneLocalBindTransform(int boneIndex, Maths::Vector3& pos, Maths::Quaternion& q)
{
	GetSkeletonBoneLocalBindTransform(boneIndex, pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z);
}

void GetAnimLocalBoneTransform(const char* animName, int boneIndex, int keyFrameIndex, Maths::Vector3& pos, Maths::Quaternion& q)
{
	GetAnimLocalBoneTransform(animName, boneIndex, keyFrameIndex, pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z);
}

void DrawLine(Maths::Vector3 pos1, Maths::Vector3 pos2, float r, float g, float b)
{
	DrawLine(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z, r, g, b);
}

Maths::Vector3 GetAnimGlobalPos(const char* animName, int boneIndex, int keyFrameIndex)
{
	if (boneIndex == 0)
	{
		Maths::Vector3 pos;
		Quaternion q;
		GetAnimLocalBoneTransform(animName, boneIndex, keyFrameIndex, pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z);

		return pos;
	}

	Maths::Vector3 pos;
	Quaternion q;
	GetAnimLocalBoneTransform(animName, boneIndex, keyFrameIndex, pos.x, pos.y, pos.z, q.w, q.x, q.y, q.z);

	return pos + GetAnimGlobalPos(animName, GetSkeletonBoneParentIndex(boneIndex), keyFrameIndex);
}
#pragma endregion

#pragma region TO MOVE TO BONE CLASS
Bone GetParentRelativeBone(int parentIndex, Bone& bone)
{
	if (parentIndex <= -1) return {};
	Bone parent;
	GetSkeletonBoneLocalBindTransform(parentIndex, parent.position, parent.rotation);

	int parentsParent = GetSkeletonBoneParentIndex(parentIndex);

	if (parentsParent <= -1)
		return { parent.position + Maths::RotateVectorByQuaternion(bone.position, parent.rotation), QuaternionMultiply(parent.rotation, bone.rotation) };

	parent = GetParentRelativeBone(parentsParent, parent);
	return { parent.position + Maths::RotateVectorByQuaternion(bone.position, parent.rotation), QuaternionMultiply(parent.rotation, bone.rotation) };

	//return {};
}

void DrawBoneNode(const Bone& bone, const float size, const Maths::Vector3 color)
{
	DrawLine(bone.position + Maths::Vector3{ -size, 0.f, size }, bone.position - Maths::Vector3{ -size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(bone.position + Maths::Vector3{ size, 0.f, size }, bone.position - Maths::Vector3{ size, 0.f, size }, color.r, color.g, color.b);
	DrawLine(bone.position + Maths::Vector3{ 0.f, -size, size }, bone.position - Maths::Vector3{ 0.f, -size, size }, color.r, color.g, color.b);
	DrawLine(bone.position + Maths::Vector3{ 0.f, size, size }, bone.position - Maths::Vector3{ 0.f, size, size }, color.r, color.g, color.b);
}

Maths::mat4x4 GetBoneMatrix(const Bone& bone, int parentIndex)
{
	Maths::mat4x4 matrix = Maths::mat4::identity();

	if (parentIndex >= 0)
	{
		Bone parent;
		GetSkeletonBoneLocalBindTransform(parentIndex, parent.position, parent.rotation);
		matrix = matrix * GetBoneMatrix(parent, GetSkeletonBoneParentIndex(parentIndex));
	}

	matrix = matrix * Maths::mat4::translate(bone.position) * Maths::mat4::MakeRotationMatFromQuaternion(bone.rotation);
	
	return matrix;
}
#pragma endregion

class CSimulation : public ISimulation
{
	Maths::Vector3 skeletonDrawOffset = { 0.f, -20.f, 0.f };
	std::vector<Bone> bones;
	
	Maths::mat4x4* GetSkeletonMatrixArray()
	{
		Maths::mat4x4 matrix[64];
		for (int i = 0; i < bones.size(); i++)
		{
			matrix[i] = GetBoneMatrix(bones[i], GetSkeletonBoneParentIndex(i));
		}

		return matrix;
	}

	virtual void Init() override
	{
		//system("pause");

		int spine01 =	GetSkeletonBoneIndex("spine_01");
		int spineParent = GetSkeletonBoneParentIndex(spine01);
		const char* spineParentName = GetSkeletonBoneName(spineParent);
		size_t boneCount = GetSkeletonBoneCount();

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, keyCount / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);
		
		printf("Spine parent bone : %s\n", spineParentName);
		printf("Anim key count : %ld\n", keyCount);
		printf("Anim key : pos(%.2f,%.2f,%.2f) rotation quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);
	
		printf("Bones: ");
		for (int i = 0; (size_t)i < boneCount; i++)
		{
			//printf();
			bones.push_back(Bone());
			GetSkeletonBoneLocalBindTransform(i, bones.back().position, bones.back().rotation);
		}

		for (int i = 0; i < bones.size(); i++)
			printf("\t-%s, rot.x = %f\n", GetSkeletonBoneName(i), bones[i].rotation.x);
	}

	void DrawSkeleton()
	{
		for (int i = 0; i < bones.size(); i++)
		{
			std::string boneName = GetSkeletonBoneName(i);
			if (boneName.substr(0, 3) == "ik_")
				continue;

			int parentIndex = GetSkeletonBoneParentIndex(i);
			if (parentIndex >= 0)
			{
				Bone bone = GetParentRelativeBone(parentIndex, bones[i]);
				Bone parent = GetParentRelativeBone(GetSkeletonBoneParentIndex(parentIndex), bones[parentIndex]);
				bone.position = bone.position + skeletonDrawOffset;
				parent.position = parent.position + skeletonDrawOffset;
				
				DrawLine(bone.position, parent.position, 0.f, 0.8f, 1.f);
				DrawBoneNode(bone, 0.5f, { 1.f, 0.6f, 0.f });
				continue;
			}
			Bone bone = { bones[i].position + skeletonDrawOffset, bones[i].rotation };

			DrawBoneNode(bone, 1.f, { 1.f, 0.f, 0.f });
		}
	}

	virtual void Update(float frameTime) override
	{
		// X axis
		DrawLine(0, 0, 0, 100, 0, 0, 1, 0, 0);

		// Y axis
		DrawLine(0, 0, 0, 0, 100, 0, 0, 1, 0);

		// Z axis
		DrawLine(0, 0, 0, 0, 0, 100, 0, 0, 1);

		Bone leftLowerarm;
		GetSkeletonBoneLocalBindTransform(GetSkeletonBoneIndex("lowerarm_l"), leftLowerarm.position, leftLowerarm.rotation);

		Maths::mat4x4* skelMatrixArray = GetSkeletonMatrixArray();
		float skelMatrixFloat[64 * 16];
		
		for (int i = 0; i < 64; i++)
			for (int j = 0; j < 16; j++)
				skelMatrixFloat[i * 16 + j] = skelMatrixArray[i].e[j];

		SetSkinningPose(skelMatrixFloat, 64);

		DrawSkeleton();
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

    return 0;
}
