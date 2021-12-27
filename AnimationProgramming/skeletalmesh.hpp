#pragma once

#include "Maths.hpp"

#include <vector>

#include "animation.hpp"

class Bone
{
public:
	std::string	name = "";

	//bind pos, t-pose pos
	Vector3		bindPos = {};
	Quaternion	bindRot = {};
	//current pos
	Vector3		pos = {};
	Quaternion	rot = {};

	int			parent = -1;

	void DrawBoneNode(const float size, const Maths::Vector3 color);
};

class SkeletalMesh
{
private:
	std::vector<Bone> bones;

public:
	void AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent = -1);

	void PlayAnimation(const Animation& anim, const float& playSpeed);

	//returns the number of bones
	unsigned int	GetSkeletonSize() const;

	void			SetLocalBoneFromIndex(const int index, const Vector3& pos, const Quaternion& rot);
	//returns a bone with local transform
	Bone			GetLocalBoneFromIndex(const int index) const;
	//returns a bone with global transform
	Bone			GetGlobalBoneFromIndex(const int index) const;

	const char*		GetBoneNameFromIndex(const int index) const;

	void PrintSkeleton() {
		printf("Bones: ");
		for (unsigned int i = 0; i < GetSkeletonSize(); i++)
			printf("\t-%d: %s\n", i, bones[i].name.c_str());
	}
	void DrawSkeleton(const Maths::Vector3& skeletonDrawOffset);
	void UpdateSkeleton(float deltaTime = 1/60.f);

	Maths::mat4x4	GetBoneMatrix(int index, bool getInRestSkeleton = false);
	void 			GetSkeletonMatrixArray(Maths::mat4x4* matrix);
	void			GetSkeletonMatrixFloat(float* fMatrix);
};