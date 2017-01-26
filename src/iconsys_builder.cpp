/**
 * @file src/iconsys_builder.hpp
 *
 * @brief A tool for creating and manipulating Icon.sys files
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include <iostream>
#include <iomanip>
#include "../include/ps2_iconsys.hpp"

char const* input_file   = NULL;	///< name of the input file
char const* output_file  = NULL;	///< name of the output file
bool list_file           = false;	///< flag for file listing before writing
bool verbose_output      = false;	///< flag for verbose output
char* title_string       = NULL;	///< new title string
int title_linebreak      = 32;		///< index of title linebreak
char* icon_string        = NULL;	///< icon filename
char* icon_copy_string   = NULL;	///< copy icon filename
char* icon_delete_string = NULL;	///< delete icon filename
int bg_opacity           = 0;		///< background opacity
float* light1_dir        = NULL;	///< direction vector light #1
float* light2_dir        = NULL;	///< direction vector light #2
float* light3_dir        = NULL;	///< direction vector light #3
int* light1_color        = NULL;	///< color vector light #1
int* light2_color        = NULL;	///< color vector light #2
int* light3_color        = NULL;	///< color vector light #3
int* ambient_color       = NULL;	///< color vector ambient light
int* bg_color_ul         = NULL;	///< color vector upper left
int* bg_color_ur         = NULL;	///< color vector upper right
int* bg_color_ll         = NULL;	///< color vector lower left
int* bg_color_lr         = NULL;	///< color vector lower right

/** Print a help text on screen
 * @param[in] self Name of the executable (e.g. obtained from argv[0])
 */
void PrintHelp(char* self)
{
	std::cout << "********************************************************"    << "\n"
		      << " *** PS2 Icon.Sys Builder  V-1.0                     ***"    << "\n"
		      << "  **  by Ghulbus Inc.  (http://www.ghulbus-inc.de/) **"      << "\n"
			  << "   **************************************************"       << "\n"
			  << "\n"
			  << " Usage: " << self << " [OPTION]..."                          << "\n"
			  << "Build or manipulate a PS2 icon.sys file"                     << "\n"
			  << "\n"
			  << "  -h, --help           display this help"                              << "\n"
			  << "  -f, --input-file     Existing icon.sys file used as input"           << "\n"
			  << "  -o, --output-file    Name of the destination file"                   << "\n"
			  << "  -v, --verbose        activate verbose output"                        << "\n"
			  << "  -l, --list-file      list the file data before writing"              << "\n"
			  << "\n"
			  << "   --set-title       Set the title string"                                << "\n"
			  << "   --title-linebreak Set the index in the title string where a linebreak" << "\n"
			  << "                     should be inserted"                                  << "\n"
			  << "   --set-icon        Set the standard icon filename"                      << "\n"
			  << "                      Implicitly changes copy and delete icons as well,"  << "\n"
			  << "                      unless they are specified explicitly."              << "\n"
			  << "   --set-copy-icon   Set the copy icon filename"                          << "\n"
			  << "   --set-delete-icon Set the delete icon filename"                        << "\n"
			  << "   --light-<n>       Set the direction of light <n>"                      << "\n"
			  << "                      <n> has to be one of 1, 2 or 3"                     << "\n"
			  << "                      Takes four parameters for each x, y, z and w"       << "\n"
			  << "   --lcolor-<n>      Set the color of light <n>"                          << "\n"
			  << "                      <n> has to be one of 1, 2, 3 or a (for ambient)"    << "\n"
			  << "                      Takes four parameters for each r, g, b and x"       << "\n"
			  << "   --color-<n>       Set the background color of corner <n>"              << "\n"
			  << "                      <n> has to be one of 1, 2, 3, or 4"                 << "\n"
			  << "                           1- Upper left, 2- Upper right,"                << "\n"
			  << "                           3- Lower left, 4- Lower right"                 << "\n"
			  << "                      Takes four parameters for each r, g, b and x"       << "\n"
			  << "   --set-opacity      Set the background opacity"                         << "\n"
			  << "\n"
			  << " Notes:"                                                                << "\n"
			  << " * If no output file is specified, all output will be written"          << "\n"
			  << "   to a file icon.sys"                                                  << "\n"
			  << " * If no input file is specified, default values will be used"          << "\n"
			  << "   for all values not explicitly specified as parameters"               << "\n"
			  << " * All color and opacity values have to be in a range [0..255]"         << "\n"
			  << "\n"
			  << " Examples:"                                                             << "\n"
			  << "  " << self << " --set-title \"Test Icon\""                             << "\n"
			  << "Creates a new default icon.sys with title \"Test Icon\" and saves it"   << "\n"
			  << "to a file icon.sys."                                                    << "\n"
			  << "\n"
			  << "  " << self << " -f myicon --lcolor-1 255 255 0 255"                    << "\n"
			  << "Opens the existing icon.sys in file myicon, changes the color of the"   << "\n"
			  << "first light source to yellow and saves the result to icon.sys."         << "\n"
			  << "\n"
			  << "  " << self << " -f myicon --set-icon my_icon.icn -o myicon"            << "\n"
			  << "Opens the existing file myicon, changes the standard, copy and delete"  << "\n"
			  << "icon to my_icon.icn and saves the result back to myicon."               << "\n"
			  << "\n"
			  << "  " << self << " -f myicon -l"                                          << "\n"
			  << "Prints a listing of the data in the existing file myicon and saves a"   << "\n"
			  << "copy to icon.sys."                                                      << "\n"
			  << std::endl;
}

