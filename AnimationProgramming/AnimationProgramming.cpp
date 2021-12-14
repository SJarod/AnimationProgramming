// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Engine.h"
#include "Simulation.h"
#include "Maths.hpp"

//#include <cstdlib>

#include "skeletalmesh.hpp"

#pragma region ENGINE_TYPED_FUNCTIONS
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

#pragma region OTHER
Maths::Vector3 GetParentRelativePos(int parentIndex, Bone bone)
{
	Bone parent;
	GetSkeletonBoneLocalBindTransform(parentIndex, parent.localPos, parent.localRot);

	int parentsParent = GetSkeletonBoneParentIndex(parentIndex);

	if (parentsParent <= -1)
		return parent.localPos + Maths::RotateVectorByQuaternion(bone.localPos, parent.localRot);

	return GetParentRelativePos(parentsParent, parent);
}

Bone GetParentRelativeBone(int parentIndex, Bone& bone)
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
#pragma endregion


class CSimulation : public ISimulation
{
	SkeletalMesh   skmesh;
	Maths::Vector3 skeletonDrawOffset = { 0.f, -20.f, 0.f };
	
	virtual void Init() override
	{
		//system("pause");

		int spine01 =	GetSkeletonBoneIndex("spine_01");
		int spineParent = GetSkeletonBoneParentIndex(spine01);
		const char* spineParentName = GetSkeletonBoneName(spineParent);

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, keyCount / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);
		
		printf("Spine parent bone : %s\n", spineParentName);
		printf("Anim key count : %ld\n", keyCount);
		printf("Anim key : pos(%.2f,%.2f,%.2f) localRot quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);
	}

	virtual void Update(float frameTime) override
	{
		// X axis
		DrawLine(0, 0, 0, 100, 0, 0, 1, 0, 0);

		// Y axis
		DrawLine(0, 0, 0, 0, 100, 0, 0, 1, 0);

		// Z axis
		DrawLine(0, 0, 0, 0, 0, 100, 0, 0, 1);

		size_t boneCount = GetSkeletonBoneCount();

		for (int i = 0; (size_t)i < boneCount; i++)
		{
			std::string boneName = GetSkeletonBoneName(i);
			if (boneName.substr(0, 3) == "ik_")
				continue;

			int parentIndex = GetSkeletonBoneParentIndex(i);
			if (parentIndex >= 0)
			{
				Bone parent, bone;
				GetSkeletonBoneLocalBindTransform(parentIndex, parent.localPos, parent.localRot);
				GetSkeletonBoneLocalBindTransform(i, bone.localPos, bone.localRot);

 				bone = GetParentRelativeBone(parentIndex, bone);
				parent = GetParentRelativeBone(GetSkeletonBoneParentIndex(parentIndex), parent);
				bone.localPos = bone.localPos + skeletonDrawOffset; 
				parent.localPos = parent.localPos + skeletonDrawOffset;
				// draw bone
				DrawLine(bone.localPos, parent.localPos, 0.f, 0.8f, 1.f);
				// draw bone origin only
				DrawLine(bone.localPos + Maths::Vector3{ -0.5f, 0.f, 0.5f }, bone.localPos - Maths::Vector3{ -0.5f, 0.f, 0.5f }, 1.f, 0.6f, 0.f);
				DrawLine(bone.localPos + Maths::Vector3{ 0.5f, 0.f, 0.5f }, bone.localPos - Maths::Vector3{ 0.5f, 0.f, 0.5f }, 1.f, 0.6f, 0.f);
				continue;
			}
			Bone bone;
			GetSkeletonBoneLocalBindTransform(i, bone.localPos, bone.localRot);
			bone.localPos = bone.localPos + skeletonDrawOffset;

			DrawLine(bone.localPos + Maths::Vector3{ -1.f, 0.f, 1.f }, bone.localPos - Maths::Vector3{ -1.f, 0.f, 1.f }, 1.f, 0.f, 0.f);
			DrawLine(bone.localPos + Maths::Vector3{ 1.f, 0.f, 1.f }, bone.localPos - Maths::Vector3{1.f, 0.f, 1.f}, 1.f, 0.f, 0.f);

		}
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

    return 0;
}

