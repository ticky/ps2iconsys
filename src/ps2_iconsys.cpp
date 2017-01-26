/**
 * @file src/ps2_iconsys.cpp
 *
 * @brief Implementation of the icon.sys file loader
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include "../include/ps2_iconsys.hpp"

bool IconSys::CheckValidity(IconSys::File_t const& f)
{
	if( (f.ps2d_string[0] != 'P') ||
		(f.ps2d_string[1] != 'S') ||
		(f.ps2d_string[2] != '2') ||
		(f.ps2d_string[3] != 'D') ||
		/* (f.reserve1 != 0) || */ //this is not necessary, see e.g. system configuration file
		(f.reserve2 != 0) ) { return false; }
	for(int i=0; i<512; i++) {
		if(f.reserve3[i] != 0) { return false; }
	}
	return true;
}

void IconSys::DecodeTitle(unsigned char const * str_in, char* str_out)
{
	unsigned char t1, t2;

	for(int i=0; i<68; i+=2) {
		t1 = str_in[i];		//each S-JIS character consists of two bytes
		t2 = str_in[i+1];

		switch(t1) {
			case 0x00:
				if(t2 == 0x00) {
					str_out[i>>1] = '\0';
					return;
				} else {
					str_out[i>>1] = '?';
				}
				break;
			case 0x81:
				if(t2 == 0x40) {
					str_out[i>>1] = ' ';
				} else if(t2 == 0x46) {
					str_out[i>>1] = ':';
				} else if(t2 == 0x5E) {
					str_out[i>>1] = '/';
				} else if(t2 == 0x69) {
					str_out[i>>1] = '(';
				} else if(t2 == 0x6A) {
					str_out[i>>1] = ')';
				} else if(t2 == 0x6D) {
					str_out[i>>1] = '[';
				} else if(t2 == 0x6E) {
					str_out[i>>1] = ']';
				} else if(t2 == 0x6F) {
					str_out[i>>1] = '{';
				} else if(t2 == 0x70) {
					str_out[i>>1] = '}';
				} else {
					str_out[i>>1] = '?';
				}
				break;
			case 0x82:
				if((t2 >= 0x4f) && (t2 <= 0x7A)) {
					//digits (0-9), capital letters (A-Z)
					str_out[i>>1] = t2-31;
				} else if( (t2 >= 0x81) && (t2 <= 0x9B) ) {
					//lowercase letters (a-z)
					str_out[i>>1] = t2-32;
				} else if(t2 == 0x3F) {
					str_out[i>>1] = ' ';
				}  else {
					str_out[i>>1] = '?';
				}
				break;			
			default:
				str_out[i>>1] = '?';
				break;
		}
	}
}

void IconSys::EncodeTitle(char const* str_in, unsigned char* str_out) {
	int sjis_count = 0;
	for(size_t i=0; i<strlen(str_in); i++) {
		switch(str_in[i]) {
			case ' ':
				str_out[sjis_count++] = 0x82;
				str_out[sjis_count++] = 0x3F;
				break;
			case ':':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x46;
				break;
			case '/':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x5E;
				break;
			case '(':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x69;
				break;
			case ')':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x6A;
				break;
			case '[':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x6D;
				break;
			case ']':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x6E;
				break;
			case '{':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x6F;
				break;
			case '}':
				str_out[sjis_count++] = 0x81;
				str_out[sjis_count++] = 0x70;
				break;
			default:
				if((str_in[i] >= 48) && (str_in[i] <= 90)) {
					//digits (0-9), capital letters (A-Z)
					str_out[sjis_count++] = 0x82;
					str_out[sjis_count++] = str_in[i] + 31;
				} else if((str_in[i] >= 97) && (str_in[i] <= 122)) {
					//lowercase letters (a-z)
					str_out[sjis_count++] = 0x82;
					str_out[sjis_count++] = str_in[i] + 32;
				}
				break;
		}
	}
	for(; sjis_count < 68; sjis_count++) {
		str_out[sjis_count] = 0;
	}	
}

void IconSys::GetTitleString(char const * str_in, unsigned int pos_linebreak, char * str_out)
{
	unsigned int i, j;
	pos_linebreak = pos_linebreak >> 1;				//remember S-JIS uses two bytes per character!
	if( (pos_linebreak>0) && (pos_linebreak < 33) ) {
		for(i=0, j=0; i<pos_linebreak; i++, j++) {
			str_out[i] = str_in[j];
		}
		str_out[i++] = '\n';
	} else {
		i = 0;
		j = 0;
	}
	for(; (i<35) || (str_in[i] == '\0'); i++, j++) {
		str_out[i] = str_in[j];
	}
}

