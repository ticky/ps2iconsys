/**
 * @file src/ps2_ps2icon.cpp
 *
 * @brief Implementation of the PS2Icon class
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include "../include/ps2_ps2icon.hpp"

/** Helper function: converts float32 to float16
 */
inline short convert_f32_to_f16(float const& f) {
	return( static_cast<short>(f * 4096.0f) );
}

/** Helper function: converts float16 to float32
 */
inline float convert_f16_to_f32(short const& i) {
	return( static_cast<float>(i) / 4096.0f );	
}

bool PS2Icon::CheckValidity(PS2Icon::Icon_Header const& p) {
	if( (p.file_id != 0x010000) ||
		(p.reserved != 0x3F800000) )
	{ 
		// it seems not all icons set these ids right;
		// since the ps2 doesn't seem to care too much, there's no reason why we should
		//return false; 
	}
	//n_vertices has to be divisible by three, that's for sure:
	if(p.n_vertices % 3 != 0) {
		return false;
	}
	//seems that all we can check for now;
	//the regulations on the header files are somewhat
	//sloppy, so we'll just have to hope the best...
	return true;
}

PS2Icon::PS2Icon(const char *fname): vertices(NULL), normals(NULL), vert_texture(NULL),
fvertices(NULL), fnormals(NULL), animation(NULL), anim_keys(NULL)
{
	std::ifstream fin(fname, std::ios_base::in | std::ios_base::binary);
	if(fin.fail()) { throw( Ghulbus::gbException(Ghulbus::gbException::GB_FAILED,
	                                             "Could not open icon file for read") ); }
	//read icon data from file:
	try {
		ReadFile(fin);
	} catch(std::exception) {
		fin.close();
		throw;
	}
	fin.close();
}

PS2Icon::PS2Icon(): vertices(NULL), normals(NULL), vert_texture(NULL), 
fvertices(NULL), fnormals(NULL), animation(NULL), anim_keys(NULL)
{
	header.file_id          = 0x010000;
	header.animation_shapes = 1;
	header.texture_type     = 0x07;
	header.reserved         = 0x3F800000;
	header.n_vertices       = 0;
	
	anim_header.id_tag       = 1;
	anim_header.frame_length = 31;
	anim_header.anim_speed   = 1.0f;
	anim_header.play_offset  = 0;
	anim_header.n_frames     = 0;
	
	memset(texture, 0, sizeof(unsigned int)*16384);
}

