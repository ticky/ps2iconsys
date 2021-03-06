# PS2 IconSys

## What is it?
The PS2 IconSys are a number of tools that allow the creation and manipulation of icons and ICON.SYS files for Sony Playstation 2 Memory Cards.

They can be used either as command line tools for the system shell or as libraries for C++ development.

## How does it work
The PS2 browser expects at least two file in each subdirectory of a memory card. First, a file named icon.sys which contains basic information about the data in that directory, like the description string displayed in the browser, as well as the name of at least one Icon file in that directory. This Icon will then be displayed in the browser. If either of these two files is missing, the browser will display the directory as a blue box labeled "corrupted data". With the enclosed tools, one can easily create both icon.sys and Icon files that are needed for proper browser display.

## Installation
For detailed compilation instructions, refer to the COMPILE document in the sources archive.

## Contents
As of the current version, the PS2 IconSys toolset consists of three command line tools:

* `build_iconsys` - Allows the creation and manipulation of Icon.sys files. Those files act as content descriptors for the Playstation 2 internal browser and must be present in each subdirectory of a PS2 memory card.
* `obj_to_ps2icon` - Allows the conversion of a PS2 Icon into a Wavefront OBJ file for model geometry, and TGA file for 2d texture data.
* `ps2icon_to_obj` - Allows to create PS2 Icons of arbitrary geometry from Wavefront OBJ files and TGA/BMP texture files.

A detailed description for each of the tools can be viewed by calling the respective tool with the `-h` parameter.

## FAQ & Known Issues

**What is Wavefront OBJ?**  
Wavefront OBJ is a well known human-readable format for 3d graphics interchange. It is known by most 3d-applications out there, including 3ds max and Blender.

**I noticed that the converted OBJ files have lots of doubled vertices. Is that intentional?**  
This issue comes from the fact of how PS2 Icons store their geometry. It should however not provide any problem and can easily be fixed with a 3d editor of your choice.

**When I open a converted OBJ in a 3d editor the model has some holes in it.**  
Some 3d editors tend to flip faces of imported OBJ-meshes for no apparent reason. This can usually be fixed inside the 3d application with no greater effort. Consult the manual of your 3d application for further details.

**The parameter list of the build_iconsys tool makes my head feel funny.**  
If you'd prefer a GUI, look for MC_ICON.SYS_Generator at http://ps2dev.org/ps2/Tools.  
  If I get bored, I might add Lua scripting support in a future version to get rid of the lengthy parameter lists.

**What about animated Icons?**  
In a future version. If you're really impatient, feel free to play around with the source, it should contain everything you need.

**The PS2 doesn't read my Icon!**  
This is probably due to the polygon limit. Try keeping your mesh under 1500 triangles to ensure compatibility.

## License
This software is provided under the MIT license. See the enclosed LICENSE file for further details.

## Contact
URL: http://www.ghulbus-inc.de/  
Mail: der_ghulbus@ghulbus-inc.de