IconSys::IconSys() 
{
	SetToDefault();
}

IconSys::IconSys(const char* fname)
{
	std::ifstream fin(fname, std::ios_base::in | std::ios_base::binary);
	if(fin.fail()) { throw( Ghulbus::gbException(Ghulbus::gbException::GB_FAILED,
	                                             "Could not open icon file for read") ); }

	fin.read( reinterpret_cast<char*>(&File), sizeof(File) );
	if(fin.fail()) {
		fin.close();
		throw( Ghulbus::gbException(Ghulbus::gbException::GB_FAILED,
			                        "File read error") ); 
	}
	fin.close();

	/*if(!CheckValidity(File)) { 
		throw( Ghulbus::gbException(Ghulbus::gbException::GB_FAILED,
			                        "File seems to be corrupted") ); 
	}*/

	DecodeTitle(File.title, decoded_title);
	GetTitleString(decoded_title, File.offset_2nd_line, title_str);
	strcpy(title_str_single_line, title_str);
	char* tmp = strchr(title_str_single_line, '\n');
	if(tmp) { *tmp = ' '; }
};

IconSys::~IconSys()
{
	;
}

void IconSys::SetToDefault() {
	File.ps2d_string[0] = 'P';  File.ps2d_string[1] = 'S';  
	File.ps2d_string[2] = '2';  File.ps2d_string[3] = 'D';
	File.reserve1 = 0;
	File.offset_2nd_line = 32;
	File.reserve2 = 0;
	File.bg_opacity = 0;
	File.bg_color_upperleft[0]  = 0x80;  File.bg_color_upperleft[1]  = 0x80;
	File.bg_color_upperleft[2]  = 0x80;  File.bg_color_upperleft[3]  = 0x00;
	File.bg_color_upperright[0] = 0x80;  File.bg_color_upperright[1] = 0x80;
	File.bg_color_upperright[2] = 0x80;  File.bg_color_upperright[3] = 0x00;
	File.bg_color_lowerleft[0]  = 0x80;  File.bg_color_lowerleft[1]  = 0x80;
	File.bg_color_lowerleft[2]  = 0x80;  File.bg_color_lowerleft[3]  = 0x00;
	File.bg_color_lowerright[0] = 0x80;  File.bg_color_lowerright[1] = 0x80;
	File.bg_color_lowerright[2] = 0x80;  File.bg_color_lowerright[3] = 0x00;
	File.light1_direction[0] =  0.5f;  File.light1_direction[1] =  0.5f;
	File.light1_direction[2] =  0.5f;  File.light1_direction[3] =  0.0f;
	File.light2_direction[0] =  0.0f;  File.light2_direction[1] = -0.4f;
	File.light2_direction[2] = -0.1f;  File.light2_direction[3] =  0.0f;
	File.light3_direction[0] = -0.5f;  File.light3_direction[1] = -0.5f;
	File.light3_direction[2] =  0.5f;  File.light3_direction[3] =  0.0f;
	File.light1_color[0] = 1.0f;  File.light1_color[1] = 1.0f;
	File.light1_color[2] = 1.0f;  File.light1_color[3] = 0.0f;
	File.light2_color[0] = 1.0f;  File.light2_color[1] = 1.0f;
	File.light2_color[2] = 1.0f;  File.light2_color[3] = 0.0f;
	File.light3_color[0] = 1.0f;  File.light3_color[1] = 1.0f;
	File.light3_color[2] = 1.0f;  File.light3_color[3] = 0.0f;
	File.light_ambient_color[0] = 1.0f;  File.light_ambient_color[1] = 1.0f;
	File.light_ambient_color[2] = 1.0f;  File.light_ambient_color[3] = 0.0f;
	SetTitle("DEFAULT");
	SetIconFilename("ICON.ICN");
	SetIconCopyFilename("ICON.ICN");
	SetIconDeleteFilename("ICON.ICN");
	memset(File.reserve3, 0, 512); 
}

char const * IconSys::GetTitle() const {
	return title_str;
}

void IconSys::SetTitle(char const* str) {
	size_t len = strlen(str);
	if(len > 32) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER,
			                         "Title string exceeds character limit" ) );
	}
	//write ASCII string:
	memcpy(decoded_title, str, len+1);
	//write S-JIS string:
	EncodeTitle(decoded_title, File.title);
	//update linebreaks (remember the / 2 since File is referring to SJIS):
	SetLinebreak(File.offset_2nd_line / 2);
}

