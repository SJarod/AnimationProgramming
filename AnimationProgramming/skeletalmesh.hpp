#pragma once

#include "Maths.hpp"

#include <vector>

class Bone
{
public:
	std::string	name = "";

	Vector3		pos = {};
	Quaternion	rot = {};

	int			parent = -1;

	void DrawBoneNode(const float size, const Maths::Vector3 color);

	Maths::mat4x4 GetBoneMatrix(int parentIndex);
};

class SkeletalMesh
{
private:
	std::vector<Bone> bones;

public:
	void AddBone(const Bone& bone);
	void AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent = -1);
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
			printf("\t%d - %s\n", i, bones[i].name.c_str());
	}
	void DrawSkeleton(const Maths::Vector3& skeletonDrawOffset);
	void UpdateSkeleton();

	Maths::mat4x4*	GetSkeletonMatrixArray();
	float*			GetSkeletonMatrixFloat();
};