/** Helper function for ParseCommandLine()
 */
bool ParseCommandLine_1Arg(int i, char* argv[])
{
	//Parameters with 1 argument
	if( (strcmp( argv[i], "-f" ) == 0) || (strcmp( argv[i], "--input-file" ) == 0) ) {
		input_file = argv[++i];
	} else if( (strcmp( argv[i], "-o" ) == 0) || (strcmp( argv[i], "--output-file" ) == 0) ) {
		output_file = argv[++i];
	} else if(strcmp( argv[i], "--set-title" ) == 0) {
		title_string = new char[strlen(argv[i+1]) + 1];
		strcpy(title_string, argv[i+1]);
		i++;
	} else if(strcmp( argv[i], "--title-linebreak" ) == 0) {
		title_linebreak = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--set-icon" ) == 0) {
		icon_string = new char[strlen(argv[i+1]) + 1];
		strcpy(icon_string, argv[i+1]);
		i++;
	} else if(strcmp( argv[i], "--set-copy-icon" ) == 0) {
		icon_copy_string = new char[strlen(argv[i+1]) + 1];
		strcpy(icon_copy_string, argv[i+1]);
		i++;
	} else if(strcmp( argv[i], "--set-delete-icon" ) == 0) {
		icon_delete_string = new char[strlen(argv[i+1]) + 1];
		strcpy(icon_delete_string, argv[i+1]);
		i++;
	} else if(strcmp( argv[i], "--set-opacity" ) == 0) {
		bg_opacity = atoi(argv[++i]);
	} else {
		return false;
	}
	return true;
}
/** Helper function for ParseCommandLine()
 */