void IconSys::SetLinebreak(int lb) {
	if(lb > 32) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER,
									 "Linebreak exceeds character limit" ) );
	}
	File.offset_2nd_line = static_cast<unsigned short>(lb * 2);
	GetTitleString(decoded_title, File.offset_2nd_line, title_str);
	strcpy(title_str_single_line, title_str);
	char* tmp = strchr(title_str_single_line, '\n');
	if(tmp) { *tmp = ' '; }
}

char const * IconSys::GetTitleSingleLine() const {
	return title_str_single_line;
}

char const * IconSys::GetIconFilename() const {
	return reinterpret_cast<char const*>(File.icon_file);
}

void IconSys::SetIconFilename(char const* fname) {
	if(strlen(fname) > 32) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	for(size_t i=0; i<64; i++) {
		if(i < strlen(fname)) {
			File.icon_file[i] = fname[i];
		} else {
			File.icon_file[i] = 0;
		}
	}
}

char const * IconSys::GetIconCopyFilename() const {
	return reinterpret_cast<char const*>(File.icon_copy_file);
}

void IconSys::SetIconCopyFilename(char const* fname) {
	if(strlen(fname) > 32) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	for(size_t i=0; i<64; i++) {
		if(i < strlen(fname)) {
			File.icon_copy_file[i] = fname[i];
		} else {
			File.icon_copy_file[i] = 0;
		}
	}
}

char const * IconSys::GetIconDeleteFilename() const {
	return reinterpret_cast<char const*>(File.icon_delete_file);
}

void IconSys::SetIconDeleteFilename(char const* fname) {
	if(strlen(fname) > 32) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	for(size_t i=0; i<64; i++) {
		if(i < strlen(fname)) {
			File.icon_delete_file[i] = fname[i];
		} else {
			File.icon_delete_file[i] = 0;
		}
	}
}

int IconSys::GetBackgroundOpacity() const {
	if(File.bg_opacity > INT_MAX) { throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED ) ); }
	return static_cast<int>(File.bg_opacity);
}

void IconSys::SetBackgroundOpacity(int i) {
	if((i < 0) || (i > 255)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	File.bg_opacity = i;
}

IconSys::IconSys_Color IconSys::GetBackgroundColor_UL() const {
	return IconSys_Color(File.bg_color_upperleft);
}
void IconSys::SetBackgroundColor_UL(IconSys_Color const& c) {
	c.Get(File.bg_color_upperleft);
}
IconSys::IconSys_Color IconSys::GetBackgroundColor_UR() const {
	return IconSys_Color(File.bg_color_upperright);
}
void IconSys::SetBackgroundColor_UR(IconSys_Color const& c) {
	c.Get(File.bg_color_upperright);
}
IconSys::IconSys_Color IconSys::GetBackgroundColor_LL() const {
	return IconSys_Color(File.bg_color_lowerleft);
}
void IconSys::SetBackgroundColor_LL(IconSys_Color const& c) {
	c.Get(File.bg_color_lowerleft);
}
IconSys::IconSys_Color IconSys::GetBackgroundColor_LR() const {
	return IconSys_Color(File.bg_color_lowerright);
}
void IconSys::SetBackgroundColor_LR(IconSys_Color const& c) {
	c.Get(File.bg_color_lowerright);
}
IconSys::IconSys_LightColor IconSys::GetLight1Color() const {
	return IconSys_LightColor(File.light1_color);
}
void IconSys::SetLight1Color(IconSys_LightColor const& c) {
	c.Get(File.light1_color);
}
IconSys::IconSys_LightColor IconSys::GetLight2Color() const {
	return IconSys_LightColor(File.light2_color);
}
void IconSys::SetLight2Color(IconSys_LightColor const& c) {
	c.Get(File.light2_color);
}
IconSys::IconSys_LightColor IconSys::GetLight3Color() const {
	return IconSys_LightColor(File.light3_color);
}
void IconSys::SetLight3Color(IconSys_LightColor const& c) {
	c.Get(File.light3_color);
}
IconSys::IconSys_LightColor IconSys::GetLightAmbientColor() const {
	return IconSys_LightColor(File.light_ambient_color);
}
void IconSys::SetLightAmbientColor(IconSys_LightColor const& c) {
	c.Get(File.light_ambient_color);
}
IconSys::IconSys_LightVec IconSys::GetLight1Dir() const {
	return IconSys_LightVec(File.light1_direction);
}
void IconSys::SetLight1Dir(IconSys_LightVec const& v) {
	v.Get(File.light1_direction);
}
IconSys::IconSys_LightVec IconSys::GetLight2Dir() const {
	return IconSys_LightVec(File.light2_direction);
}
void IconSys::SetLight2Dir(IconSys_LightVec const& v) {
	v.Get(File.light2_direction);
}
IconSys::IconSys_LightVec IconSys::GetLight3Dir() const {
	return IconSys_LightVec(File.light3_direction);
}
void IconSys::SetLight3Dir(IconSys_LightVec const& v) {
	v.Get(File.light3_direction);
}

void IconSys::WriteFile(const char * fname) {
	std::ofstream fout(fname, std::ios_base::out | std::ios_base::binary);
	if(fout.fail()) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                         "Error opening output file for icon.sys") );
	}
	fout.seekp(std::ios::beg);

	//write data:
	fout.write( reinterpret_cast<char*>(&File), sizeof(File) );
	if(fout.fail()) {
		fout.close();
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                         "Error writing output file for icon.sys") );
	}
	fout.close();
}


