#define _CRT_SECURE_NO_WARNINGS

#include "Texture2D.h"
#include <fstream>
#include <vector>
#include <string>

using namespace std;

Texture2D::Texture2D()
{

}


Texture2D::~Texture2D()
{

}

bool Texture2D::Load(string path, int width, int height)
{
	char* tempTextureData;
	int fileSize;
	ifstream inFile;

	_width = width;
	_height = height;

	inFile.open(path, ios::binary);
	if (!inFile.good())
	{
		cerr << "Can't open texture file " << path.c_str() << endl;
		return false;
	}
	else
	{
		inFile.seekg(0, ios::end);
		fileSize = (int)inFile.tellg();
		tempTextureData = new char[fileSize];
		inFile.seekg(0, ios::beg);
		inFile.read(tempTextureData, fileSize);
		inFile.close();

		cout << path.c_str() << " loaded." << endl;

		glGenTextures(1, &_ID);
		glBindTexture(GL_TEXTURE_2D, _ID);
		//glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTextureData);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, tempTextureData);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete[] tempTextureData;
		return true;
	}
}

bool Texture2D::BmpToRaw(string path)
{
	char fileType[2];

	struct Header
	{
		uint32_t fileSize;
		uint16_t creator1;
		uint16_t creator2;
		uint32_t bmpOffset;
	};

	struct InfoHeader
	{
		uint32_t headerSize;
		int32_t width;
		int32_t height;
		uint16_t numPlanes;
		uint16_t bitsPerPixel;
		uint32_t compressionType;
		uint32_t byteSize;
		int32_t horizontalRes;
		int32_t vertcalRes;
		uint32_t numColours;
		uint32_t numImpColours;
	};

	struct Colour
	{
		char r;
		char g;
		char b;
	};

	InfoHeader headerInfo;
	char headerInfoCharArray[sizeof(InfoHeader)];

	Header header;
	char headerCharArray[sizeof(header)];

	Colour* imageBuffer;

	string newFilePath = "";

	for (int i = 0; i < path.length() - 3; i++)
	{
		newFilePath += path[i];
	}

	newFilePath += "raw";

	FILE* inFile;
	FILE* outFile;

	inFile = fopen(path.c_str(), "rb");
	outFile = fopen(newFilePath.c_str(), "wb");

	if (inFile == nullptr)
	{
		cout << path.c_str() << " could not be opened!" << endl;
		return false;
	}
	else
	{
		cout << path.c_str() << " was opened!" << endl;

		fread(fileType, sizeof(char) * 2, 1, inFile);
		fread(&header, sizeof(Header), 1, inFile);
		fread(&headerInfo, sizeof(InfoHeader), 1, inFile);

		if (headerInfo.width != headerInfo.height)
		{
			cout << "Image not a square!" << endl;
			return false;
		}
		else
		{
			imageBuffer = (Colour*)malloc(sizeof(Colour) * headerInfo.width);

			for (int row = 0; row < headerInfo.height; row++)
			{
				fread(imageBuffer, sizeof(Colour), headerInfo.width, inFile);

				for (int column = 0; column < headerInfo.width; column++)
				{
					putc(imageBuffer[column].r, outFile);
					putc(imageBuffer[column].g, outFile);
					putc(imageBuffer[column].b, outFile);
				}
			}

			fclose(inFile);
			fclose(outFile);
			free(imageBuffer);
			return true;
		}
	}
}

bool HasAlphaChannel(uint32_t _bitsPerPixel)
{
	if (_bitsPerPixel == 32)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Texture2D::LoadTGA(string path)
{
	struct PixelInfo
	{

		uint32_t Colour;

		struct
		{
			uint8_t R, G, B, A;
		};
	};

	PixelInfo* pixelInfo;

	std::vector<uint8_t> pixels;
	bool imageCompressed;
	uint32_t width, height, size, bitsPerPixel;

	std::fstream inFile(path, std::ios::in | std::ios::binary);
	if (!inFile.is_open()) 
	{ 
		cout << path.c_str() << " not found!" << endl;
		return false;
	}

	uint8_t header[18] = { 0 };
	vector<uint8_t> imageData;
	static uint8_t deCompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	static uint8_t isCompressed[12] = { 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	inFile.read(reinterpret_cast<char*>(&header), sizeof(header));

	if (!memcmp(deCompressed, &header, sizeof(deCompressed)))
	{
		bitsPerPixel = header[16];
		width = header[13] * 256 + header[12];
		height = header[15] * 256 + header[14];
		size = size = ((width * bitsPerPixel + 31) / 32) * 4 * height;

		if ((bitsPerPixel != 24) && (bitsPerPixel != 32))
		{
			inFile.close();
			cout << path.c_str() << " is an invalid file format!" << endl;
			return false;
		}

		imageData.resize(size);
		imageCompressed = false;
		inFile.read(reinterpret_cast<char*>(imageData.data()), size);
	}
	else if(!memcmp(isCompressed, &header, sizeof(isCompressed)))
	{
		bitsPerPixel = header[16];
		width = header[13] * 256 + header[12];
		height = header[15] * 256 + header[14];
		size = ((width * bitsPerPixel + 31) / 32) * 4 * height;

		if ((bitsPerPixel != 24) && (bitsPerPixel != 32))
		{
			inFile.close();
			cout << path.c_str() << " is an invalid file format!" << endl;
			return false;
		}

		PixelInfo pixel = { 0 };
		int currentByte = 0;
		size_t currentPixel = 0;
		imageCompressed = true;
		uint8_t chunkHeader = { 0 };
		int bytesPerPixel = (bitsPerPixel / 8);
		imageData.resize(width * height * sizeof(PixelInfo));

		do
		{
			inFile.read(reinterpret_cast<char*>(&chunkHeader), sizeof(chunkHeader));

			if (chunkHeader < 128)
			{
				++chunkHeader;
				for (int i = 0; i < chunkHeader; ++i, ++currentPixel)
				{
					inFile.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);

					imageData[currentByte++] = pixel.B;
					imageData[currentByte++] = pixel.G;
					imageData[currentByte++] = pixel.R;
					if (bitsPerPixel > 24) imageData[currentByte++] = pixel.A;
				}
			}
			else
			{
				chunkHeader -= 127;
				inFile.read(reinterpret_cast<char*>(&pixel), bytesPerPixel);

				for (int i = 0; i < chunkHeader; ++i, ++chunkHeader)
				{
					imageData[currentByte++] = pixel.B;
					imageData[currentByte++] = pixel.G;
					imageData[currentByte++] = pixel.R;
					if (bitsPerPixel > 24)
					{
						imageData[currentByte++] = pixel.A;
					}
				}
			}
		} while (currentPixel < (width * height));
	}
	else
	{
		inFile.close();
		cout << path.c_str() << " is an invalid file format!" << endl;
		return false;
	}

	inFile.close();
	pixels = imageData;

	glGenTextures(1, &_ID);
	glBindTexture(GL_TEXTURE_2D, _ID);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTextureData);
	//glTexImage2D(GL_TEXTURE_2D, 0, HasAlphaChannel(bitsPerPixel) ? GL_RGBA : GL_RGB, width, width, 0, HasAlphaChannel(bitsPerPixel) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, HasAlphaChannel(bitsPerPixel) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cout << path.c_str() << " Loaded!" << endl;

	return true;
}