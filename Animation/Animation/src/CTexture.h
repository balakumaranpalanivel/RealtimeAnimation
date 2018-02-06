#pragma once
// TODO: Update all name of all the data memebers of the class to preced "m"

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define TYPE_DIFFUSE_SHADER		"texture_diffuse"
#define TYPE_SPECULAR_SHADER	"texture_specular"
#define TYPE_NORMAL_SHADER		"texture_normal"

struct CTexture
{
	unsigned int nID;
	std::string sType;
	// store the path of the texture to compare with other textures
	aiString filePath;
};