//IMPLEMENTATION of IconSys_Color:
IconSys::IconSys_Color::IconSys_Color(int const * p): R(p[0]), G(p[1]), B(p[2]), X(p[3])
{
	CheckRanges();
}
IconSys::IconSys_Color::IconSys_Color(unsigned int const * p): R(p[0]), G(p[1]), B(p[2]), X(p[3])
{
	CheckRanges();
}
IconSys::IconSys_Color::IconSys_Color(int r, int g, int b, int x): R(r), G(g), B(b), X(x)
{ 
	CheckRanges();
}
IconSys::IconSys_Color::IconSys_Color(IconSys_Color const& in): R(in.R), G(in.G), B(in.B), X(in.X)
{
	CheckRanges();
}
IconSys::IconSys_Color::~IconSys_Color()
{ ; }
IconSys::IconSys_Color& IconSys::IconSys_Color::operator =(IconSys::IconSys_Color const& in) {
	if(this != &in) {
		R = in.R;
		G = in.G;
		B = in.B;
		X = in.X;
	}
	return *this;
}
int IconSys::IconSys_Color::GetR() const {
	return R;
}
int IconSys::IconSys_Color::GetG() const {
	return G;
}
int IconSys::IconSys_Color::GetB() const {
	return B;
}
int IconSys::IconSys_Color::GetX() const {
	return X;
}
int IconSys::IconSys_Color::GetR8() const {
	int ret = R<<1;
	return (ret>255)?255:ret;
}
int IconSys::IconSys_Color::GetG8() const {
	int ret = G<<1;
	return (ret>255)?255:ret;
}
int IconSys::IconSys_Color::GetB8() const {
	int ret = B<<1;
	return (ret>255)?255:ret;
}
int IconSys::IconSys_Color::GetX8() const {
	int ret = X<<1;
	return (ret>255)?255:ret;
}
bool IconSys::IconSys_Color::SetR(int r) 
{
	if( (r >= 0) && (r <= 128) ) {		//color values must range between 0x00 and 0x80
		R = r;
		return true;
	}
	return false;
}
bool IconSys::IconSys_Color::SetG(int g) 
{
	if( (g >= 0) && (g <= 128) ) {		//color values must range between 0x00 and 0x80
		G = g;
		return true;
	}
	return false;
}
bool IconSys::IconSys_Color::SetB(int b) 
{
	if( (b >= 0) && (b <= 128) ) {		//color values must range between 0x00 and 0x80
		B = b;
		return true;
	}
	return false;
}
bool IconSys::IconSys_Color::SetX(int x) 
{
	if( (x >= 0) && (x <= 128) ) {		//color values must range between 0x00 and 0x80
		X = x;
		return true;
	}
	return false;
}
void IconSys::IconSys_Color::Get(unsigned int* p) const
{
	p[0] = static_cast<unsigned int>(R);
	p[1] = static_cast<unsigned int>(G);
	p[2] = static_cast<unsigned int>(B);
	p[3] = static_cast<unsigned int>(X);
}
void IconSys::IconSys_Color::CheckRanges() {
	if(R < 0)   { R = 0; }
	if(R > 255) { R = 255; }
	if(G < 0)   { G = 0; }
	if(G > 255) { G = 255; }
	if(B < 0)   { B = 0; }
	if(B > 255) { B = 255; }
	if(X < 0)   { X = 0; }
	if(X > 255) { X = 255; }
}

