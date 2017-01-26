/**
 * @file src/ps2icon_to_obj.cpp
 *
 * @brief A tool for converting PS2 Icons to OBJ
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include <iostream>
#include "../include/ps2_ps2icon.hpp"
#include "../include/obj_loader.hpp"
#include "../gbLib/include/gbException.hpp"
#include "../gbLib/include/gbColor.hpp"
#include "../gbLib/include/gbImageLoader.hpp"

char const* ps2_input_file      = NULL;		///< path to the input file
char const* obj_output_file     = NULL;		///< path to the output file
char const* texture_output_file = NULL;		///< path to the output texture file
bool verbose_output             = false;	///< flag for verbose output

/** Print a help text on screen
 * @param[in] self Name of the executable (e.g. obtained from argv[0])
 */
void PrintHelp(char* self)
{
	std::cout << "********************************************************"    << "\n"
		      << " *** PS2Icon to OBJ Converter  V-1.0                ***"     << "\n"
		      << "  **  by Ghulbus Inc.  (http://www.ghulbus-inc.de/) **"      << "\n"
			  << "   **************************************************"       << "\n"
			  << "\n"
			  << " Usage: " << self << " [OPTION]..."                          << "\n"
			  << "Extract geometry and texture from a PS2Icon file"            << "\n"
			  << "\n"
			  << "  -h,  --help            display this help"                  << "\n"
			  << "  -f,  --input-file      PS2Icon file used as input"         << "\n"
			  << "  -o,  --output-file     Name of the OBJ destination file"   << "\n"
			  << "  -ot, --output-texture  Texture file output (TGA)"          << "\n"
			  << "  -v,  --verbose         activate verbose output"            << "\n"
			  << "\n"
			  << " Examples:"                                                             << "\n"
			  << "  " << self << " -f foo.icn"                                            << "\n"
			  << "Extracts geometry and texture info from foo.icn to default.obj and"     << "\n"
			  << "default.tga."                                                           << "\n"
			  << "\n"
			  << "  " << self << " -f foo.icn -o out.obj -ot out.tga"                     << "\n"
			  << "Extracts geometry and texture info from foo.icn and saves it out to"    << "\n"
			  << "out.obj and out.tga."                                                   << "\n"
			  << std::endl;
}

/** Parse the command line arguments and set globals accordingly
 * @param[in] argc argc
 * @param[in] argv argv
 */
void ParseCommandLine(int argc, char* argv[])
{
	for(int i=1; i<argc; i++) {
		//Flag parameters:
		if( (strcmp( argv[i], "-h" ) == 0) || (strcmp( argv[i], "--help" ) == 0) ) {
			PrintHelp(argv[0]);
			exit(0);
		} else if( (strcmp( argv[i], "-v" ) == 0) || (strcmp( argv[i], "--verbose" ) == 0) ) {
			verbose_output = true;
		} else if(i < argc-1) {
		//Parameters with 1 argument
			if( (strcmp( argv[i], "-f" ) == 0) || (strcmp( argv[i], "--input-file" ) == 0) ) {
				ps2_input_file = argv[++i];
			} else if( (strcmp( argv[i], "-o" ) == 0) || (strcmp( argv[i], "--output-file" ) == 0) ) {
				obj_output_file = argv[++i];
			} else if( (strcmp( argv[i], "-ot" ) == 0) || (strcmp( argv[i], "--output-texture" ) == 0) ) {
				texture_output_file = argv[++i];
			} else {
				std::cout << "Invalid argument.\n" << std::endl;
				PrintHelp(argv[0]);
				exit(1);
			}
		} else {
			std::cout << "Invalid argument.\n" << std::endl;
			PrintHelp(argv[0]);
			exit(1);
		}
	}
}

