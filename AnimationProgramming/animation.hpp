#pragma once

#include "Maths.hpp"

#include <vector>

class KeyFrameBone
{
public:
	//key frame pos
	Vector3		pos = {};
	Quaternion	rot = {};

	//next key frame index
	int			nextKF = 0;
};

class KeyFrameSkeleton
{
public:
	std::vector<KeyFrameBone> kfBones;

	void AddKFBone(const Vector3& pos, const Quaternion& rot, const int nextKeyFrameIndex);
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
	KeyFrameBone	GetBoneFromKeyFrame(const int keyFrame, const int boneIndex) const;
};