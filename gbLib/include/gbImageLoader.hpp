/**
 * @file include/gbImageLoader.hpp
 *
 * ghulbusUtil - A collection of useful stuff
 *
 * @brief gbUtil Image Loader
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */

#ifndef _GHULBUSUTIL_IMAGELOADER_HPP_INCLUDE_GUARD_
#define _GHULBUSUTIL_IMAGELOADER_HPP_INCLUDE_GUARD_

#include <fstream>

#include "gbException.hpp"
#include "gbColor.hpp"

/** Root namespace of the ghulbusUtil Library
 */
namespace GhulbusUtil {
	/** Used for loading image files
	*/
	class gbImageLoader {
	public:
		/** Interface for image file loaders
		 */
		class gbImageType {
		public:
			/** Read image data from file
			 * @param[in] file           An open filestream to the image file
			 * @param[out] width         The image's width in pixels
			 * @param[out] height        The image's height in pixels
			 * @param[out] bpp           Bits per pixel
			 * @param[in,out] pp_data    A field containing the image data; Memory will be allocated by ReadFile() itself! 
			 * @param[in,out] pp_palette A field containing palette data; Memory will be allocated by ReadFile() itself! 
			 * @throw std::bad_alloc
			 */
			virtual void ReadFile(std::ifstream& file, int* width, int* height, int* bpp, 
								unsigned char** pp_data, unsigned int** pp_palette)=0;
			/** Check if the file is of a specific image type
			 * @param[in] file An open filestream to the image file
			 * @return True if the file can be read using the current image type, false otherwise 
			 */
			virtual bool CheckFile(std::ifstream const& file)=0;
			/** Destructor
			 */
			virtual ~gbImageType();
		};
	private:
		unsigned char* m_data;			///< Pointer to image data
		unsigned int* m_palette;		///< Pointer to color palette
		int m_width;					///< Image width (pixels)
		int m_height;					///< Image height (pixels)
		int m_bpp;						///< Bits per pixel
	public:
		/** Constructor
		 * @param[in] fname Full path to the image file that is to be loaded
		 * @param[in,out] img_type The image type loading strategy, specified as gbImageType object;
		 * @throw Ghulbus::gbException GB_FAILED usually indicates a file read error; 
		 *                             GB_NOTIMPLEMENTED;
		 * @throw std::bad_alloc
		 */
		gbImageLoader(char const* fname, gbImageType* img_type);
		/** Destructor
		 */
		~gbImageLoader();
		/** Get the image's width
		 * @return Image width in pixels
		 */
		int GetWidth() const;
		/** Get the image's height
		 * @return Image height in pixels
		 */
		int GetHeight() const;
		/** Get the image's color depth
		 * @return Bits per pixel in the image
		 */
		int GetBpp() const;
		/** Check whether the image has a palette
		 * @return True if a palette is present, false otherwise
		 */
		bool HasPalette() const;
		/** Get the image data as read from the file
		 * @param[out] pData A field of at least size width*height*bpp
		 */
		void GetImageData(unsigned char* pData) const;
		/** Get the image data in unmapped GBCOLORs
		 * @param[out] pData A field of at least size width*height
		 * @throw Ghulbus::gbException GB_FAILED indicates an unexpected color depth
		 */
		void GetImageData32(GhulbusGraphics::GBCOLOR* pData) const;
		/** Get the palette data
		 * @param[out] pPal A field of at least size (2^bpp)
		 * @throw Ghulbus::gbException GB_FAILED indicates that no palette data is present
		 */
		void GetPaletteData(GhulbusGraphics::GBCOLOR* pPal) const;
		/** Flips the image vertically
		 */
		void FlipV();
	};

	/** Writes image data to a TGA image file
	 * @param[in] fname Full path to the output file
	 * @param[in] data Field containing the image data as 32bit ARGB
	 * @param[in] width Image width in pixels
	 * @param[in] height Image height in pixels
	 * @throw Ghulbus::gbException GB_FAILED indicates a file access error
	 */
	void WriteImage(char const* fname, GhulbusGraphics::GBCOLOR const* data, int width, int height);

	/** Get a loading strategy for BMP files; use in gbImageLoader
	 * @remark This approach uses static objects and is therefore *not* thread-safe
	 */
	gbImageLoader::gbImageType* gbImageType_BMP();
	class gbImageType_BMP_T: public gbImageLoader::gbImageType {
	private:
		unsigned int m_file_offset;			///< offset of file pointer (if multiple images are stored in the same file)
	public:
		void ReadFile(std::ifstream& file, int* width, int* height, int* bpp, 
			unsigned char** pp_data, unsigned int** pp_palette);
		bool CheckFile(std::ifstream const& file);
		gbImageType_BMP_T();
		virtual ~gbImageType_BMP_T();
	};

	/** Get a loading strategy for TGA files; use in gbImageLoader
	 * @remark This approach uses static objects and is therefore *not* thread-safe
	 */
	gbImageLoader::gbImageType* gbImageType_TGA();
	class gbImageType_TGA_T: public gbImageLoader::gbImageType {
	private:
		unsigned int m_file_offset;			///< offset of file pointer (if multiple images are stored in the same file)
	public:
		void ReadFile(std::ifstream& file, int* width, int* height, int* bpp, 
			unsigned char** pp_data, unsigned int** pp_palette);
		bool CheckFile(std::ifstream const& file);
		gbImageType_TGA_T();
		virtual ~gbImageType_TGA_T();
	};
};

#endif
