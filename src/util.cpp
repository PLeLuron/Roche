#include "util.h"

#include <string>
#include <fstream>
#include <cstring>

std::string read_file(const std::string filename)
{
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw std::runtime_error("Can't open" + filename);
	return "";
}

typedef uint32_t DWORD;

struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

struct DDS_HEADER {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
};

int DDSLoader::skipMipmap = 0;

void DDSLoader::setSkipMipmap(const int skipMipmap)
{
	DDSLoader::skipMipmap = std::max(0, skipMipmap);
}

DDSLoader::Format DDSLoader::getFormat()
{
	return format;
}

int getFormatBytesPerBlock(DDSLoader::Format format)
{
	switch (format)
	{
		case DDSLoader::Format::BC1: return 8;
		case DDSLoader::Format::BC2: return 16;
		case DDSLoader::Format::BC3: return 16;
	}
}

int getImageSize(const int width, const int height, DDSLoader::Format format)
{
	return std::max(1, (width+3)/4)*std::max(1, (height+3)/4)*getFormatBytesPerBlock(format);
}

DDSLoader::Format getFourCCFormat(char* fourCC)
{
	if (!strncmp(fourCC, "DXT1", 4))
	{
		return DDSLoader::Format::BC1;
	}
	if (!strncmp(fourCC, "DXT3", 4))
	{
		return DDSLoader::Format::BC2;
	}
	if (!strncmp(fourCC, "DXT5", 4))
	{
		return DDSLoader::Format::BC3;
	}
	return DDSLoader::Format::Undefined;
}

bool DDSLoader::open(const std::string filename)
{
	this->filename = filename;
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if (!in) return false;

	// Magic number
	in.seekg(0, std::ios::beg);
	char buf[4];
	in.read(buf, 4);
	if (strncmp(buf, "DDS ", 4))
	{
		return false;
	}

	// DDS header
	DDS_HEADER header;
	in.read((char*)&header, sizeof(DDS_HEADER));
	format = getFourCCFormat((char*)&(header.ddspf.dwFourCC));

	// Mipmap count check
	mipmapCount = (header.dwFlags&0x20000)?header.dwMipMapCount:1;

	width = header.dwWidth;
	height = header.dwHeight;

	size_t offset = 128;
	for (int i=0;i<skipMipmap;++i)
	{
		offset += getImageSize(getWidth(i), getHeight(i), format);
	}

	for (int i=skipMipmap;i<mipmapCount;++i)
	{
		int size = getImageSize(getWidth(i), getHeight(i), format);
		offsets.push_back(offset);
		sizes.push_back(size);
		offset += size;
	}

	width = width >> skipMipmap;
	height = height >> skipMipmap;
	mipmapCount -= skipMipmap;

	return true;
}

int DDSLoader::getMipmapCount()
{
	return mipmapCount;
}

int DDSLoader::getWidth(const int mipmapLevel)
{
	return std::max(1, width>>mipmapLevel);
}

int DDSLoader::getHeight(const int mipmapLevel)
{
	return std::max(1, height>>mipmapLevel);
}

void DDSLoader::getImageData(uint32_t mipmapLevel, std::vector<uint8_t> &data)
{
	if (mipmapLevel >= getMipmapCount()) return;
	else if (mipmapLevel < 0) mipmapLevel = 0;

	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if (!in) return;
	// Offset into file to get pixel data
	const int imageSize = sizes[mipmapLevel];
	const int offset = offsets[mipmapLevel];

	data.resize(imageSize);
	in.seekg(offset, std::ios::beg);
	in.read((char*)data.data(), imageSize);
}