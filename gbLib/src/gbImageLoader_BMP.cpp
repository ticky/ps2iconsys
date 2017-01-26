/**
 * @file src/gbImageLoader.cpp
 *
 * ghulbusUtil - A collection of useful stuff
 *
 * @brief gbUtil Image Loader BMP implementation
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#include "../include/gbImageLoader.hpp"

namespace GhulbusUtil {
	gbImageLoader::gbImageType* gbImageType_BMP() {
		static gbImageType_BMP_T img_type;
		return (&img_type);
	}
	///////////
	//  BMP  //
	///////////
	/*
	 * currently supported: uncompressed 1,4,8,24,32 bits
	 *					   bitfields 32 bits
	 * todo: 16 bit uncompressed & bitfields; compression BI_RLE8; compression BI_RLE4;
	 */
#ifdef WIN32
#	include <pshpack2.h>
#endif
	//this struct must not be packed!
	struct BITMAPFILEHEADER {
		unsigned short bfType;
		unsigned int   bfSize;
		unsigned short bfReserved1;
		unsigned short bfReserved2;
		unsigned int   bfOffBits;
	}
#ifndef WIN32
	__attribute__ ((packed));
#else
	;
#	include <poppack.h>
#endif
	struct BITMAPINFOHEADER {
		unsigned int   biSize;
		int            biWidth;
		int            biHeight;
		unsigned short biPlanes;
        unsigned short biBitCount;
        unsigned int   biCompression;
        unsigned int   biSizeImage;
        int            biXPelsPerMeter;
        int            biYPelsPerMeter;
        unsigned int   biClrUsed;
        unsigned int   biClrImportant;
	};

	/** Helper function: Flips the image in Y
	 */
	static void FlipImageVertical(BITMAPINFOHEADER const& iheader, unsigned char* data) {
		//Flip image in Y:
		int p = (iheader.biBitCount >= 8)?(iheader.biBitCount >> 3):1;
		for(int y=0; y<iheader.biHeight/2; y++) {
			for(int x=0; x<iheader.biWidth; x++) {
				for(int q=0; q<p; q++) {			//loop over different color channels
					data[(y*iheader.biWidth + x) * p + q] 
						^= data[((((iheader.biHeight-1)-y)*iheader.biWidth) + x) * p + q]
						^= data[(y*iheader.biWidth + x) * p + q] 
						^= data[((((iheader.biHeight-1)-y)*iheader.biWidth) + x) * p + q];
				}
			}
		}
	}

#ifdef WIN32
	//don't bother about the unreferenced parameter warnings