void PS2Icon::ReadFile(std::ifstream & fin)
{
	//read header:
	fin.read( reinterpret_cast<char*>(&header), sizeof(header) );

	if(!CheckValidity(header)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED, "Icon Header seems to be corrupted" ) );
	}

	//allocate memory for vertex data:
	AllocateVertexMemory();

	//read icon data from file:
	///Vertex data
	// each vertex consists of animation_shapes tuples for vertex coordinates,
	// followed by one vertex coordinate tuple for normal coordinates
	// followed by one texture data tuple for texture coordinates and color
	for(unsigned int i=0; i<header.n_vertices; i++) {
		fin.read( reinterpret_cast<char*>(&vertices[i*header.animation_shapes]), 
			      sizeof(Vertex_Coord) * header.animation_shapes );
		fin.read( reinterpret_cast<char*>(&normals[i]), sizeof(Vertex_Coord) );
		fin.read( reinterpret_cast<char*>(&vert_texture[i]), sizeof(Texture_Data) );
		
		for(unsigned int j=0; j<header.animation_shapes; j++) {
			fvertices[(i*header.animation_shapes + j) * 3] = 
				convert_f16_to_f32(vertices[i*header.animation_shapes + j].f16_x);
			fvertices[(i*header.animation_shapes + j) * 3 + 1] = 
				convert_f16_to_f32(vertices[i*header.animation_shapes + j].f16_y);
			fvertices[(i*header.animation_shapes + j) * 3 + 2] = 
				convert_f16_to_f32(vertices[i*header.animation_shapes + j].f16_z);
		}
		fnormals[i*3]     = convert_f16_to_f32(normals[i].f16_x);
		fnormals[i*3 + 1] = convert_f16_to_f32(normals[i].f16_y);
		fnormals[i*3 + 2] = convert_f16_to_f32(normals[i].f16_z);
	}

	//animation data
	// preceeded by an animation header, there is a frame data/key set for every frame:
	fin.read( reinterpret_cast<char*>(&anim_header), sizeof(Animation_Header) );

	//allocate memory for animation data:
	animation = new Frame_Data[anim_header.n_frames];
	anim_keys = new Frame_Key*[anim_header.n_frames];
	//read animation data:
	for(unsigned int i=0; i<anim_header.n_frames; i++) {
		fin.read( reinterpret_cast<char*>(&animation[i]), sizeof(Frame_Data) );
		if(animation[i].n_keys > 0) {
			anim_keys[i] = new Frame_Key[animation[i].n_keys];
			fin.read( reinterpret_cast<char*>(anim_keys[i]), sizeof(Frame_Key)*animation[i].n_keys);
		} else {
			anim_keys[i] = NULL;
		}
	}

	//read texture data:
	if(header.texture_type <= 0x07) {	//uncompressed textures
		unsigned short c;
		for(int i=0; i<16384; i++) {
			fin.read( reinterpret_cast<char*>(&c), 2 );
			unsigned char r = static_cast<unsigned char>( ( c        & 0x1f) << 3 );
			unsigned char g = static_cast<unsigned char>( ((c >> 5)  & 0x1f) << 3 );
			unsigned char b = static_cast<unsigned char>( ((c >> 10)       ) << 3 );
			texture[i] =  static_cast<unsigned int>( (    (0xff)<<24) |
				                                     (((r)&0xff)<<16) | 
										             (((g)&0xff)<<8)  |
										             ( (b)&0xff)  );
		}
	} else {							//compressed textures
		//simple rle encoding:
		// first 32 bits hold size of texture data
		unsigned int data_size;
		fin.read( reinterpret_cast<char*>(&data_size), 4 );
		unsigned int limit = fin.tellg();
		limit += data_size;
		if(limit > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED, 
			                                               "File size is bigger than INT_MAX" ) ); }
		unsigned int index = 0;
		while( fin.tellg() < static_cast<int>(limit) ) {
			//next 16 bits indicate the type of data to follow:
			unsigned short rep_count;
			fin.read( reinterpret_cast<char*>(&rep_count), 2 );
			if(rep_count < 0xFF00) {			//repeat the pixel rep_count times
				unsigned short c;
				fin.read( reinterpret_cast<char*>(&c), 2 );
				unsigned char r = static_cast<unsigned char>( ( c        & 0x1f) << 3 );
				unsigned char g = static_cast<unsigned char>( ((c >> 5)  & 0x1f) << 3 );
				unsigned char b = static_cast<unsigned char>( ((c >> 10)       ) << 3 );
				for(int i=0; i<rep_count; i++) {
					texture[index++] = static_cast<unsigned int> (     ((0xff)<<24) |
				                                                   (((r)&0xff)<<16) | 
										                           (((g)&0xff)<<8)  |
										                           ( (b)&0xff)  );
				}
			} else {							//copy the next rep_count pixels directly
				for(unsigned int i=0; i<=(0xFFFF ^ static_cast<unsigned int>(rep_count)); i++) {
					unsigned short c;
					fin.read( reinterpret_cast<char*>(&c), 2 );
					unsigned char r = static_cast<unsigned char>( ( c        & 0x1f) << 3 );
					unsigned char g = static_cast<unsigned char>( ((c >> 5)  & 0x1f) << 3 );
					unsigned char b = static_cast<unsigned char>( ((c >> 10)       ) << 3 );
					texture[index++] = static_cast<unsigned int>(     ((0xff)<<24) |
				                                                  (((r)&0xff)<<16) | 
										                          (((g)&0xff)<<8)  |
										                          ( (b)&0xff)  );
				}
			}
		}
	}

	if(fin.fail()) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
	                                              "File read error" ) ); }
}

