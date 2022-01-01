#pragma once

#include "Maths.hpp"

#include <vector>

class KeyFrameBone
{
public:
	//key frame pos
	Vector3		pos = {};
	Quaternion	rot = {};
};

class KeyFrameSkeleton
{
public:
	std::vector<KeyFrameBone> kfBones;

	void AddKFBone(const Vector3& pos, const Quaternion& rot);
};

class Animation
{
private:
	std::string name = "";
	int			animationSize = 0;

	std::vector<KeyFrameSkeleton> kfSkel;

	void AddKFSkeleton(const KeyFrameSkeleton& skel);

public:
	void LoadAnimation(const char* filename, const int skeletonSize);

	//animation's key frames count, count starts from 1 not 0
	int				GetAnimationSize() const;
	KeyFrameBone	GetKeyFrameBone(const int keyFrame, const int boneIndex) const;
};

class AnimationPlayer
{
private:
	float			time = 0.f;
	float			playSpeed = 0.1f;
	
	//from 0 to [max animation size]
	unsigned int	keyFrameProgress = 0;
	unsigned int	maxProgress = 100;

	unsigned int	firstAnimKf = 0;
	unsigned int	secondAnimKf = 0;

	// 0.f is first animation, 1.f is second animation
	float			crossfade = 0.f;
	Animation		anims[2];

public:
	AnimationPlayer() = default;
	AnimationPlayer(const Animation& anim1, const Animation& anim2);

	void			UpdatePlayer();
	//index : bone index, next : take next key frame
	KeyFrameBone	GetKeyFrameBoneFromIndex(const int index, const bool next = false) const;

	const float&	GetTime() const;
	void			SetPlaySpeed(const float& newSpeed);
	void			SetCrossfadePercent(const float& cf);
};