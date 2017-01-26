/**
 * @file include/ps2_iconsys.hpp
 *
 * @brief A loader for Playstation2 icon.sys files
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#ifndef __PS2_ICON_SYS_HPP_INCLUDE_GUARD__
#define __PS2_ICON_SYS_HPP_INCLUDE_GUARD__

#include <fstream>
#include "../gbLib/include/gbException.hpp"

/** A loader for the PS2 icon.sys files
 */
class IconSys {
public:
	/** This class stores integer color value data
	 * @note Note that icon.sys int color values must be in a range 0x00 - 0x80; 
	 * The Set*() functions and constructors take care of proper truncations
	 */
	class IconSys_Color {		
	private:
		int R;
		int G;
		int B;
		int X;
	public:
		IconSys_Color(int const *);
		IconSys_Color(unsigned int const *);
		IconSys_Color(int r, int g, int b, int x);
		IconSys_Color(IconSys_Color const&);
		~IconSys_Color();
		IconSys_Color& operator =(IconSys_Color const&);
		int GetR() const;
		int GetG() const;
		int GetB() const;
		int GetX() const;
		int GetR8() const;
		int GetG8() const;
		int GetB8() const;
		int GetX8() const;
		bool SetR(int r);
		bool SetG(int g);
		bool SetB(int b);
		bool SetX(int x);
		void Get(unsigned int* p) const;
	private:
		void CheckRanges();
	};
	/** This class stores float color value data
	 * @note Note that icon.sys float color values must be in a range 0.0f - 1.0f; 
	 * The Set*() functions and constructors take care of proper truncations
	 */
	class IconSys_LightColor {
	private:
		float R; 
		float G;
		float B;
		float X;
	public:
		IconSys_LightColor(float const*);
		IconSys_LightColor(float r, float g, float b, float x);
		IconSys_LightColor(IconSys_LightColor const& rhs);
		~IconSys_LightColor();
		IconSys_LightColor& operator=(IconSys_LightColor const& rhs);
		float GetR() const;
		float GetG() const;
		float GetB() const;
		float GetX() const;
		int GetR8() const;
		int GetG8() const;
		int GetB8() const;
		int GetX8() const;
		bool SetR(float r);
		bool SetG(float g);
		bool SetB(float b);
		bool SetX(float x);
		void Get(float* p) const;
	private:
		void CheckRanges();
	};
	/** This class stores the light direction vectors
	 */
	class IconSys_LightVec {
	private:
		float X;
		float Y;
		float Z;
		float W;
	public:
		IconSys_LightVec(float const*);
		IconSys_LightVec(float fx, float fy, float fz, float fw);
		IconSys_LightVec(IconSys_LightVec const &);
		~IconSys_LightVec();
		IconSys_LightVec& operator=(IconSys_LightVec const&);
		float GetX() const;
		float GetY() const;
		float GetZ() const;
		float GetW() const;
		bool SetX(float x);
		bool SetY(float y);
		bool SetZ(float z);
		bool SetW(float w);
		void Get(float* p) const;
	};
private:
	/** The main struct resembling a complete icon.sys file
	 * @note Currently the struct is assumed to be completely unpadded! 
	 *       Due to the structure of the struct this shouldn't be a 
	 *       problem yet, but keep it in mind!
	 */
	struct File_t {
		/* some notes about the file format:
		 *  - the file should always be exactly 964 bytes in size
		 *  - colors are specified in 128 bit rgb with range 0x00-0x80 for each channel
		 *  - light direction?
		 *  - S-JIS strings?
		 *  - filename convention?
		 */
		char ps2d_string[4];						///< reserved, should be: PS2D
		unsigned short reserve1;					///< reserved, should be: 0
		unsigned short offset_2nd_line;				///< position of the linebreak in the SJIS title string
		unsigned int reserve2;						///< reserved, should be: 0
		unsigned int bg_opacity;					///< background opacity
		unsigned int bg_color_upperleft[4];			///< background color upper left
		unsigned int bg_color_upperright[4];		///< background color upper right
		unsigned int bg_color_lowerleft[4];			///< background color lower left
		unsigned int bg_color_lowerright[4];		///< background color lower right
		float light1_direction[4];					///< light 1 direction
		float light2_direction[4];					///< light 2 direction
		float light3_direction[4];					///< light 3 direction
		float light1_color[4];						///< light 1 color (0..1)
		float light2_color[4];						///< light 2 color (0..1)
		float light3_color[4];						///< light 3 color (0..1)
		float light_ambient_color[4];				///< ambient light color (0..1)
		unsigned char title[68];					///< title string (null terminated S-JIS)
		unsigned char icon_file[64];				///< filename of normal icon (null terminated ASCII)
		unsigned char icon_copy_file[64];			///< filename of copy operation icon (null terminated ASCII)
		unsigned char icon_delete_file[64];			///< filename of delete opertation icon (null terminated ASCII)
		unsigned char reserve3[512];				///< reserved, should be: 0
	} File;
	char decoded_title[34];							///< title string in ASCII
	char title_str[35];								///< decoded_title with proper linebreak
	char title_str_single_line[35];					///< decoded_title with whitespace instead of linebreak
public:
	/** Constructor
	 */
	IconSys();
	/** Constructor
	 * @param[in] fname Complete path to a valid icon.sys file
	 * @throw Ghulbus::gbException GB_FAILED indicates either a file read error or corrupt file;
	 */
	IconSys(const char* fname);
	/** Destructor
	 */
	~IconSys();
	/** Fills the File structure with default values
	 */
	void SetToDefault();
	/** Get the title string as displayed by the PS2s file manager
	 * @return The file's title as a null terminated C-String
	 */
	char const * GetTitle() const;
	/** Set the title string (both S-JIS and ASCII)
	 * @param[in] str A null-terminated C-string (at most 32 characters!)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetTitle(char const* str);
	/** Set the title string linebreak
	 * @param[in] lb Position of the linebreak in the ASCII string (0..32)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetLinebreak(int lb);
	/** Get the title string without enclosed linebreaks
	 * @return The file's title as a null terminated C-string without enclosed linebreaks
	 */
	char const * GetTitleSingleLine() const;
	/** Get the filename of the standard icon referenced by the file
	 * @return The standard icon's path as a null terminated C-string
	 */
	char const * GetIconFilename() const;
	/** Set the standard icon filename
	 * @param[in] fname A null-terminated C-string (length at most 32)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetIconFilename(char const* fname);
	/** Get the filename of the copy icon referenced by the file
	 * @return The copy icon's path as a null terminated C-string
	 */
	char const * GetIconCopyFilename() const;
	/** Set the copy icon filename
	 * @param[in] fname A null-terminated C-string (length at most 32)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetIconCopyFilename(char const* fname);
	/** Get the filename of the delete icon referenced by the file
	 * @return The delete icon's path as a null terminated C-string
	 */
	char const * GetIconDeleteFilename() const;
	/** Set the delete icon filename
	 * @param[in] fname A null-terminated C-string (length at most 32)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetIconDeleteFilename(char const* fname);
	/** Get the background opacity value
	 * @return The specified background opacity
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 */
	int GetBackgroundOpacity() const;
	/** Set the background opacity
	 * @param[in] i The new background opacity [0..255]
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER
	 */
	void SetBackgroundOpacity(int i);
	/** Get the color of the upper left corner
	 * @return The color of the upper left corner
	 */
	IconSys_Color GetBackgroundColor_UL() const;
	/** Set the color of the upper left corner
	 */
	void SetBackgroundColor_UL(IconSys_Color const& c);
	/** Get the color of the upper right corner
	 * @return The color of the upper right corner
	 */
	IconSys_Color GetBackgroundColor_UR() const;
	/** Set the color of the upper right corner
	 */
	void SetBackgroundColor_UR(IconSys_Color const& c);
	/** Get the color of the lower left corner
	 * @return The color of the lower left corner
	 */
	IconSys_Color GetBackgroundColor_LL() const;
	/** Set the color of the lower left corner
	 */
	void SetBackgroundColor_LL(IconSys_Color const& c);
	/** Get the color of the lower right corner
	 * @return The color of the lower right corner
	 */
	IconSys_Color GetBackgroundColor_LR() const;
	/** Set the color of the lower right corner
	 */
	void SetBackgroundColor_LR(IconSys_Color const& c);
	/** Get the color of light source #1
	 * @return Light source's color
	 */
	IconSys_LightColor GetLight1Color() const;
	/** Set the color of light source #1
	 */
	void SetLight1Color(IconSys_LightColor const& c);
	/** Get the color of light source #2
	 * @return Light source's color
	 */
	IconSys_LightColor GetLight2Color() const;
	/** Set the color of light source #2
	 */
	void SetLight2Color(IconSys_LightColor const& c);
	/** Get the color of light source #3
	 * @return Light source's color
	 */
	IconSys_LightColor GetLight3Color() const;
	/** Set the color of light source #3
	 */
	void SetLight3Color(IconSys_LightColor const& c);
	/** Get the color of the ambient light source
	 * @return Light source's color
	 */
	IconSys_LightColor GetLightAmbientColor() const;
	/** Set the color of the ambient light source
	 */
	void SetLightAmbientColor(IconSys_LightColor const& c);
	/** Get the direction of light source #1
	 * @return The light source's direction vector
	 */
	IconSys_LightVec GetLight1Dir() const;
	/** Set the direction of light source #1
	 */
	void SetLight1Dir(IconSys_LightVec const& v);
	/** Get the direction of light source #2
	 * @return The light source's direction vector
	 */
	IconSys_LightVec GetLight2Dir() const;
	/** Set the direction of light source #2
	 */
	void SetLight2Dir(IconSys_LightVec const& v);
	/** Get the direction of light source #3
	 * @return The light source's direction vector
	 */
	IconSys_LightVec GetLight3Dir() const;
	/** Set the direction of light source #3
	 */
	void SetLight3Dir(IconSys_LightVec const& v);
	/** Write the current data to a file
	 * @param[in] fname Name of the file to save to
	 * @throw Ghulbus::gbException GB_FAILED indicates a file access error;
	 */
	void WriteFile(char const * fname);
private:
	/** Internal helper function
	 * Checks the PS2D string and the reserved fields for consistency
	 */
	static bool CheckValidity(File_t const&);
	/** Decodes S-JIS to ASCII
	 * @note Not all S-JIS characters are supported!
	 */
	static void DecodeTitle(unsigned char const * str_in, char* str_out);
	/** Encode S-JIS from ASCII
	 * @note Not all S-JIS characters are supported!
	 */
	static void EncodeTitle(char const* str_in, unsigned char* str_out);
	/** Augments the ASCII title string with a proper line break
	 */
	static void GetTitleString(char const * str_in, unsigned int pos_linebreak, char * str_out);
	IconSys(IconSys const&);						///< private copy constructor (not implemented!)
	IconSys& operator=(IconSys const&);				///< private copy assignment (not implemented!)
};
typedef IconSys *LPICONSYS;

//EXTENSIVE DOCUMENTATION:
/**
 * @class IconSys
 * This class can be used to read and write icon.sys files. The heart of the class is 
 * the File_t struct which resembles a complete icon.sys file on a binary basis.
 * Due to their static structure, icon.sys files can be load by directly reading to a
 * a File_t and written by directly writing from a File_t, which makes the implementation
 * pretty straightforward.
 *
 * The implementation of the S-JIS/ASCII converter is not very good and far from 
 * complete. While it supports all of alphanumeric characters and a few special 
 * chars it will certainly fail when encountering some more exotic chars. Feel free
 * to augment/replace the DecodeTitle() function at your needs.
 *
 */
#endif
