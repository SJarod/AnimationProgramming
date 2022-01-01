#include "animation.hpp"

#include "Engine.h"
#include "EngineOverloads.h"

#include <time.h>
#include <iostream>

void KeyFrameSkeleton::AddKFBone(const Vector3& pos, const Quaternion& rot)
{
	kfBones.push_back({ pos, rot });
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

			skel.AddKFBone(pos, rot);
		}

		AddKFSkeleton(skel);
	}
}

int Animation::GetAnimationSize() const
{
	return animationSize;
}

KeyFrameBone Animation::GetKeyFrameBone(const int keyFrame, const int boneIndex) const
{
	return kfSkel[keyFrame].kfBones[boneIndex];
}

AnimationPlayer::AnimationPlayer(const Animation& anim1, const Animation& anim2)
{
	anims[0] = anim1;
	anims[1] = anim2;

	maxProgress = (unsigned int)((1.f - crossfade) * (float)anim1.GetAnimationSize() + crossfade * (float)anim2.GetAnimationSize());
}

void AnimationPlayer::UpdatePlayer()
{
	maxProgress = (unsigned int)((1.f - crossfade) * (float)anims[0].GetAnimationSize() + crossfade * (float)anims[1].GetAnimationSize());

	time += playSpeed;
	if (time >= 1.f)
	{
		time = 0.f;
		++keyFrameProgress;
		if (keyFrameProgress >= maxProgress)
			keyFrameProgress = 0;

		firstAnimKf = remap(keyFrameProgress, 0, maxProgress, 0, anims[0].GetAnimationSize());
		secondAnimKf = remap(keyFrameProgress, 0, maxProgress, 0, anims[1].GetAnimationSize());
	}
}

KeyFrameBone AnimationPlayer::GetKeyFrameBoneFromIndex(const int index, const bool next) const
{
	int firstKf = (firstAnimKf + (int)next) % anims[0].GetAnimationSize();
	int secondKf = (secondAnimKf + (int)next) % anims[1].GetAnimationSize();

	KeyFrameBone first, second;
	first = anims[0].GetKeyFrameBone(firstKf, index);
	second = anims[1].GetKeyFrameBone(secondKf, index);

	Vector3 lPos = lerp(first.pos, second.pos, crossfade);
	Quaternion lRot = slerp(first.rot, second.rot, crossfade);

	return { lPos, lRot };
}

const float& AnimationPlayer::GetTime() const
{
	return time;
}

void AnimationPlayer::SetPlaySpeed(const float& newSpeed)
{
	playSpeed = newSpeed;
}

void AnimationPlayer::SetCrossfadePercent(const float& cf)
{
	crossfade = clamp(cf, 0.f, 1.f);
}