void PS2Icon::AllocateVertexMemory() 
{
	//helper function that reallocates all per-vertex storage fields based 
	//on the information stored in the header struct
	if(vertices)     { delete[] vertices;          vertices = NULL; }
	if(fvertices)    { delete[] fvertices;        fvertices = NULL; }
	if(normals)      { delete[] normals;            normals = NULL; }
	if(fnormals)     { delete[] fnormals;          fnormals = NULL; }
	if(vert_texture) { delete[] vert_texture;  vert_texture = NULL; }
	vertices     = new Vertex_Coord[header.n_vertices * header.animation_shapes];
	fvertices    = new        float[header.n_vertices * header.animation_shapes * 3];
	normals      = new Vertex_Coord[header.n_vertices];
	fnormals     = new        float[header.n_vertices * 3];
	vert_texture = new Texture_Data[header.n_vertices];
}

void PS2Icon::SetGeometry(OBJ_Mesh const& mesh)
{
	SetGeometry(mesh, 1.0f);
}

void PS2Icon::SetGeometry(OBJ_Mesh const& mesh, float scale_factor)
{
	//rewrite header:
	header.file_id = 0x010000;	header.reserved = 0x3F800000;
	header.animation_shapes = 1;
	header.n_vertices = mesh.GetNFaces() * 3;

	//copy animation data:
	AllocateVertexMemory();
	float* tmptexture = new float[mesh.GetNFaces() * 9];

	mesh.GetMeshGeometryUnindexed(fvertices, fnormals, tmptexture, scale_factor);
	for(int i=0; i<mesh.GetNFaces() * 3; i++) {
		vertices[i].f16_x = convert_f32_to_f16(fvertices[i*3]);
		vertices[i].f16_y = convert_f32_to_f16(fvertices[i*3 + 1]);
		vertices[i].f16_z = convert_f32_to_f16(fvertices[i*3 + 2]);
		vertices[i].f16_unknown = 0;

		normals[i].f16_x = convert_f32_to_f16(fnormals[i*3]);
		normals[i].f16_y = convert_f32_to_f16(fnormals[i*3 + 1]);
		normals[i].f16_z = convert_f32_to_f16(fnormals[i*3 + 2]);
		normals[i].f16_unknown = 0;

		vert_texture[i].f16_u = convert_f32_to_f16(tmptexture[i*3]);
		vert_texture[i].f16_v = convert_f32_to_f16(tmptexture[i*3 + 1]);
		vert_texture[i].color = 0xFFFFFFFF;
	}
	delete[] tmptexture;

	//rewrite animation data:
	if(animation)    { delete[] animation;        animation = NULL; }
	if(anim_keys)    {
		for(unsigned int i=0; i<anim_header.n_frames; i++) {
			if(anim_keys[i]) {
				delete[] anim_keys[i];
				anim_keys[i] = NULL;
			}
		}
		delete[] anim_keys;
		anim_keys = NULL;
	}
	//insert default values for no animation:
	anim_header.n_frames = 1;
	animation = new Frame_Data[1];
	animation->n_keys   = 1;
	animation->shape_id = 0;
	anim_keys = new Frame_Key*[1];
	anim_keys[0] = new Frame_Key[1];
	anim_keys[0]->time  = 0.0f;
	anim_keys[0]->value = 1.0f;
}

