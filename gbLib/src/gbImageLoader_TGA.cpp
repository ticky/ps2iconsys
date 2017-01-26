/**
 * @file src/gbImageLoader.cpp
 *
 * ghulbusUtil - A collection of useful stuff
 *
 * @brief gbUtil Image Loader TGA implementation
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#include "../include/gbImageLoader.hpp"

namespace GhulbusUtil {
	gbImageLoader::gbImageType* gbImageType_TGA() {
		static gbImageType_TGA_T img_type;
		return (&img_type);
	}
	///////////
	//  TGA  //
	///////////
	/*
	 * currently supported: unmapped rgb 16, 24, 32 bit
	 */
	struct TGAHEADER {
		unsigned char  nCharIDField;		///< Number of Chars in ID Field (1 Byte)
		unsigned char  ColorMapType;		///< is 1 if color map specified; must be 0 for us
		unsigned char  ImageTypeCode;		///< Data Type; must be 2 for us
		unsigned char  ColorMapSpec[5];		///< should be 0
		unsigned short XOrigin;				///< expected to be 0
		unsigned short YOrigin;				///< expected to be 0
		unsigned short Width;				///< Picture Width
		unsigned short Height;				///< Picture Height
		unsigned char  ImagePixelSize;		///< Bits per Pixel
		unsigned char  ImageDescByte;		///< Image Descriptor Byte
	};

	/** Helper function: Flips image data in Y
	 */
	static void FlipImageVertical(TGAHEADER const& header, unsigned char* data) {
		//Flip image in Y:
		int p = header.ImagePixelSize >> 3;
		for(int y=0; y<header.Height/2; y++) {
			for(int x=0; x<header.Width; x++) {
				for(int q=0; q<p; q++) {			//loop over different color channels
					data[(y*header.Width + x) * p + q] 
						^= data[((((header.Height-1)-y)*header.Width) + x) * p + q]
						^= data[(y*header.Width + x) * p + q] 
						^= data[((((header.Height-1)-y)*header.Width) + x) * p + q];
				}
			}
		}
	}

	/** Helper function: Reads image data from unmapped rgb file
	 */
	static void ReadUnmappedRBG(std::ifstream& file, TGAHEADER const& header, unsigned char* data, unsigned int file_offset)
	{
		//Adjust file pointer to beginning of img-data:
		file.seekg(file_offset+sizeof(TGAHEADER)+header.nCharIDField, ::std::ios::beg);
		//Read image data:
		file.read(reinterpret_cast<char*>(data), (header.Width*header.Height*header.ImagePixelSize) / 8);
	}

	gbImageType_TGA_T::gbImageType_TGA_T()
		: m_file_offset(0)
	{
		;
	}

	gbImageType_TGA_T::~gbImageType_TGA_T()
	{
		;
	}

	bool gbImageType_TGA_T::CheckFile(std::ifstream const& file) {
		///@todo
		return true;
	}

	void gbImageType_TGA_T::ReadFile(std::ifstream& file, int* width, int* height, int* bpp, 
			                       unsigned char** pp_data, unsigned int** pp_palette)
	{
		TGAHEADER header;
		file.seekg(m_file_offset, ::std::ios::beg);
		file.read( reinterpret_cast<char*>(&header), sizeof(TGAHEADER));
		unsigned char* data = new unsigned char[header.Width*header.Height*header.ImagePixelSize];

		switch(header.ImageTypeCode) {
			case 2:	//Unmapped RBG
				ReadUnmappedRBG(file, header, data, m_file_offset);
				if((header.ImageDescByte & 0x20) == 0) {		//Truvision?
					FlipImageVertical(header, data);
				}
				break;
			case 1:	case 3:	case 9:	case 10: case 11: case 32: case 33:
			default:
				delete[] data;
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_NOTIMPLEMENTED ) );
		}

		if(file.fail()) {
			delete[] data;
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                             "Error while reading TGA image file" ) );
		}
				
		*width      = header.Width;
		*height     = header.Height;
		*bpp        = header.ImagePixelSize;
		*pp_data    = data;
		*pp_palette = NULL;
	}
};