PS2Icon* LoadPS2Icon()
{
	PS2Icon* ret = NULL;
	if(verbose_output)
		std::cout << " * Reading PS2Icon file \"" << ps2_input_file << "\"...\n";
	try {
		ret = new PS2Icon(ps2_input_file);
	} catch( std::exception e ) {
		std::cout << "File read error: \"" << ps2_input_file << "\"" << std::endl;
		exit(1);
	}
	if(verbose_output)
		std::cout << " **  Found geometry - " << ret->GetNVertices() << " vertices, " 
			<< ret->GetNShapes() << " shapes." << std::endl;
	if(ret->GetNFrames() > 1) {
		std::cout << " **  Found animation - " << ret->GetNFrames() << " frames." << std::endl;
	}
	if(verbose_output)
		std::cout << " *  done." << std::endl;
	return ret;
}

void WriteOBJFile(PS2Icon* ps2_icon)
{
	OBJ_FileLoader obj_file;
	OBJ_Mesh obj_mesh(ps2_input_file);
	if(verbose_output)
		std::cout << " * Convert geometry data from \"" << ps2_input_file << "\"...";
	ps2_icon->BuildMesh(&obj_mesh);
	if(verbose_output)
		std::cout << "done." << std::endl;

	if(verbose_output)
		std::cout << " * Writing geometry output to file \"" << obj_output_file << "\"...";
	obj_file.AddMesh(obj_mesh);
	try {
		obj_file.WriteFile(obj_output_file);
	} catch( Ghulbus::gbException e ) {
		std::cout << "\nError while writing to \"" << obj_output_file << "\"" << std::endl;
		exit(1);
	}
	if(verbose_output)
		std::cout << "done." << std::endl;
}

void WriteTextureFile(PS2Icon* ps2_icon)
{
	unsigned int texture_data[128*128];
	if(verbose_output)
		std::cout << " * Convert texture data from \"" << ps2_input_file << "\"..." ;
	ps2_icon->GetTextureData(texture_data);
	//manual conversion required, since WriteImage() requires 
	// GBCOLOR32 data, whose bit pattern is not fix;
	for(int i=0; i<16384; i++) {
		texture_data[i] = GhulbusGraphics::GBCOLOR32::ARGB( static_cast<int>((texture_data[i] >> 24) & 0xff),
															static_cast<int>((texture_data[i] >> 16) & 0xff),
															static_cast<int>((texture_data[i] >>  8) & 0xff),
															static_cast<int>((texture_data[i])       & 0xff) );
	}
	//in addition the texture is flipped horizontally:
	for(int row=0; row<64; row++) {
		for(int i=0; i<128; i++) {
			texture_data[row*128 + i] ^= texture_data[(127-row)*128 + i] 
				^= texture_data[row*128 + i] ^= texture_data[(127-row)*128 + i];
		}
	}

	if(verbose_output)
		std::cout << "done." << std::endl;

	if(verbose_output)
		std::cout << " * Writing texture to file \"" << texture_output_file << "\"...";
	try {
		GhulbusUtil::WriteImage(texture_output_file, texture_data, 128, 128);
	} catch( Ghulbus::gbException e ) {
		std::cout << "\nError while writing to \"" << texture_output_file << "\"" << std::endl;
		exit(1);
	}
	if(verbose_output)
		std::cout << "done." << std::endl;
}

int main(int argc, char* argv[])
{
	ParseCommandLine(argc, argv);

	if(!ps2_input_file) {
		std::cout << "No input file specified.\n" << std::endl;
		PrintHelp(argv[0]);
		exit(1);
	}
	std::cout << "PS2Icon to OBJ Converter  V-1.0\n by Ghulbus Inc.  (http://www.ghulbus-inc.de/)\n" << std::endl;
	if(!obj_output_file)     { obj_output_file = "default.obj"; }
	if(!texture_output_file) { texture_output_file = "default.tga"; }

	PS2Icon* ps2_icon = LoadPS2Icon();

	WriteOBJFile(ps2_icon);

	WriteTextureFile(ps2_icon);
	
	std::cout << "Success :)" << std::endl;

	return 0;
}
