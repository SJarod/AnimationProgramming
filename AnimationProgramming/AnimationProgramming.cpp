// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Engine.h"
#include "Simulation.h"
#include "Maths.hpp"

//#include <cstdlib>

void GetSkeletonBoneLocalBindTransform(int boneIndex, Maths::Vector3& pos, Maths::Quaternion& q)
{
	GetSkeletonBoneLocalBindTransform(boneIndex, pos.x, pos.y, pos.z, q.x, q.y, q.z, q.w);
}

void GetAnimLocalBoneTransform(const char* animName, int boneIndex, int keyFrameIndex, Maths::Vector3& pos, Maths::Quaternion& q)
{
	GetAnimLocalBoneTransform(animName, boneIndex, keyFrameIndex, pos.x, pos.y, pos.z, q.x, q.y, q.z, q.w);
}

void DrawLine(Maths::Vector3 pos1, Maths::Vector3 pos2, float r, float g, float b)
{
	DrawLine(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z, r, g, b);
}

class CSimulation : public ISimulation
{
	Maths::Vector3 skeletonDrawOffset = { 0.f, -100.f, 0.f };
	
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
		printf("Anim key : pos(%.2f,%.2f,%.2f) rotation quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);
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
			int parentIndex = GetSkeletonBoneParentIndex(i);
			if (parentIndex != -1)
			{
				Maths::Vector3 parentPos, bonePos;
				Maths::Quaternion parentRot, boneRot;
				GetSkeletonBoneLocalBindTransform(parentIndex, parentPos, parentRot);
				GetSkeletonBoneLocalBindTransform(i, bonePos, boneRot);

				bonePos = bonePos + skeletonDrawOffset;
				parentPos = parentPos + skeletonDrawOffset;
				DrawLine(bonePos, parentPos, 0.f, 0.8f, 1.f);
				continue;
			}
			Maths::Vector3 bonePos;
			Maths::Quaternion boneRot;
			GetSkeletonBoneLocalBindTransform(i, bonePos, boneRot);
			bonePos = bonePos + skeletonDrawOffset;
			DrawLine(bonePos + Maths::Vector3{-1.f, 0.f, 1.f}, bonePos - Maths::Vector3{-1.f, 0.f, 1.f}, 1.f, 0.f, 0.f);
			DrawLine(bonePos + Maths::Vector3{1.f, 0.f, 1.f}, bonePos - Maths::Vector3{1.f, 0.f, 1.f}, 1.f, 0.f, 0.f);

		}
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

    return 0;
}

