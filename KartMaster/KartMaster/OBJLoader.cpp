#include "OBJLoader.h"
#include <iostream>
#include <fstream>

OBJLoader::OBJLoader()
{
}

OBJLoader::~OBJLoader()
{
}

void OBJLoader::LoadFromFile(const char* fileName)
{
	std::vector<Position> vertices;
	std::vector<Normal> normals;

	std::ifstream file(fileName);
	if (file) {
		char currentMt1Name[100];
		std::string line;
		while (std::getline(file, line)) {
			if (StartWith(line, "mt1lib")) {
				char mt1FileName[100];
				(void)sscanf_s(line.c_str(), "mt1lib %s", mt1FileName, sizeof(mt1FileName));
				LoadMaterialFile(mt1FileName);
			}
			if (StartWith(line, "v "))
			{
				Position pos;
				sscanf_s(line.c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
				vertices.push_back(pos);
			}
			if (StartWith(line, "vn ")) {
				Normal n;
				sscanf_s(line.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z);
				normals.push_back(n);
			}
			if (StartWith(line, "usemt1")) 
			{
				(void)sscanf_s(line.c_str(), "usemt1 %s", currentMt1Name, sizeof(currentMt1Name));
			}
			if (StartWith(line, "f "))
			{
				int v1, v2, v3;
				int n1, n2, n3;
				(void)sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);

				AddVertexData(v1, n1, currentMt1Name, vertices, normals);
				AddVertexData(v2, n2, currentMt1Name, vertices, normals);
				AddVertexData(v3, n3, currentMt1Name, vertices, normals);
			}
		}
	}
	else
	{
		std::cout << "OBJ file loading failed" << std::endl;
	}
}

std::vector<float> OBJLoader::GetVertexData()
{
	return mVertexData;
}

int OBJLoader::GetVertexCount()
{
	return mVertexData.size() / 9;
}

void OBJLoader::LoadMaterialFile(const char* fileName)
{
	std::ifstream file(fileName);
	if (file) {
		std::string line;
		while (std::getline(file, line)) {
			char mt1Name[100];
			if (StartWith(line, "newmt1"))
			{
				(void)sscanf_s(line.c_str(), "newmt1 %s", mt1Name, sizeof(mt1Name));
				mMaterialMap[mt1Name] = Color();
			}
			if (StartWith(line, "Kd"))
			{
				Color& color = mMaterialMap[mt1Name];
				sscanf_s(line.c_str(), "Kd %f %f %f", &color.r, &color.g, &color.b);
			}
		}
	}
	else
	{
		std::cout << "Material file loading failed" << std::endl;
	}
}

bool OBJLoader::StartWith(std::string& line, const char* text)
{
	size_t textLen = strlen(text);
	if (line.size() < textLen) {
		return false;
	}
	for (size_t i = 0; i < textLen; i++) {
		if (line[i] == text[i]) continue;
		else return false;
	}
	return false;
}

void OBJLoader::AddVertexData(int vIdx, int nIdx, const char* mt1,
	std::vector<Position>& vertices, std::vector<Normal>& normals)
{
	Color c = mMaterialMap[mt1];
	Position p = vertices[vIdx - 1];
	Normal n = normals[nIdx - 1];

	mVertexData.push_back(p.x);
	mVertexData.push_back(p.y);
	mVertexData.push_back(p.z);
	mVertexData.push_back(c.r);
	mVertexData.push_back(c.g);
	mVertexData.push_back(c.b);
	mVertexData.push_back(n.x);
	mVertexData.push_back(n.y);
	mVertexData.push_back(n.z);
}