//IMPLEMENTATION of IconSys_LightColor:
IconSys::IconSys_LightColor::IconSys_LightColor(float const* p) 
: R(p[0]), G(p[1]), B(p[2]), X(p[3])
{
	CheckRanges();
}
IconSys::IconSys_LightColor::IconSys_LightColor(float r, float g, float b, float x)
: R(r), G(g), B(b), X(x)
{
	CheckRanges();
}
IconSys::IconSys_LightColor::IconSys_LightColor(IconSys_LightColor const& rhs)
: R(rhs.R), G(rhs.G), B(rhs.B), X(rhs.X)
{
	CheckRanges();
}
IconSys::IconSys_LightColor::~IconSys_LightColor()
{
	CheckRanges();
}
IconSys::IconSys_LightColor& IconSys::IconSys_LightColor::operator=(IconSys::IconSys_LightColor const& rhs) 
{
	if(this != &rhs) {
		R = rhs.R;  G = rhs.G;  B = rhs.B;  X = rhs.X;
	}
	return (*this);
}
float IconSys::IconSys_LightColor::GetR() const {
	return R;
}
float IconSys::IconSys_LightColor::GetG() const {
	return G;
}
float IconSys::IconSys_LightColor::GetB() const {
	return B;
}
float IconSys::IconSys_LightColor::GetX() const {
	return X;
}
int IconSys::IconSys_LightColor::GetR8() const {
	return (int)(R * 255.0f);
}
int IconSys::IconSys_LightColor::GetG8() const {
	return (int)(G * 255.0f);
}
int IconSys::IconSys_LightColor::GetB8() const {
	return (int)(B * 255.0f);
}
int IconSys::IconSys_LightColor::GetX8() const {
	return (int)(X * 255.0f);
}
bool IconSys::IconSys_LightColor::SetR(float r) {
	if((r >= 0.0f) && (r <= 1.0f)) {
		R = r;
		return true;
	}
	return false;
}
bool IconSys::IconSys_LightColor::SetG(float g) {
	if((g >= 0.0f) && (g <= 1.0f)) {
		G = g;
		return true;
	}
	return false;
}
bool IconSys::IconSys_LightColor::SetB(float b) {
	if((b >= 0.0f) && (b <= 1.0f)) {
		B = b;
		return true;
	}
	return false;
}
bool IconSys::IconSys_LightColor::SetX(float x) {
	if((x >= 0.0f) && (x <= 1.0f)) {
		X = x;
		return true;
	}
	return false;
}
void IconSys::IconSys_LightColor::Get(float* p) const {
	p[0] = R;
	p[1] = G;
	p[2] = B;
	p[3] = X;
}
void IconSys::IconSys_LightColor::CheckRanges() {
	if(R < 0.0f) { R = 0.0f; }
	if(R > 1.0f) { R = 1.0f; }
	if(G < 0.0f) { G = 0.0f; }
	if(G > 1.0f) { G = 1.0f; }
	if(B < 0.0f) { B = 0.0f; }
	if(B > 1.0f) { B = 1.0f; }
	if(X < 0.0f) { X = 0.0f; }
	if(X > 1.0f) { X = 1.0f; }
}

//IMPLEMENTATION of IconSys_LightVec:
IconSys::IconSys_LightVec::IconSys_LightVec(float const* p): X(p[0]), Y(p[1]), Z(p[2]), W(p[3])
{ ; }
IconSys::IconSys_LightVec::IconSys_LightVec(float fx, float fy, float fz, float fw): X(fx), Y(fy), Z(fz), W(fw)
{ ; }
IconSys::IconSys_LightVec::IconSys_LightVec(IconSys_LightVec const& in): X(in.X), Y(in.Y), Z(in.Z), W(in.W)
{ ; }
IconSys::IconSys_LightVec::~IconSys_LightVec()
{ ; }
IconSys::IconSys_LightVec& IconSys::IconSys_LightVec::operator=(IconSys::IconSys_LightVec const& in)
{
	if(this != &in) {
		X = in.X;
		Y = in.Y;
		Z = in.Z;
		W = in.W;
	}
	return *this;
}
float IconSys::IconSys_LightVec::GetX() const {
	return X;
}
float IconSys::IconSys_LightVec::GetY() const {
	return Y;
}
float IconSys::IconSys_LightVec::GetZ() const {
	return Z;
}
float IconSys::IconSys_LightVec::GetW() const {
	return W;
}
bool IconSys::IconSys_LightVec::SetX(float x) {
	X = x;
	return true;
}
bool IconSys::IconSys_LightVec::SetY(float y) {
	Y = y;
	return true;
}
bool IconSys::IconSys_LightVec::SetZ(float z) {
	Z = z;
	return true;
}
bool IconSys::IconSys_LightVec::SetW(float w) {
	W = w;
	return true;
}
void IconSys::IconSys_LightVec::Get(float* p) const {
	p[0] = X;
	p[1] = Y;
	p[2] = Z;
	p[3] = W;
}
