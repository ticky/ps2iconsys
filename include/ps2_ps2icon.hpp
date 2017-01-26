/**
 * @file include/ps2_ps2icon.hpp
 *
 * @brief A loader for PS2 icon files
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#ifndef __PS2_ICON_LOADER_HPP_INCLUDE_GUARD__
#define __PS2_ICON_LOADER_HPP_INCLUDE_GUARD__

#include <fstream>
#include "../gbLib/include/gbException.hpp"
#include "obj_loader.hpp"

/** A loader for PS2 icon files
 * @note Currently all structs defined in this class are assumed to be unpadded!
 *       This doesn't prove to be a problem yet, but keep it in mind anyhow!
 */
class PS2Icon {
public:
	/** File header
	 */
	typedef struct Icon_Header_t {
		unsigned int file_id;						///< reserved; should be: 0x010000 (but does not have to ;) )
		unsigned int animation_shapes;				///< number of animation shapes per vertex
		unsigned int texture_type;					///< texture type - 0x07: uncompressed, 0x06: uncompresses, 0x0f: RLE compression
		unsigned int reserved;						///< reserved; should be: 0x3F800000 (but does not have to ;) )
		unsigned int n_vertices;					///< number of vertices; must be a multiple of 3
	} Icon_Header;
	/** Set of vertex coordinates
	 * @note The f16_* fields indicate float16 data; divide by 4096.0f to convert to float32;
	 */
	typedef struct Vertex_Coord_t {
		short f16_x;								///< vertex x coordinate in float16
		short f16_y;								///< vertex y coordinate in float16
		short f16_z;								///< vertex z coordinate in float16
		short f16_unknown;							///< unknown; seems to influence lightning?
	} Vertex_Coord;
	/** Set of texture coordinates
	 * @note The f16_* fields indicate float16 data; divide by 4096.0f to convert to float32;
	 */
	typedef struct Texture_Data_t {
		short        f16_u;							///< vertex u texture coordinate in float16
		short        f16_v;							///< vertex v texture coordinate in float16
		unsigned int color;							///< vertex color (32 bit RGBA)
	} Texture_Data;
	/** Animation header
	 */
	typedef struct Animation_Header_t {
		unsigned int id_tag;						///< ???
		unsigned int frame_length;					///< ???
		float        anim_speed;					///< ???
		unsigned int play_offset;					///< ???
		unsigned int n_frames;						///< number of frames in the animation
	} Animation_Header;
	/** Per-frame animation data
	 */
	typedef struct Frame_Data_t {
		unsigned int shape_id;						///< shape used for this frame
		unsigned int n_keys;						///< number of keys corresponding to this frame
	} Frame_Data;
	/** Per-key animation data
	 */
	typedef struct Frame_Key_t {
		float time;									///< ???
		float value;								///< ???
	} Frame_Key;
private:
	Icon_Header header;								///< icon file header
	Vertex_Coord* vertices;							///< icon vertex data
	Vertex_Coord* normals;							///< icon normal data
	Texture_Data* vert_texture;						///< per-vertex texture data
	float* fvertices;								///< converted vertex data
	float* fnormals;								///< converted normal data
	Animation_Header anim_header;					///< animation segment header
	Frame_Data* animation;							///< animation data
	Frame_Key** anim_keys;							///< frame key data
	unsigned int texture[16384];					///< texture image data (128*128 pixels) in ARGB format (8 bits per channel)
public:
	/** Constructor
	 * @note This just fills the fields with default values. 
	 *       To obtain a valid file you must at least call SetGeometry() before writing.
	 */
	PS2Icon();
	/** Constructor
	 * @param[in] fname Complete path to a valid icon file
	 * @throw Ghulbus::gbException GB_FAILED indicates a file access error; 
	 * @throw std::bad_alloc
	 */
	PS2Icon(char const * fname);
	/** Destructor
	 */
	~PS2Icon();
	/** Get the number of vertices of the icon
	 * @return The number of vertices of the icon
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 */
	int GetNVertices() const;
	/** Get the number of shapes per vertex
	 * @return The number of shapes of the icon
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 */
	int GetNShapes() const;
	/** Get the number of frames in the animation
	 * @return The number of frames of the icon
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 */
	int GetNFrames() const;
	/** Get the number of the shape used in a specific frame
	 * @param[in] frame Number of the frame [0..(n_frames-1)]
	 * @return The number of the shape used in frame [0..(animation_shapes-1)]
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 *                             GB_ILLEGALPARAMETER;
	 */
	int GetFrameShape(int frame) const;
	/** Get the number of keys used in a specific frame
	 * @param[in] frame Number of the frame [0..(n_frames-1)]
	 * @return Number of keys used in frame
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 *                             GB_ILLEGALPARAMETER;
	 */
	int GetNFrameKeys(int frame) const;
	/** Get the time key for a specific frame and key
	 * @param[in] frame Number of the frame [0..(n_frames-1)]
	 * @param[in] key Number of the key [0..(animation[frame].n_keys-1)]
	 * @return The time key used in frame, key
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER;
	 */
	float GetFrameKeyTime(int frame, int key) const;
	/** Get the value key for a specific frame and key
	 * @param[in] frame Number of the frame [0..(n_frames-1)]
	 * @param[in] key Number of the key [0..(animation[frame].n_keys-1)]
	 * @return The value key used in frame, key
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER;
	 */
	float GetFrameKeyValue(int frame, int key) const;
	/** Get the texture type
	 * @return The texture type id (>=0x07: uncompressed; <=0x07: rle encoded)
	 * @throw Ghulbus::gbException GB_FAILED uint overflow;
	 */
	int GetTextureType() const;
	/** Get the vertex data used in a specific shape
	 * @param[out] data A field of at least size (n_vertices * 3), resp. (n_vertices * n_shapes * 3)
	 * @param[in] shape A positive integer specifies the shape; A negative integer causes all shapes to be copied to data sequentially
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER;
	 */
	void GetVertexData(float* data, int shape) const;
	/** Get the per-vertex color data
	 * @param[out] data A field of at least size (n_vertices)
	 */
	void GetVertexColorData(unsigned int* data) const;
	/** Get the per-vertex normal data
	 * @param[out] data A field of at least size (n_vertices * 3)
	 */
	void GetNormalData(float* data) const;
	/** Get the per-vertex texture coordinates
	 * @param[out] data A field of at least size (n_vertices * 2)
	 */
	void GetVertexTextureData(float* data) const;
	/** Get the complete texture data block
	 * @param[out] data A field of at least size 16384
	 */
	void GetTextureData(unsigned int* data) const;
	/** Copy the complete texture block to a pitched field
	 * @param[out] data A field of at least size (16384 * sizeof(unsigned int))
	 * @param[in] pitch The size of each row in data in bytes
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER;
	 */
	void GetTextureData(unsigned int* data, int pitch) const;
	/** Get the color value of a specific pixel in the texture
	 * @param[in] x X-coordinate [0..127]
	 * @param[in] y Y-coordinate [0..127]
	 * @return The texel value (D3DCOLOR format)
	 * @throw Ghulbus::gbException GB_ILLEGALPARAMETER;
	 */
	unsigned int GetTextureData(int x, int y) const;
	/** Save the current data to a new icon file
	 * @param[in] fname The full path of the destination file
	 * @throw Ghulbus::gbException GB_FAILED file access error;
	 */
	void WriteFile(char const * fname) const;
	/** Set the geometry data of the icon
	 * @param[in] mesh A valid OBJ_Mesh object holding new geometry data
	 * @throw std::bad_alloc
	 */
	void SetGeometry(OBJ_Mesh const& mesh);
	/** Set the geometry data of the icon
	 * @param[in] mesh A valid OBJ_Mesh object holding new geometry data
	 * @param[in] scale_factor A factor that is multiplied onto each vertex for scaling
	 * @throw std::bad_alloc
	 */
	void SetGeometry(OBJ_Mesh const& mesh, float scale_factor);
	/** Set the geometry data of the icon
	 * @param[in] pverts A field of at least size n_vertices*3 holding vertex data
	 * @param[in] pnormals A field of at least size n_vertices*3 holding normal data
	 * @param[in] ptexture A field of at least size n_vertices*2 holding 2D texture coordinate data
	 * @param[in] n_vertices Number of vertices
	 */
	void SetGeometry(float const* pverts, float const* pnormals, float const* ptexture, int n_vertices);
	/** Set the texture data of the icon
	 * @param[in] data A field of at least size 16384 containing 32 bit image data
	 */
	void SetTextureData(unsigned int const* data);
	/** Build a mesh from current data
	 * @param[in,out] mesh A mesh object that will be filled with the icon geometry
	 */
	void BuildMesh(OBJ_Mesh* mesh);
private:
	/** Internal helper function: resets and allocates memory for geometry
	 * @throw std::bad_alloc
	 */
	void AllocateVertexMemory();
	/** Internal helper function: reads icon data from a (binary) filestream
	 * @throw Ghulbus::gbException GB_FAILED indicates either file access error or uint overflow;
	 * @throw std::bad_alloc
	 */
	void ReadFile(std::ifstream & fin);
	/** Internal helper function: checks the validity of a file header
	 */
	static bool CheckValidity(Icon_Header const&);
};
typedef PS2Icon *LPPS2ICON;