void PS2Icon::SetGeometry(float const* pverts, float const* pnormals, float const* ptexture, int n_vertices) 
{
	//rewrite header:
	header.file_id = 0x010000;	header.reserved = 0x3F800000;
	header.animation_shapes = 1;
	header.n_vertices = n_vertices;

	//copy animation data:
	AllocateVertexMemory();
	memcpy(fvertices, pverts, sizeof(float) * 3 * n_vertices);
	memcpy(fnormals, pnormals, sizeof(float) * 3 * n_vertices);
	float* tmptexture = new float[2*n_vertices];
	memcpy(vert_texture, ptexture, sizeof(float) * 2 * n_vertices);

	for(int i=0; i<n_vertices; i++) {
		vertices[i].f16_x = convert_f32_to_f16(fvertices[i*3]);
		vertices[i].f16_y = convert_f32_to_f16(fvertices[i*3 + 1]);
		vertices[i].f16_z = convert_f32_to_f16(fvertices[i*3 + 2]);
		vertices[i].f16_unknown = 0;

		normals[i].f16_x = convert_f32_to_f16(fnormals[i*3]);
		normals[i].f16_y = convert_f32_to_f16(fnormals[i*3 + 1]);
		normals[i].f16_z = convert_f32_to_f16(fnormals[i*3 + 2]);
		normals[i].f16_unknown = 0;

		vert_texture[i].f16_u = convert_f32_to_f16(tmptexture[i*2]);
		vert_texture[i].f16_v = convert_f32_to_f16(tmptexture[i*2 + 1]);
		vert_texture[i].color = 0xFFFFFFFF;
	}
	delete[] tmptexture;

	//rewrite animation data:
	if(animation)    { delete[] animation;        animation = NULL; }
	if(anim_keys)    {
		for(unsigned int i=0; i<anim_header.n_frames; i++) {
			if(anim_keys[i]) {
				delete[] anim_keys[i];
				anim_keys[i] = NULL;
			}
		}
		delete[] anim_keys;
		anim_keys = NULL;
	}
	//insert default values for no animation:
	anim_header.n_frames = 1;
	animation = new Frame_Data[1];
	animation->n_keys   = 1;
	animation->shape_id = 0;
	anim_keys = new Frame_Key*[1];
	anim_keys[0] = new Frame_Key[1];
	anim_keys[0]->time  = 0.0f;
	anim_keys[0]->value = 1.0f;
}

void PS2Icon::SetTextureData(unsigned int const* data) {
	for(unsigned int i=0; i<16384; i++) {
		texture[i] = data[i];
	}
}

void PS2Icon::BuildMesh(OBJ_Mesh* mesh) {
	float* tf = new float[header.n_vertices*3];
	this->GetVertexData(tf, 0);
	mesh->SetGeometry(tf, header.n_vertices*3);
	delete[] tf;
	mesh->SetNormals(fnormals, header.n_vertices*3);
	
	mesh->ClearTextureData();
	float tmp;
	for(unsigned int i=0; i<header.n_vertices; i++) {
		//texture data has to be converted by hand (2D -> 3D)
		tmp = convert_f16_to_f32(vert_texture[i].f16_u);
		mesh->AddTextureData(&tmp, 1);
		tmp = convert_f16_to_f32(vert_texture[i].f16_v);
		mesh->AddTextureData(&tmp, 1);
		tmp = 0.0f;
		mesh->AddTextureData(&tmp, 1);
	}

	mesh->ClearFaceData();
	OBJ_Mesh::Face face;
	for(unsigned int i=0; i<header.n_vertices/3; i++) {
		//face indices are calculated straightforward
		///@todo: vertex recycling?
		face.vert1 = face.normal1 = face.texture1 = i*3;
		face.vert2 = face.normal2 = face.texture2 = i*3 + 1;
		face.vert3 = face.normal3 = face.texture3 = i*3 + 2;

		face.smoothing_group = 1;
		mesh->AddFaceData(&face, 1);
	}		
}

PS2Icon::~PS2Icon() 
{
	if(vertices)     { delete[] vertices;          vertices = NULL; }
	if(fvertices)    { delete[] fvertices;        fvertices = NULL; }
	if(normals)      { delete[] normals;            normals = NULL; }
	if(fnormals)     { delete[] fnormals;          fnormals = NULL; }
	if(vert_texture) { delete[] vert_texture;  vert_texture = NULL; }
	if(animation)    { delete[] animation;        animation = NULL; }
	if(anim_keys)    {
		for(unsigned int i=0; i<anim_header.n_frames; i++) {
			if(anim_keys[i]) {
				delete[] anim_keys[i];
				anim_keys[i] = NULL;
			}
		}
		delete[] anim_keys;
		anim_keys = NULL;
	}
}

