#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>

class Texture2D
{
public:
	Texture2D();
	~Texture2D();

	bool Load(std::string path, int width, int height);
	bool LoadTGA(std::string path);

	GLuint GetID() const
	{
		return _ID;
	}

	int GetWidth() const
	{
		return _width;
	}

	int GetHeight() const
	{
		return _height;
	}

	static bool BmpToRaw(std::string path);

private:
	GLuint _ID;
	int _width;
	int _height;
};