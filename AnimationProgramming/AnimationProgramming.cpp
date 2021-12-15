// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Engine.h"
#include "EngineOverloads.h"
#include "Simulation.h"
#include "Maths.hpp"

#include "skeletalmesh.hpp"

class CSimulation : public ISimulation
{
	SkeletalMesh   skmesh;
	Maths::Vector3 skeletonDrawOffset = { 0.f, -20.f, 0.f };

	virtual void Init() override
	{
		//system("pause");

		int spine01 = GetSkeletonBoneIndex("spine_01");
		int spineParent = GetSkeletonBoneParentIndex(spine01);
		const char* spineParentName = GetSkeletonBoneName(spineParent);
		size_t boneCount = GetSkeletonBoneCount();

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, keyCount / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);
		
		printf("Spine parent bone : %s\n", spineParentName);
		printf("Anim key count : %ld\n", keyCount);
		printf("Anim key : pos(%.2f,%.2f,%.2f) localRot quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);
	
		for (int i = 0; (size_t)i < boneCount; i++)
		{
			Bone bone;
			bone.name = GetSkeletonBoneName(i);

			if (bone.name.substr(0, 3) == "ik_")
				continue;

			GetSkeletonBoneLocalBindTransform(i, bone.pos, bone.rot);
			bone.parent = GetSkeletonBoneParentIndex(i);

			skmesh.AddBone(bone);
		}
		
		skmesh.PrintSkeleton();

		for (int i = 0; i < (int)GetSkeletonBoneCount(); ++i)
		{
			Vector3 pos;
			Quaternion rot;
			GetSkeletonBoneLocalBindTransform(i, pos, rot);

			skmesh.AddBone(GetSkeletonBoneName(i), pos, rot, GetSkeletonBoneParentIndex(i));
		}
	}
	
	virtual void Update(float frameTime) override
	{
		skmesh.UpdateSkeleton();

		// X axis
		DrawLine(0, 0, 0, 100, 0, 0, 1, 0, 0);

		// Y axis
		DrawLine(0, 0, 0, 0, 100, 0, 0, 1, 0);

		// Z axis
		DrawLine(0, 0, 0, 0, 0, 100, 0, 0, 1);

		Maths::mat4x4* skelMatrixArray = skmesh.GetSkeletonMatrixArray();

		float* skelMatrixFloat = skmesh.GetSkeletonMatrixFloat();
		SetSkinningPose(skelMatrixFloat, 64);

		skmesh.DrawSkeleton(skeletonDrawOffset);
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

    return 0;
}