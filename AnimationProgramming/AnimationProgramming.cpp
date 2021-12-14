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
#pragma endregion

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

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, keyCount / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);
		
		printf("Spine parent bone : %s\n", spineParentName);
		printf("Anim key count : %ld\n", keyCount);
		printf("Anim key : pos(%.2f,%.2f,%.2f) localRot quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);

		for (int i = 0; i < (int)GetSkeletonBoneCount(); ++i)
		{
			std::cout << i << " : " << GetSkeletonBoneName(i) << std::endl;

			Vector3 pos;
			Quaternion rot;
			GetSkeletonBoneLocalBindTransform(i, pos, rot);

			skmesh.AddBone(GetSkeletonBoneName(i), pos, rot, GetSkeletonBoneParentIndex(i));
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

		static float angle = 0.f;
		skmesh.SetLocalBoneFromIndex(27, skmesh.GetLocalBoneFromIndex(27).pos, QuaternionFromAxisAngle({ 1, 0, 0 }, angle));
		angle += 0.01f;
		skmesh.SetLocalBoneFromIndex(6, skmesh.GetLocalBoneFromIndex(6).pos, QuaternionFromAxisAngle({ 1, 0, 0 }, -angle));

		for (unsigned int i = 0; i < skmesh.GetSkeletonSize(); ++i)
		{
			std::string boneName = skmesh.GetBoneNameFromIndex(i);
			if (boneName.substr(0, 3) == "ik_")
				continue;

			int parentIndex = skmesh.GetLocalBoneFromIndex(i).parent;

			if (parentIndex >= 0)
			{
				Bone bone = skmesh.GetGlobalBoneFromIndex(i);
				Bone parent = skmesh.GetGlobalBoneFromIndex(parentIndex);
				bone.pos = bone.pos + skeletonDrawOffset;
				parent.pos = parent.pos + skeletonDrawOffset;

				// draw bone
				DrawLine(bone.pos, parent.pos, 0.f, 0.8f, 1.f);
				// draw bone origin only
				DrawLine(bone.pos + Maths::Vector3{ -0.5f, 0.f, 0.5f }, bone.pos - Maths::Vector3{ -0.5f, 0.f, 0.5f }, 1.f, 0.6f, 0.f);
				DrawLine(bone.pos + Maths::Vector3{ 0.5f, 0.f, 0.5f }, bone.pos - Maths::Vector3{ 0.5f, 0.f, 0.5f }, 1.f, 0.6f, 0.f);
				continue;
			}

			Bone bone = skmesh.GetGlobalBoneFromIndex(i);
			bone.pos = bone.pos + skeletonDrawOffset;

			DrawLine(bone.pos + Maths::Vector3{ -1.f, 0.f, 1.f }, bone.pos - Maths::Vector3{ -1.f, 0.f, 1.f }, 1.f, 0.f, 0.f);
			DrawLine(bone.pos + Maths::Vector3{ 1.f, 0.f, 1.f }, bone.pos - Maths::Vector3{1.f, 0.f, 1.f}, 1.f, 0.f, 0.f);
		}
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

    return 0;
}