#	pragma warning(disable: 4100)
#endif
	/** Helper function: Reads image data from 1bpp BMP file
	 */
	static void ReadData1Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		switch(iheader.biCompression)
			{
			case 0:					//uncompressed rgb (1 bit, palettized)
				(*palette) = new unsigned int[2];
				file.read( reinterpret_cast<char*>(*palette), 8 );
				file.seekg( file_offset+fheader.bfOffBits, ::std::ios::beg );
				file.read( reinterpret_cast<char*>(data), (iheader.biWidth*iheader.biHeight)>>3 );
				//unpack data (currently each byte describes eight consecutive pixels):
				for(int i=(iheader.biWidth*iheader.biHeight)-1; i>=7; i-=8) {
					data[i]   = (data[(i>>3)] & 0x1);
					data[i-1] = (data[(i>>3)] & 0x2)  >> 1;
					data[i-2] = (data[(i>>3)] & 0x4)  >> 2;
					data[i-3] = (data[(i>>3)] & 0x8)  >> 3;
					data[i-4] = (data[(i>>3)] & 0x10) >> 4;
					data[i-5] = (data[(i>>3)] & 0x20) >> 5;
					data[i-6] = (data[(i>>3)] & 0x40) >> 6;
					data[i-7] = (data[(i>>3)] & 0x80) >> 7;
				}
				break;
			default:
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
			}
	}

	/** Helper function: Reads image data from 4bpp BMP file
	 */
	static void ReadData4Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		switch(iheader.biCompression)
		{
		case 0:						//uncompressed rgb (4 bits, palettized)
			(*palette) = new unsigned int[16];
			file.read( reinterpret_cast<char*>(*palette), 64 );
			file.seekg( file_offset+fheader.bfOffBits, ::std::ios::beg );
			file.read( reinterpret_cast<char*>(data), (iheader.biWidth*iheader.biHeight)>>1 );
			//unpack data (currently each byte describes two consecutive pixels):
			for(int i=(iheader.biWidth*iheader.biHeight)-1; i>0; i-=2) {
				data[i]   = (data[i>>1] & 0xF);			//Lower half-byte
				data[i-1] = (data[i>>1] & 0xF0) >> 4;	//Upper half-byte
			}
			break;
		default:
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}
	}

	/** Helper function: Reads image data from 8bpp BMP file
	 */
	static void ReadData8Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		switch(iheader.biCompression)
		{
		case 0:						//uncompressed rgb (8 bit, palettized)
			(*palette) = new unsigned int[256];
			file.read( reinterpret_cast<char*>(*palette), 1024 );
			file.seekg( file_offset+fheader.bfOffBits, ::std::ios::beg );
			file.read( reinterpret_cast<char*>(data), iheader.biWidth*iheader.biHeight );
			break;
		default:
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}
	}

	/** Helper function: Reads image data from 16bpp BMP file
	 */
	static void ReadData16Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
	}

	/** Helper function: Reads image data from 24bpp BMP file
	 */
	static void ReadData24Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		switch(iheader.biCompression)
		{
		case 0:						//uncompressed rgb (24 bits)
			//read image data:
			file.seekg( file_offset+fheader.bfOffBits, ::std::ios::beg );
			file.read( reinterpret_cast<char*>(data), iheader.biWidth*iheader.biHeight*4 );
			break;
		default:
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}
	}

	/** Helper function: Reads image data from 32bpp BMP file
	 */
	static void ReadData32Bit(std::ifstream& file, BITMAPINFOHEADER const& iheader, BITMAPFILEHEADER const& fheader, 
		                     unsigned char* data, unsigned int** palette, unsigned int file_offset)
	{
		switch(iheader.biCompression)
		{
		case 0:						//uncompressed rgb (32 bit)
		case 3:						//uncompressed rgb with color masks (32 bits)
			//file uses 3 dword color masks:
			unsigned int colormasks[3];
			if(iheader.biCompression == 0) {
				colormasks[0] = 0x00FF0000;		//R
				colormasks[1] = 0x0000FF00;		//G
				colormasks[2] = 0x000000FF;		//B
			} else {
				file.read( reinterpret_cast<char*>(colormasks), 12 );
			}
			//read image data:
			file.seekg( file_offset+fheader.bfOffBits, ::std::ios::beg );
			file.read( reinterpret_cast<char*>(data), iheader.biWidth*iheader.biHeight*4 );
			unsigned int tmp, *ptmp;
			for(int i=0; i<iheader.biWidth*iheader.biHeight; i++) {		//apply color masks
				ptmp = (unsigned int*)(&data[i*4]);
				tmp = *ptmp;
				*ptmp = ( ((((tmp & colormasks[0]) / ((colormasks[0]>>8)+1)) & 0xff) << 16) |
						  ((((tmp & colormasks[1]) / ((colormasks[1]>>8)+1)) & 0xff) <<  8) |
						  ((((tmp & colormasks[2]) / ((colormasks[2]>>8)+1)) & 0xff)      ) );
			}
			break;
		default:
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}
	}
#ifdef WIN32
#	pragma warning(default: 4100)
#endif

	gbImageType_BMP_T::gbImageType_BMP_T() 
		: m_file_offset(0)
	{
		;
	}
	gbImageType_BMP_T::~gbImageType_BMP_T()
	{
		;
	}
	bool gbImageType_BMP_T::CheckFile(std::ifstream const& file) 
	{
		///@todo
		return true;
	}
	void gbImageType_BMP_T::ReadFile(std::ifstream& file, int* width, int* height, int* bpp, 
		                           unsigned char** pp_data, unsigned int** pp_palette) 
	{
		BITMAPFILEHEADER fheader;
		BITMAPINFOHEADER iheader;
		file.seekg(m_file_offset, ::std::ios::beg);
		file.read( reinterpret_cast<char*>(&fheader), sizeof(BITMAPFILEHEADER) );
		file.read( reinterpret_cast<char*>(&iheader), sizeof(BITMAPINFOHEADER));

		if(iheader.biClrUsed || (iheader.biHeight < 0)) { 
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}

		unsigned int* palette = NULL;
		unsigned char* data = new unsigned char[iheader.biWidth*iheader.biHeight*((iheader.biBitCount >= 8)?(iheader.biBitCount>>3):1)];

		try {
			switch(iheader.biBitCount)
			{
			case 1:
				ReadData1Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			case 4:
				ReadData4Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			case 8:
				ReadData8Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			case 16:
				ReadData16Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			case 24:
				ReadData24Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			case 32:
				ReadData32Bit(file, iheader, fheader, data, &palette, m_file_offset);
				break;
			default:
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
											 "Illegal bit depth in BMP image file") );
			}
		} catch(Ghulbus::gbException) {
			if(data)    { delete[] data; }
			if(palette) { delete[] palette; }
			throw;
		}

		FlipImageVertical(iheader, data);

		*width      = iheader.biWidth;
		*height     = iheader.biHeight;
		*bpp        = iheader.biBitCount;
		*pp_data    = data;
		*pp_palette = palette;
	}
};