bool ParseCommandLine_4Args(int i, char* argv[])
{
	//Parameters with 4 arguments
	if(strcmp( argv[i], "--light-1" ) == 0) {
		light1_dir = new float[4];
		light1_dir[0] = static_cast<float>( atof(argv[++i]) );  
		light1_dir[1] = static_cast<float>( atof(argv[++i]) );
		light1_dir[2] = static_cast<float>( atof(argv[++i]) );
		light1_dir[3] = static_cast<float>( atof(argv[++i]) );
	} else if(strcmp( argv[i], "--light-2" ) == 0) {
		light2_dir = new float[4];
		light2_dir[0] = static_cast<float>( atof(argv[++i]) );
		light2_dir[1] = static_cast<float>( atof(argv[++i]) );
		light2_dir[2] = static_cast<float>( atof(argv[++i]) );
		light2_dir[3] = static_cast<float>( atof(argv[++i]) );
	} else if(strcmp( argv[i], "--light-3" ) == 0) {
		light3_dir = new float[4];
		light3_dir[0] = static_cast<float>( atof(argv[++i]) );
		light3_dir[1] = static_cast<float>( atof(argv[++i]) );
		light3_dir[2] = static_cast<float>( atof(argv[++i]) );
		light3_dir[3] = static_cast<float>( atof(argv[++i]) );
	} else if(strcmp( argv[i], "--lcolor-1" ) == 0) {
		light1_color = new int[4];
		light1_color[0] = atoi(argv[++i]);  light1_color[1] = atoi(argv[++i]);
		light1_color[2] = atoi(argv[++i]);  light1_color[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--lcolor-2" ) == 0) {
		light2_color = new int[4];
		light2_color[0] = atoi(argv[++i]);  light2_color[1] = atoi(argv[++i]);
		light2_color[2] = atoi(argv[++i]);  light2_color[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--lcolor-3" ) == 0) {
		light3_color = new int[4];
		light3_color[0] = atoi(argv[++i]);  light3_color[1] = atoi(argv[++i]);
		light3_color[2] = atoi(argv[++i]);  light3_color[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--lcolor-a" ) == 0) {
		ambient_color = new int[4];
		ambient_color[0] = atoi(argv[++i]);  ambient_color[1] = atoi(argv[++i]);
		ambient_color[2] = atoi(argv[++i]);  ambient_color[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--color-1" ) == 0) {
		bg_color_ul = new int[4];
		bg_color_ul[0] = atoi(argv[++i]);  bg_color_ul[1] = atoi(argv[++i]);
		bg_color_ul[2] = atoi(argv[++i]);  bg_color_ul[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--color-2" ) == 0) {
		bg_color_ur = new int[4];
		bg_color_ur[0] = atoi(argv[++i]);  bg_color_ur[1] = atoi(argv[++i]);
		bg_color_ur[2] = atoi(argv[++i]);  bg_color_ur[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--color-3" ) == 0) {
		bg_color_ll = new int[4];
		bg_color_ll[0] = atoi(argv[++i]);  bg_color_ll[1] = atoi(argv[++i]);
		bg_color_ll[2] = atoi(argv[++i]);  bg_color_ll[3] = atoi(argv[++i]);
	} else if(strcmp( argv[i], "--color-4" ) == 0) {
		bg_color_lr = new int[4];
		bg_color_lr[0] = atoi(argv[++i]);  bg_color_lr[1] = atoi(argv[++i]);
		bg_color_lr[2] = atoi(argv[++i]);  bg_color_lr[3] = atoi(argv[++i]);
	} else {
		return false;
	}
	return true;
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
		} else if( (strcmp( argv[i], "-l" ) == 0) || (strcmp( argv[i], "--list-file" ) == 0) ) {
			list_file = true;
		} else if( (strcmp( argv[i], "-v" ) == 0) || (strcmp( argv[i], "--verbose" ) == 0) ) {
			verbose_output = true;
		} else if( (i < argc - 1) && (ParseCommandLine_1Arg(i, argv)) ) {
			i++;
		} else if( (i < argc - 4) && (ParseCommandLine_4Args(i, argv)) ) {
			i += 4;
		} else {
			std::cout << "Invalid argument." << std::endl << std::endl;
			PrintHelp(argv[0]);
			exit(1);
		}
	}
}

/** Helper function for CheckParameters()
 */
void CheckColor(int* c) {
	if(c) {		//NULL pointer is okay
		for(int i=0; i<4; i++) {
			if((c[i] < 0) || (c[i] > 255)) {		//numbers must range between 0..255
				std::cout << "All color values must range between 0 and 255." << std::endl;
				exit(1);
			}
		}
	}
}

/** Check if the supplied parameters were sane
 */
void CheckParameters()
{
	CheckColor(light1_color);
	CheckColor(light2_color);
	CheckColor(light3_color);
	CheckColor(ambient_color);
	CheckColor(bg_color_ul);
	CheckColor(bg_color_ur);
	CheckColor(bg_color_ll);
	CheckColor(bg_color_lr);
	if((bg_opacity < 0) || (bg_opacity > 255)) {
		std::cout << "Background opacity must range between 0 and 255." << std::endl;
		exit(1);
	}
	if((title_linebreak <= 0) || (title_linebreak > 32)) {
		std::cout << "Invalid title linebreak." << std::endl;
		exit(1);
	}
	if(title_string) {
		if(strlen(title_string) > 32) {
			std::cout << "Title string exceeds character limit." << std::endl;
			exit(1);
		}
	}
}

/** Adjust icon_sys according to the parameters
 */
void ProcessParameters(IconSys* icon_sys)
{
	if(verbose_output) 
		std::cout << " * Adjusting parameters...";
	if(title_string) {
		icon_sys->SetTitle(title_string);
	}
	icon_sys->SetLinebreak(title_linebreak);
	if(icon_string) {
		try {
			icon_sys->SetIconFilename(icon_string);
			icon_sys->SetIconCopyFilename(icon_string);
			icon_sys->SetIconDeleteFilename(icon_string);
		} catch (Ghulbus::gbException e) {
			std::cout << "Icon filename exceeds character limit." << std::endl;
			exit(1);
		}
	}
	if(icon_copy_string) {
		try {
			icon_sys->SetIconCopyFilename(icon_copy_string);
		} catch (Ghulbus::gbException e) {
			std::cout << "Copy icon filename exceeds character limit." << std::endl;
			exit(1);
		}
	}
	if(icon_delete_string) {
		try {
			icon_sys->SetIconDeleteFilename(icon_delete_string);
		} catch (Ghulbus::gbException e) {
			std::cout << "Delete icon name exceeds character limit." << std::endl;
			exit(1);
		}
	}
	icon_sys->SetBackgroundOpacity(bg_opacity);
	if(light1_dir) { icon_sys->SetLight1Dir( IconSys::IconSys_LightVec(light1_dir) ); }
	if(light2_dir) { icon_sys->SetLight2Dir( IconSys::IconSys_LightVec(light2_dir) ); }
	if(light3_dir) { icon_sys->SetLight3Dir( IconSys::IconSys_LightVec(light3_dir) ); }
	if(light1_color) { 
		float tmp[4];
		for(int i=0; i<4; i++) { tmp[i] = static_cast<float>(light1_color[i]) / 255.0f; }
		icon_sys->SetLight1Color( IconSys::IconSys_LightColor(tmp) ); 
	}
	if(light2_color) { 
		float tmp[4];
		for(int i=0; i<4; i++) { tmp[i] = static_cast<float>(light2_color[i]) / 255.0f; }
		icon_sys->SetLight2Color( IconSys::IconSys_LightColor(tmp) ); 
	}
	if(light3_color) { 
		float tmp[4];
		for(int i=0; i<4; i++) { tmp[i] = static_cast<float>(light3_color[i]) / 255.0f; }
		icon_sys->SetLight3Color( IconSys::IconSys_LightColor(tmp) ); 
	}
	if(ambient_color) { 
		float tmp[4];
		for(int i=0; i<4; i++) { tmp[i] = static_cast<float>(ambient_color[i]) / 255.0f; }
		icon_sys->SetLightAmbientColor( IconSys::IconSys_LightColor(tmp) ); 
	}
	if(bg_color_ul) { icon_sys->SetBackgroundColor_UL( IconSys::IconSys_Color( bg_color_ul ) ); }
	if(bg_color_ur) { icon_sys->SetBackgroundColor_UR( IconSys::IconSys_Color( bg_color_ur ) ); }
	if(bg_color_ll) { icon_sys->SetBackgroundColor_LL( IconSys::IconSys_Color( bg_color_ll ) ); }
	if(bg_color_lr) { icon_sys->SetBackgroundColor_LR( IconSys::IconSys_Color( bg_color_lr ) ); }
	if(verbose_output) 
		std::cout << "done." << std::endl;
}

/** Helper function for ListFile()
 */
template<typename T>
void PrintColors(T t)
{
	std::cout << std::hex << std::setfill('0')
		<< "0x" << std::setw(2) << t.GetR8() << ", 0x" << std::setw(2) << t.GetG8() 
		<< ", 0x" << std::setw(2) << t.GetB8() << ", 0x" << std::setw(2) << t.GetX8() 
		<< std::dec << std::setfill(' ') << std::endl;
}

/** Print the contents of icon_sys to the console
 */
void ListFile(IconSys* icon_sys) 
{
	std::cout << " * Listing file... " << std::endl;
	std::cout << " ** Title  \"" << icon_sys->GetTitleSingleLine() << "\"" << std::endl;
	std::cout << " ** Icon         \"" << icon_sys->GetIconFilename() << "\"" << std::endl;
	std::cout << " ** Icon Copy    \"" << icon_sys->GetIconCopyFilename() << "\"" << std::endl;
	std::cout << " ** Icon Delete  \"" << icon_sys->GetIconDeleteFilename() << "\"" << std::endl;
	std::cout << " ** Background Opacity: 0x" << std::hex << std::setfill('0') 
		<< std::setw(2) <<  icon_sys->GetBackgroundOpacity() 
		<< std::dec << std::setfill(' ') << std::endl;
	{
		IconSys::IconSys_Color t(0, 0, 0, 0);
		std::cout << " ** Background Colors  (R, G, B, X)" << std::endl;
		t = icon_sys->GetBackgroundColor_UL();
		std::cout << " **  Upper Left:   ";
		PrintColors(t);
		t = icon_sys->GetBackgroundColor_UR();
		std::cout << " **  Upper Right:  ";
		PrintColors(t);
		t = icon_sys->GetBackgroundColor_LL();
		std::cout << " **  Lower Left:   ";
		PrintColors(t);
		t = icon_sys->GetBackgroundColor_LR();
		std::cout << " **  Lower Right:  ";
		PrintColors(t);
	}
	{
		IconSys::IconSys_LightVec t(0.0f, 0.0f, 0.0f, 0.0f);
		std::cout << " ** Light Sources  (X, Y, Z, W)" << std::endl;
		t = icon_sys->GetLight1Dir();
		std::cout << " **  #1: " << std::setw(6) << t.GetX() << ", " << std::setw(6) << t.GetY() 
			<< ", " << std::setw(6) << t.GetZ() << ", " << std::setw(6) << t.GetW() << std::endl;
		t = icon_sys->GetLight2Dir();
		std::cout << " **  #2: " << std::setw(6) << t.GetX() << ", " << std::setw(6) << t.GetY() 
			<< ", " << std::setw(6) << t.GetZ() << ", " << std::setw(6) << t.GetW() << std::endl;
		t = icon_sys->GetLight3Dir();
		std::cout << " **  #3: " << std::setw(6) << t.GetX() << ", " << std::setw(6) << t.GetY() 
			<< ", " << std::setw(6) << t.GetZ() << ", " << std::setw(6) << t.GetW() << std::endl;
	}
	{
		IconSys::IconSys_LightColor t(0.0f, 0.0f, 0.0f, 0.0f);
		std::cout << " ** Light Colors  (R, G, B, X)" << std::endl;
		t = icon_sys->GetLight1Color();
		std::cout << " ** #1: ";
		PrintColors(t);
		t = icon_sys->GetLight2Color();
		std::cout << " ** #2: ";
		PrintColors(t);
		t = icon_sys->GetLight3Color();
		std::cout << " ** #3: ";
		PrintColors(t);
		t = icon_sys->GetLightAmbientColor();
		std::cout << " ** Ambient: ";
		PrintColors(t);
	}
}

void WriteOutput(IconSys* icon_sys) 
{
	if(verbose_output)
		std::cout << " * Writing output file to \"" << output_file << "\"...";
	try {
		icon_sys->WriteFile(output_file);
	} catch(Ghulbus::gbException e) {
		std::cout << "\nFile write error: \"" << output_file << "\"" << std::endl;
	}
	if(verbose_output)
		std::cout << "done." << std::endl;
}

void Cleanup()
{
	if(title_string)       { delete[] title_string;              title_string = NULL; }
	if(icon_string)        { delete[] icon_string;                icon_string = NULL; }
	if(icon_copy_string)   { delete[] icon_copy_string;      icon_copy_string = NULL; }
	if(icon_delete_string) { delete[] icon_delete_string;  icon_delete_string = NULL; }
	if(light1_dir)         { delete[] light1_dir;                  light1_dir = NULL; }
	if(light2_dir)         { delete[] light2_dir;                  light2_dir = NULL; }
	if(light3_dir)         { delete[] light3_dir;                  light3_dir = NULL; }
	if(light1_color)       { delete[] light1_color;              light1_color = NULL; }
	if(light2_color)       { delete[] light2_color;              light2_color = NULL; }
	if(light3_color)       { delete[] light3_color;              light3_color = NULL; }
	if(ambient_color)      { delete[] ambient_color;            ambient_color = NULL; }
	if(bg_color_ul)        { delete[] bg_color_ul;                bg_color_ul = NULL; }
	if(bg_color_ur)        { delete[] bg_color_ur;                bg_color_ur = NULL; }
	if(bg_color_ll)        { delete[] bg_color_ll;                bg_color_ll = NULL; }
	if(bg_color_lr)        { delete[] bg_color_lr;                bg_color_lr = NULL; }
}

int main(int argc, char* argv[])
{
	ParseCommandLine(argc, argv);

	if(argc < 2) {
		std::cout << "No parameters specified." << std::endl << std::endl;
		PrintHelp(argv[0]);
		exit(1);
	}

	CheckParameters();

	std::cout << "PS2 Icon.Sys Builder  V-1.0\n by Ghulbus Inc.  (http://www.ghulbus-inc.de/)\n" << std::endl;

	if(!output_file) {
		output_file = "icon.sys";
	}

	IconSys* icon_sys = NULL;

	if(input_file) {
		if(verbose_output)
			std::cout << " * Reading input file \"" << input_file << "\"...";
		try {
			icon_sys = new IconSys(input_file);
		} catch(Ghulbus::gbException e) {
			std::cout << "\nFile read error: \"" << input_file << "\"" << std::endl;
		}
		if(verbose_output)
			std::cout << "done." << std::endl;
	} else {
		icon_sys = new IconSys();
	}

	ProcessParameters(icon_sys);

	WriteOutput(icon_sys);

	if(list_file) {
		ListFile(icon_sys);
	}

	Cleanup();	
	delete icon_sys;

	std::cout << "Success :)" << std::endl;

	return 0;
}
