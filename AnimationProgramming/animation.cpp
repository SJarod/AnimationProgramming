#include "animation.hpp"

#include "Engine.h"
#include "EngineOverloads.h"

void KeyFrameSkeleton::AddKFBone(const Vector3& pos, const Quaternion& rot, const int nextKeyFrameIndex)
{
	kfBones.push_back({ pos, rot, nextKeyFrameIndex });
}

void Animation::AddKFSkeleton(const KeyFrameSkeleton& skel)
{
	kfSkel.push_back(skel);
}

void Animation::LoadAnimation(const char* filename, const int skeletonSize)
{
	name = filename;
	animationSize = GetAnimKeyCount(filename);

	for (int kf = 0; kf < animationSize; ++kf)
	{
		KeyFrameSkeleton skel;

		for (int i = 0; i < skeletonSize; ++i)
		{
			Vector3 pos;
			Quaternion rot;
			GetAnimLocalBoneTransform(filename, i, kf, pos, rot);

			skel.AddKFBone(pos, rot, (kf + 1) % animationSize);
		}

		AddKFSkeleton(skel);
	}
}

int Animation::GetAnimationSize() const
{
	return animationSize;
}

KeyFrameBone Animation::GetBoneFromKeyFrame(const int keyFrame, const int boneIndex) const
{
	return kfSkel[keyFrame].kfBones[boneIndex];
}