//EXTENSIVE DOCUMENTATION:
/**
 * @class PS2Icon
 * This class can be used to read and write icon files.
 * @note This class keeps two representations of most of its data. Aside from the
 *       representation also used in the file, there is also a representation 
 *       that fits the standards of modern graphic APIs for visualization.
 * @todo SetAnimation
 *
 * @section ps2icon_file The file format
 * The file is made up of the following segments:
 * @li @ref ps2icon_file_header "A file header"
 * @li @ref ps2icon_file_vertexdata "A vertex data segment"
 * @li @ref ps2icon_file_animation_header "An animation header"
 * @li @ref ps2icon_file_animationdata "An animation data segment"
 * @li @ref ps2icon_file_texturedata "A texture data segment"
 *
 * @subsection ps2icon_file_header The file header
 * The file header is defined by the @ref Icon_Header_t struct and is 
 * pretty straightforward. It stores the number of vertices present as
 * well as the number of animation shapes which are both required to
 * calculate the offset to the animation header segment.
 * The file header is always 20 bytes in size.
 *
 * @subsection ps2icon_file_vertexdata The vertex data segment
 * PS2 Icons store all vertex data linearly and unindexed. The vertex
 * segment consists of (n_vertices) different entries of the following
 * structure:
 * @li First, (animation_shapes) entries of type @ref Vertex_Coord_t 
 *     specifying the vertex's position in space 
 * @li Followed by one entry of type @ref Vertex_Coord_t specifying 
 *     the normal for that vertex
 * @li Followed by one entry of type @ref Texture_Data_t specifying 
 *     texture coordinates and color for that vertex
 * Notice that, while a single vertex usually has a different position in
 * each shape, it always has the same normal and texture data!
 * 
 * @subsection ps2icon_file_animation_header The animation header
 * The animation header is defined by @ref Animation_Header_t. Little is
 * known about those values, except that there is one entry specifying
 * the number of frames present in the animation.
 * @note Notice that the number of frames is not the same as the number
 *       of shapes! A shape can be used in several frames in order to
 *       to compose complex animation with little memory overhead.
 *
 * @subsection ps2icon_file_animationdata The animation data segment
 * The animation data segment holds n_frames entries, each consisting of:
 * @li First, a @ref Frame_Data_t struct, specifying the used shape and the
 *     number of keys that follow.
 * @li Then (n_keys) numbers of @ref Frame_Key_t specifying additional
 *     information about the frame; The exact semantics of the key are unknown.
 * @note The documentation by Martin Akesson claims that the @ref Frame_Data_t
 *       is actually 16 bytes large, which is probably a typo.
 *
 * @subsection ps2icon_file_texturedata The texture data segment
 * Each icon has a 128*128 pixel texture attached, with 16 bits reserved for
 * each pixel. The color format is presumably @c ABBBBBGGGGGRRRRR where the
 * exact purpose of the alpha bit is yet to be discovered.
 * If the texture_type value in the header has a value of 0x07 or higher the
 * texture data is encoded with a simple RLE algorithm, which was documented
 * in detail by Martin Akesson.
 *
 * @remarks Currently the framework assumes that none of the 32 bit unsigned
 *          integer values present in a file exceeds INT_MAX. If at some point
 *          a value is encountered, that is greater than INT_MAX and can thus
 *          not be safely cast to an int, an exception will be thrown. However,
 *          this should never occur in a real world example.
 */
#endif
