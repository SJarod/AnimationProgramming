#pragma once

#include "Maths.hpp"

#include <vector>

class Bone
{
public:
	std::string	name = "";

	Vector3		localPos = {};
	Quaternion	localRot = {};

	Vector3		globalPos = {};
	Quaternion  globalRot = {};

	int			parent = -1;
};

class SkeletalMesh
{
private:
	std::vector<Bone> bones;

public:
	void AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent = -1);
	//returns the number of bones
	unsigned int GetSkeletonSize() const;

	Bone GetBoneFromIndex(const int index) const;

	const char* GetBoneNameFromIndex(const int index) const;
};