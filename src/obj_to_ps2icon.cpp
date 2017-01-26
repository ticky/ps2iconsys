/**
 * @file src/obj_to_ps2icon.cpp
 *
 * @brief A tool for converting Wavefront OBJ to PS2 Icons
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include <iostream>
#include "../include/ps2_ps2icon.hpp"
#include "../include/obj_loader.hpp"
#include "../gbLib/include/gbException.hpp"
#include "../gbLib/include/gbImageLoader.hpp"

char const* obj_input_file     = NULL;		///< path to the input file
int obj_mesh_index             = 0;			///< 0-based index of the mesh to use
char const* ps2_output_file    = NULL;		///< path to the output file
char const* texture_input_file = NULL;		///< path to the texture file (tga or bmp)
bool verbose_output            = false;		///< flag for verbose output
bool list_obj_file             = false;		///< flag for obj content listing
float obj_scale_factor         = 0.0f;		///< geometric scale factor for conversion

/** Print a help text on screen
 * @param[in] self Name of the executable (e.g. obtained from argv[0])
 */
void PrintHelp(char* self)
{
	std::cout << "********************************************************"  << "\n"
		      << " *** OBJ to PS2Icon Converter  V-1.0                ***"   << "\n"
		      << "  **  by Ghulbus Inc.  (http://www.ghulbus-inc.de/) **"    << "\n"
			  << "   **************************************************"     << "\n"
			  << "\n"
			  << " Usage: " << self << " [OPTION]..."                        << "\n"
			  << "Build a PS2Icon from a Wavefront OBJ file."                << "\n"
			  << "\n"
			  << "  -h, --help           display this help"                  << "\n"
			  << "  -f, --input-file     Wavefront OBJ file used as input"   << "\n"
			  << "  -o, --output-file    Name of the destination file"       << "\n"
			  << "  -t, --input-texture  Texture file used as input (must be BMP or TGA)" << "\n"
			  << "  -m, --mesh-index     Index of the OBJ mesh to use (0-based)"          << "\n"
			  << "  -s, --scale-factor   Scale factor that is applied to geometry"        << "\n"
			  << "  -v, --verbose        activate verbose output"                         << "\n"
			  << "  -l, --list-obj-file  list the meshes contained in input"              << "\n"
			  << "\n"
			  << " Examples:"                                                              << "\n"
			  << "  " << self << " -f foo.obj"                                            << "\n"
			  << "Converts the first mesh in file foo.obj to an icon file default.icn"    << "\n"
			  << "using a default texture."                                               << "\n"
			  << "\n"
			  << "  " << self << " -f foo.obj -m 3 -s 0.5 -t bar.tga -o out.icn"          << "\n"
			  << "Converts the fourth mesh in file foo.obj to an icon file out.icn"       << "\n"
			  << "using the image from bar.tga as a texture and scaling the geometry"     << "\n"
			  << "to half the size before writing."                                       << "\n"
			  << "\n"
			  << "  " << self << " -f foo.obj -l"                                         << "\n"
			  << "Prints a list of all meshes in foo.obj. No files are written."          << "\n"
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
		} else if( (strcmp( argv[i], "-l" ) == 0) || (strcmp( argv[i], "--list-obj-file" ) == 0) ) {
			list_obj_file = true;
		} else if( (strcmp( argv[i], "-v" ) == 0) || (strcmp( argv[i], "--verbose" ) == 0) ) {
			verbose_output = true;
		} else if(i < argc-1) {
		//Parameters with 1 argument
			if( (strcmp( argv[i], "-f" ) == 0) || (strcmp( argv[i], "--input-file" ) == 0) ) {
				obj_input_file = argv[++i];
			} else if( (strcmp( argv[i], "-t" ) == 0) || (strcmp( argv[i], "--input-texture" ) == 0) ) {
				texture_input_file = argv[++i];
			} else if( (strcmp( argv[i], "-o" ) == 0) || (strcmp( argv[i], "--output-file" ) == 0) ) {
				ps2_output_file = argv[++i];
			} else if( (strcmp( argv[i], "-m" ) == 0) || (strcmp( argv[i], "--mesh-index" ) == 0) ) {
				obj_mesh_index = atoi(argv[++i]);
			} else if( (strcmp( argv[i], "-s" ) == 0) || (strcmp( argv[i], "--scale-factor" ) == 0) ) {
				obj_scale_factor = static_cast<float>(atof(argv[++i]));
			} else {
				std::cout << "Invalid argument." << std::endl << std::endl;
				PrintHelp(argv[0]);
				exit(1);
			}
		} else {
			std::cout << "Invalid argument." << std::endl << std::endl;
			PrintHelp(argv[0]);
			exit(1);
		}
	}
}

/** Load the obj file
 */
OBJ_FileLoader* LoadOBJFile()
{
	OBJ_FileLoader* ret = NULL;
	if(verbose_output)
		std::cout << " * Reading OBJ file \"" << obj_input_file << "\"...";
	try {
		ret = new OBJ_FileLoader(obj_input_file);
	} catch(Ghulbus::gbException e) {
		std::cout << "\nFile read error: \"" << obj_input_file << "\"" << std::endl;
		exit(1);
	}
	if(verbose_output)
		std::cout << "done." << std::endl;

	if(obj_mesh_index >= ret->GetNMeshes()) {
		std::cout << "Invalid mesh index. Index given: " << obj_mesh_index << "; Maximum allowed for \"" 
			<< obj_input_file << "\": " << (ret->GetNMeshes() - 1) << std::endl;
		exit(1);
	}
	return ret;
}

