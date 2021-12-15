#pragma once

#include "Maths.hpp"

void GetSkeletonBoneLocalBindTransform(int boneIndex, Maths::Vector3& pos, Maths::Quaternion& q);

void GetAnimLocalBoneTransform(const char* animName, int boneIndex, int keyFrameIndex, Maths::Vector3& pos, Maths::Quaternion& q);

void DrawLine(Maths::Vector3 pos1, Maths::Vector3 pos2, float r, float g, float b);