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
};

class SkeletalMesh
{
private:
	std::vector<Bone> bones;

public:
	void			AddBone(const std::string& name, const Vector3& pos, const Quaternion& rot, const int parent = -1);
	//returns the number of bones
	unsigned int	GetSkeletonSize() const;

	//returns a bone with local transform
	Bone			GetLocalBoneFromIndex(const int index) const;
	//returns a bone with global transform
	Bone			GetGlobalBoneFromIndex(const int index) const;

	const char*		GetBoneNameFromIndex(const int index) const;
};