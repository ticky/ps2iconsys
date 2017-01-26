/**
 * @file src/gbImageLoader.cpp
 *
 * ghulbusUtil - A collection of useful stuff
 *
 * @brief gbUtil Image Loader implementation
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#include "../include/gbImageLoader.hpp"

namespace GhulbusUtil {
	gbImageLoader::gbImageLoader(char const* fname, gbImageType* img_type)
		:m_data(NULL), m_palette(NULL), m_width(0), m_height(0), m_bpp(0)
	{
		std::ifstream file( fname, std::ios_base::binary| std::ios_base::in );
		if( file.fail() ) {
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
				                         "Image file could not be opened" ) );
		}		
		if( !img_type->CheckFile(file) ) {
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
				                         "Image file seems to be corrupted" ) );
		}

		img_type->ReadFile(file, &m_width, &m_height, &m_bpp, &m_data, &m_palette);
	}

	gbImageLoader::~gbImageLoader() {
		if(m_palette) { delete[] m_palette;  m_palette = NULL; }
		if(m_data)    { delete[] m_data;     m_data    = NULL; }
	}

	int gbImageLoader::GetWidth() const {
		return m_width;
	}
	int gbImageLoader::GetHeight() const {
		return m_height;
	}
	int gbImageLoader::GetBpp() const {
		return m_bpp;
	}
	bool gbImageLoader::HasPalette() const {
		return (m_palette != NULL);
	}
	void gbImageLoader::GetImageData(unsigned char* pData) const {
		memcpy(pData, m_data, (m_width*m_height*m_bpp));
	}
	void gbImageLoader::GetPaletteData(unsigned int* pPal) const {
		switch(m_bpp) {
			case 1:
				memcpy(pPal, m_palette, 2 * 4);
				break;
			case 4:
				memcpy(pPal, m_palette, 16 * 4);
				break;
			case 8:
				memcpy(pPal, m_palette, 256 * 4);
				break;
			default:
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
					                         "Unable to acquire palette data" ) );
				break;
		}
	}
	void gbImageLoader::GetImageData32(GhulbusGraphics::GBCOLOR* pData) const {
		switch(m_bpp) {
			case 32:
				//32 bit image: one byte per color-channel, one byte alpha:
				for(int i=0; i<(m_width*m_height); i++) {
					pData[i] = GhulbusGraphics::GBCOLOR32::ARGB(m_data[i*4+3], m_data[i*4+2], m_data[i*4+1], m_data[i*4]);
				}
				break;
			case 24:
				//24 bit image: one byte per color-channel, no alpha channel
				for(int i=0; i<(m_width*m_height); i++) {
					pData[i] = GhulbusGraphics::GBCOLOR32::XRGB(m_data[i*3+2], m_data[i*3+1], m_data[i*3]);
				}
				break;
			case 16:
				//16 bit image: 5 bit per color channel, 1 bit alpha channel (unsupported)
				for(int i=0; i<(m_width*m_height); i++) {
					GhulbusGraphics::GBCOLOR_COMPONENT r,g,b,a;
					r =  (m_data[i*2+1] & 0x7C) >> 2;
					g = ((m_data[i*2]   & 0xE0) >> 5) | ((m_data[i*2+1] & 0x03) << 3);
					b =  (m_data[i*2]   & 0x1F);
					a =  (m_data[i*2+1] & 0x80) >> 7;
					a = 1;
					///@todo alpha channel
					///@todo dithering(?)
					pData[i] = GhulbusGraphics::GBCOLOR32::ARGB(a*255,r*8,g*8,b*8);
				}
				break;
			case 8: case 4: case 1:
				//one byte palette index per pixel
				for(int i=0; i<(m_width*m_height); i++) {
					GhulbusGraphics::GBCOLOR_COMPONENT r,g,b,a;
					r = static_cast<GhulbusGraphics::GBCOLOR_COMPONENT>((m_palette[m_data[i]] >> 16) & 0xff);
					g = static_cast<GhulbusGraphics::GBCOLOR_COMPONENT>((m_palette[m_data[i]] >>  8) & 0xff);
					b = static_cast<GhulbusGraphics::GBCOLOR_COMPONENT>((m_palette[m_data[i]])       & 0xff);
					a = static_cast<GhulbusGraphics::GBCOLOR_COMPONENT>((m_palette[m_data[i]] >> 24) & 0xff);
					pData[i] = GhulbusGraphics::GBCOLOR32::ARGB(a, r, g, b);
				}
				///@todo dithering(?)
				break;
			default:
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
					                         "Unable to acquire 32bit image data" ) );
				break;
		}
	}

	gbImageLoader::gbImageType::~gbImageType() {
		;
	}

	void WriteImage(char const* fname, GhulbusGraphics::GBCOLOR const* data, int width, int height)
	{
		struct {
			unsigned char  nCharIDField;		// Number of Chars in ID Field (1 Byte)
			unsigned char  ColorMapType;		// is 1 if color map specified; must be 0 for us
			unsigned char  ImageTypeCode;		// Data Type; must be 2 for us
			unsigned char  ColorMapSpec[5];		// should be 0
			unsigned short XOrigin;				// expected to be 0
			unsigned short YOrigin;				// expected to be 0
			unsigned short Width;				// Picture Width
			unsigned short Height;				// Picture Height
			unsigned char  ImagePixelSize;		// Bits per Pixel
			unsigned char  ImageDescByte;		// Image Descriptor Byte
		} header;

		memset(&header, 0, sizeof(header));
		header.ImageTypeCode = 2;
		if((width > USHRT_MAX) || (height > USHRT_MAX)) { 
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
		}
		header.Width  = static_cast<unsigned short>(width);
		header.Height = static_cast<unsigned short>(height);
		header.ImagePixelSize = 32;
		header.ImageDescByte = 0x28;		//magic number: 8 bits per channel, origin upper left, no padding

		std::ofstream  fout( fname, std::ios_base::out | std::ios_base::binary );
		if(fout.fail()) {
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED, "Output file could not be opened" ) );
		}
		fout.write( reinterpret_cast<char*>(&header), sizeof(header) );
		
		for(int i=0; i<width*height; ++i) {
			unsigned char a = GhulbusGraphics::GBCOLOR32::GetA(data[i]);
			unsigned char r = GhulbusGraphics::GBCOLOR32::GetR(data[i]);
			unsigned char g = GhulbusGraphics::GBCOLOR32::GetG(data[i]);
			unsigned char b = GhulbusGraphics::GBCOLOR32::GetB(data[i]);
			fout << b << g << r << a;
		}
		if(fout.fail()) {
			throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED, "Error while writing to output file" ) );
		}
	}

	void gbImageLoader::FlipV() {
		int pitch;		//fields per row
		switch(m_bpp) {
			case 32: case 24: case 16: case 8: 
				pitch = (m_bpp / 8) * m_width;
				break;
			case 4: case 1:
				pitch = m_width;
				break;
			default:
				throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) );
				break;
		}
		for(int row=0; row<(m_height/2); row++) {
			for(int i=0; i<pitch; i++) {
				m_data[row * pitch + i] ^= m_data[((m_height-1) - row) * pitch + i]
					^= m_data[row * pitch + i] ^= m_data[((m_height-1) - row) * pitch + i];
			}
		}
	}
};
