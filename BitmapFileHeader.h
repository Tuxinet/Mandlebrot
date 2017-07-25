
#ifndef BITMAPFILEHEADER_H_
#define BITMAPFILEHEADER_H_


#pragma pack(2)			//fjerner padding i memory

struct BitmapFileHeader
{
	char header[2]{'B', 'M'};
	int32_t fileSize;
	int32_t reserved{ 0 };
	int32_t dataOffset;				//how long into the file the data actually begins
};

#endif // !BITMAPFILEHEADER_H_
