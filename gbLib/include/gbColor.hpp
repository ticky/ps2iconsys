/**
 * @file include/gbColor.hpp
 *
 * ghulbusGraphics - The Ghulbus Graphics Library
 *
 * @brief gbGraphics Definition of color format
 *
 * @version 1.1
 * @author Andreas Weis
 *
 */
#ifndef _GHULBUSGRAPHICS_COLOR_HPP_INCLUDE_GUARD_
#define _GHULBUSGRAPHICS_COLOR_HPP_INCLUDE_GUARD_

namespace GhulbusGraphics {
		/** 32 bit color type
	 * A 32 bit RGBA color value;
	 * The actual bit pattern is platform dependent:
	 * - Win32 Direct3D9: AAAA AAAA RRRR RRRR GGGG GGGG BBBB BBBB
	 * - SDL OpenGL:      AAAA AAAA BBBB BBBB GGGG GGGG RRRR RRRR
	 */
	typedef unsigned int GBCOLOR;
	/** 8 bit color component type
	 */
	typedef unsigned char GBCOLOR_COMPONENT;

	/** Contains Macros for generating 32 bit RGBA color values
	 */
	namespace GBCOLOR32 {
		inline GBCOLOR XRGB(GBCOLOR_COMPONENT r, GBCOLOR_COMPONENT g, GBCOLOR_COMPONENT b) {
			return ( 0xFF000000 | (r<<16) | (g<<8) | (b) );
		}
		inline GBCOLOR XRGB(int r, int g, int b) {
			return ( 0xFF000000 | ((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff) );
		}
		inline GBCOLOR ARGB(GBCOLOR_COMPONENT a, GBCOLOR_COMPONENT r, GBCOLOR_COMPONENT g, GBCOLOR_COMPONENT b) {
			return ( (a<<24) | (r<<16) | (g<<8) | (b) );
		}
		inline GBCOLOR ARGB(int a, int r, int g, int b) {
			return ( ((a&0xff)<<24) | ((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff) );
		}
		inline GBCOLOR_COMPONENT GetR(GBCOLOR c) {
			return static_cast<GBCOLOR_COMPONENT>( (c>>16) & 0xff );
		}
		inline GBCOLOR_COMPONENT GetG(GBCOLOR c) {
			return static_cast<GBCOLOR_COMPONENT>( (c>>8) & 0xff );
		}
		inline GBCOLOR_COMPONENT GetB(GBCOLOR c) {
			return static_cast<GBCOLOR_COMPONENT>( c & 0xff );
		}
		inline GBCOLOR_COMPONENT GetA(GBCOLOR c) {
			return static_cast<GBCOLOR_COMPONENT>( (c>>24) & 0xff );
		}	
	};
};

#endif
