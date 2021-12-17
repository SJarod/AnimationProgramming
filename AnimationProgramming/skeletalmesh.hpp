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

};

class SkeletalMesh
{
private:
	std::vector<Bone> bones;
	std::vector<Bone> restBones;

public:
	void AddBone(const Bone& bone);
	void AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent = -1);
	//returns the number of bones
	unsigned int GetSkeletonSize() const;
	void SetRestBones() 
	{
		for (Bone bone : bones)
			restBones.push_back(bone);
	}

	void			SetLocalBoneFromIndex(const int index, const Vector3& pos, const Quaternion& rot);
	//returns a bone with local transform
	Bone			GetLocalBoneFromIndex(const int index) const;
	//returns a bone with global transform
	Bone			GetGlobalBoneFromIndex(const int index) const;

	const char*		GetBoneNameFromIndex(const int index) const;

	void PrintSkeleton() {
		printf("Bones: ");
		for (int i = 0; i < bones.size(); i++)
			printf("\t-%d: %s\n", i, bones[i].name.c_str());
	}
	void DrawSkeleton(const Maths::Vector3& skeletonDrawOffset);
	void UpdateSkeleton(float deltaTime = 1/60.f);

	Maths::mat4x4 GetBoneMatrix(int index, bool getInRestSkeleton = false);
	Maths::mat4x4*	GetSkeletonMatrixArray();
	float*			GetSkeletonMatrixFloat();
};