/** Print a list of all meshes contained in an obj file
 * @param[in] obj_file Working loader to the file to list
 */
void ListOBJFile(OBJ_FileLoader const* obj_file)
{
	std::cout << " * Parsing OBJ file \"" << obj_input_file << "\" contents...\n";
	std::cout << " **  Found " << obj_file->GetNMeshes() << " meshes: " << std::endl;
	for(int i=0; i<obj_file->GetNMeshes(); ++i) {
		OBJ_Mesh const* tmp = obj_file->GetMesh(i);
		std::cout << " **   #" << i << ": " << tmp->GetName() << " - " 
			<< tmp->GetNFaces() << " Triangles, " << tmp->GetNVertices() << " Vertices\n";
	}
	std::cout << " *  done." << std::endl;
}

/** Helper function: Is f a path to a BMP file?
 */
bool IsBMP(char const* f)
{
	//check for .bmp ending:
	int len = static_cast<int>(strlen(f));
	if( (f[len-3] == 'B' || f[len-3] == 'b') && 
		(f[len-2] == 'M' || f[len-2] == 'm') && 
		(f[len-1] == 'P' || f[len-1] == 'p') ) {
			GhulbusUtil::gbImageType_BMP_T checker;
			std::ifstream tmp(f, std::ios_base::in | std::ios_base::binary);
			return checker.CheckFile(tmp);
	}
	return false;
}

/** Load a texture file
 */
GhulbusUtil::gbImageLoader* LoadTexture()
{
	GhulbusUtil::gbImageLoader* ret = NULL;
	if(IsBMP(texture_input_file)) {
		try {
			ret = new GhulbusUtil::gbImageLoader( texture_input_file, GhulbusUtil::gbImageType_BMP() );
		} catch( Ghulbus::gbException e ) {
			std::cout << "\"" << texture_input_file << "\" is no valid BMP file." << std::endl;
			exit(1);
		}
	} else {
		try {
			ret = new GhulbusUtil::gbImageLoader( texture_input_file, GhulbusUtil::gbImageType_TGA() );
		} catch( Ghulbus::gbException e ) {
			std::cout << "\"" << texture_input_file << "\" is no valid TGA file." << std::endl;
			exit(1);
		}
	}
	if( (ret->GetWidth() != 128) || (ret->GetHeight() != 128) ) {
		std::cout << "Only Textures of size 128x128 allowed! \"" << texture_input_file << "\" has "
			<< ret->GetWidth() << "x" << ret->GetHeight() << std::endl;
		exit(1);
	}
	return ret;
}

/** Write a PS2Icon file
 */
void WriteOutputFile(OBJ_FileLoader const* obj_file, GhulbusUtil::gbImageLoader* img_loader)
{
	PS2Icon ps2_icon;
	if(img_loader) {
		if(verbose_output)
			std::cout << " * Copying texture data from \"" << texture_input_file << "\"...";
		unsigned int* tmp = new unsigned int[img_loader->GetWidth() * img_loader->GetHeight()];
		img_loader->FlipV();
		img_loader->GetImageData32(tmp);
		ps2_icon.SetTextureData(tmp);
		delete[] tmp;
		if(verbose_output)
			std::cout << "done." << std::endl;
	}
	OBJ_Mesh const* tmp = obj_file->GetMesh(obj_mesh_index);
	if(verbose_output)
		std::cout << " * Copying geometry data from \"" << obj_input_file << "\": Mesh #" << obj_mesh_index
			<< " - " << tmp->GetName() << "...";
	if(obj_scale_factor != 0.0f) {
		if(verbose_output)
			std::cout << "\n    Scale factor is " << obj_scale_factor << " ...";
		if(obj_scale_factor < 0.0f) {
			std::cout << "\n!WARNING! Scale factor is negative.\n    ";
		}
		ps2_icon.SetGeometry(*tmp, obj_scale_factor);
	} else {
		ps2_icon.SetGeometry(*tmp);
	}
	if(verbose_output)
		std::cout << "done." << std::endl;
	
	if(verbose_output)
		std::cout << " * Writing output to \"" << ps2_output_file << "\"...";
	try {
		ps2_icon.WriteFile(ps2_output_file);
	} catch(Ghulbus::gbException e) {
		std::cout << "\nError while writing to \"" << ps2_output_file << "\"" << std::endl;
		exit(1);
	}
	if(verbose_output)
		std::cout << "done." << std::endl;
}

int main(int argc, char* argv[])
{
	ParseCommandLine(argc, argv);

	if(!obj_input_file) {
		std::cout << "No input file specified." << std::endl << std::endl;
		PrintHelp(argv[0]);
		exit(1);
	}
	std::cout << "OBJ to PS2Icon Converter  V-1.0\n by Ghulbus Inc.  (http://www.ghulbus-inc.de/)\n" << std::endl;
	if((!list_obj_file) && (!ps2_output_file)) { ps2_output_file = "default.icn"; }

	OBJ_FileLoader* obj_file = LoadOBJFile();

	if(list_obj_file) {
		ListOBJFile(obj_file);
	}

	GhulbusUtil::gbImageLoader* img_loader = NULL;
	if(texture_input_file) {
		img_loader = LoadTexture();
	}
	
	if(ps2_output_file) {
		WriteOutputFile(obj_file, img_loader);
	}

	delete img_loader;
	delete obj_file;

	std::cout << "Success :)" << std::endl;

	return 0;
}