int PS2Icon::GetNVertices() const {
	if(header.n_vertices > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return (int)header.n_vertices;
}
int PS2Icon::GetNShapes() const {
	if(header.animation_shapes > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return (int)header.animation_shapes;
}
int PS2Icon::GetNFrames() const {
	if(anim_header.n_frames > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return static_cast<int>(anim_header.n_frames);
}
int PS2Icon::GetFrameShape(int frame) const {
	if(static_cast<unsigned int>(frame) >= anim_header.n_frames) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) ); 
	}
	if(animation[frame].shape_id > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return static_cast<int>(animation[frame].shape_id);
}
int PS2Icon::GetNFrameKeys(int frame) const {
	if(static_cast<unsigned int>(frame) >= anim_header.n_frames) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) ); 
	}
	if(animation[frame].n_keys > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return static_cast<int>(animation[frame].n_keys);
}
float PS2Icon::GetFrameKeyTime(int frame, int key) const {
	if((static_cast<unsigned int>(frame) >= anim_header.n_frames) ||
	   (static_cast<unsigned int>(key) >= animation[frame].n_keys) ) {
		   throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );;
	}
	return anim_keys[frame][key].time;
}
float PS2Icon::GetFrameKeyValue(int frame, int key) const {
	if((static_cast<unsigned int>(frame) >= anim_header.n_frames) ||
	   (static_cast<unsigned int>(key) >= animation[frame].n_keys) ) {
		   throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return anim_keys[frame][key].value;
}
int PS2Icon::GetTextureType() const {
	if(header.texture_type > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return static_cast<int>(header.texture_type);
}
void PS2Icon::GetVertexData(float* data, int shape) const {
	if(shape >= static_cast<int>(header.animation_shapes)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) ); 
	}
	if(shape >= 0) {	//copy specified shape
		for(unsigned int i=0; i<header.n_vertices; i++) {
			data[i*3]     = fvertices[(i*header.animation_shapes + shape) * 3];
			data[i*3 + 1] = fvertices[(i*header.animation_shapes + shape) * 3 + 1];
			data[i*3 + 2] = fvertices[(i*header.animation_shapes + shape) * 3 + 2];
		}
	} else {			//copy *all* shapes one after the other
		for(unsigned int j=0; j<header.animation_shapes; j++) {
			for(unsigned int i=0; i<header.n_vertices; i++) {
				data[j*header.n_vertices*3 + i*3]     = fvertices[(i*header.animation_shapes + j) * 3];
				data[j*header.n_vertices*3 + i*3 + 1] = fvertices[(i*header.animation_shapes + j) * 3 + 1];
				data[j*header.n_vertices*3 + i*3 + 2] = fvertices[(i*header.animation_shapes + j) * 3 + 2];
			}
		}
	}
}
void PS2Icon::GetVertexColorData(unsigned int* data) const {
	for(unsigned int i=0; i<header.n_vertices; i++) {
		data[i] = vert_texture[i].color;
	}
}
void PS2Icon::GetNormalData(float* data) const {
	memcpy(data, fnormals, sizeof(float)*header.n_vertices*3);
}
void PS2Icon::GetVertexTextureData(float* data) const {
	for(unsigned int i=0; i<header.n_vertices; i++) {
		data[i*2]   = convert_f16_to_f32(vert_texture[i].f16_u);
		data[i*2+1] = convert_f16_to_f32(vert_texture[i].f16_v);
	}
}
void PS2Icon::GetTextureData(unsigned int* data) const {
	memcpy(data, texture, sizeof(unsigned int)*16384);
}
void PS2Icon::GetTextureData(unsigned int* data, int pitch) const {
	if((pitch < 512) || (pitch % 4 != 0)) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	for(int i=0; i<128; i++) {
		memcpy(&(data[i*(pitch>>2)]), &(texture[i*128]), sizeof(unsigned int)*128);
	}
}
unsigned int PS2Icon::GetTextureData(int x, int y) const {
	if((x < 0) || (x >= 128) || (y < 0) || (y >= 128)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return texture[y*128 + x];
}

void PS2Icon::WriteFile(const char * fname) const {
	std::ofstream fout(fname, std::ios_base::out | std::ios_base::binary);
	if(fout.fail()) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                         "Output icon file could not be opened") );
	}
	fout.seekp(std::ios::beg);

	//write header:
	fout.write( reinterpret_cast<char const*>(&header), sizeof(Icon_Header) );

	//write vertex segment:
	for(unsigned int i=0; i<header.n_vertices; i++) {
		//vertex coordinates, one for each shape:
		fout.write( reinterpret_cast<char*>(&vertices[i*header.animation_shapes]),
			        sizeof(Vertex_Coord) * header.animation_shapes );
		//normal coordinates:
		fout.write( reinterpret_cast<char*>(&normals[i]), sizeof(Vertex_Coord) );
		//texture coordinates:
		fout.write( reinterpret_cast<char*>(&vert_texture[i]), sizeof(Texture_Data) );
	}

	//write animation segment:
	//header:
	fout.write( reinterpret_cast<char const*>(&anim_header), sizeof(Animation_Header) );
	//data:
	for(unsigned int i=0; i<anim_header.n_frames; i++) {
		fout.write( reinterpret_cast<char*>(&animation[i]), sizeof(Frame_Data) );
		fout.write( reinterpret_cast<char*>(anim_keys[i]), sizeof(Frame_Key) * animation[i].n_keys );
	}

	//write texture segment:
	if(header.texture_type <= 0x07) {
		//uncompressed:
		unsigned short c;
		for(int i=0; i<16384; i++) {
			unsigned char r = static_cast<unsigned char>( (texture[i] >> 16) & 0xff );
			unsigned char g = static_cast<unsigned char>( (texture[i] >>  8) & 0xff );
			unsigned char b = static_cast<unsigned char>(  texture[i]        & 0xff );
			c = static_cast<unsigned short>( ((r >> 3) & 0x001f) | ((g << 2) & 0x03e0) | ((b << 7) & 0x7c00) );
			fout.write( reinterpret_cast<char*>(&c), 2 );
		}
	} else {
		//compressed textures:
		//allocate space for size info:
		int base = fout.tellp();
		unsigned int size = 0;
		fout.write( reinterpret_cast<char*>(&base), 4 );	//dummy data
		//rle main loop:
		int i=0;
		while(i<16384) {
			//rle step 1: count replication
			int rep_count = 1;
			while( (texture[i] == texture[i+rep_count]) ) { rep_count++; }
			if(rep_count > 1) {		//pixels are replicated rep_count times
				unsigned char r = static_cast<unsigned char>( (texture[i] >> 16) & 0xff );
				unsigned char g = static_cast<unsigned char>( (texture[i] >>  8) & 0xff );
				unsigned char b = static_cast<unsigned char>(  texture[i]        & 0xff );
				unsigned short c = ((unsigned short)(r >> 3) | (g << 2) | (b << 7) );
				fout.write( reinterpret_cast<char*>(&rep_count), 2 );
				fout.write( reinterpret_cast<char*>(&c), 2 );
				i += rep_count;
			} else {				//no replication
				unsigned short pix_count = 0;	//number of non equal subsequent pixels
				while((texture[i+pix_count] != texture[i+pix_count+1]) &&
				      (pix_count < 255) ) { pix_count++; }
				pix_count = 0xFFFF ^ (pix_count-1);
				fout.write( reinterpret_cast<char*>(&pix_count), 2 );
				pix_count ^= 0xFFFF;
				for(unsigned short j=0; j<=pix_count; j++) {		//insert pixels:
					unsigned char r = static_cast<unsigned char>( (texture[i] >> 16) & 0xff );
					unsigned char g = static_cast<unsigned char>( (texture[i] >>  8) & 0xff );
					unsigned char b = static_cast<unsigned char>(  texture[i]        & 0xff );
					unsigned short c = ((unsigned short)(r >> 3) | (g << 2) | (b << 7) );
					fout.write( reinterpret_cast<char*>(&c), 2 );
					i++;
				}
			}
		}
		//finalize: write size of the encoded segment to base:
		size = fout.tellp();
		size -= (base + 4);

		fout.seekp(base);
		fout.write( reinterpret_cast<char*>(&size), 4 );
	}

	fout.close();
	if(fout.fail()) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                         "Error while writing output icon file") );
	}
}

///@todo support for alpha bit (bit #16) in texture
