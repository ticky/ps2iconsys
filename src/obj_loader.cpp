/**
 * @file src/obj_loader.cpp
 *
 * @brief Implementation of OBJ_FileLoader and OBJ_Mesh
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
#include "../include/obj_loader.hpp"
#include <cstdio>
#include <string>

OBJ_FileLoader::OBJ_FileLoader()
{
	;
}

OBJ_FileLoader::OBJ_FileLoader(const char* fname)
{
	std::ifstream fin(fname, std::ios_base::in);
	if(fin.fail()) { throw( Ghulbus::gbException(Ghulbus::gbException::GB_FAILED,
	                                             "Could not open obj file for read") ); }
	ReadFile(fin);
}
OBJ_FileLoader::~OBJ_FileLoader()
{
	for(std::vector<OBJ_Mesh*>::iterator i = m_MeshList.begin(); i != m_MeshList.end(); ++i) {
		delete *i;
	}
	m_MeshList.clear();
}

int OBJ_FileLoader::GetNMeshes() const {
	return static_cast<int>(m_MeshList.size());
}
OBJ_Mesh const* OBJ_FileLoader::GetMesh(int index) const {
	return m_MeshList[index];
}

void OBJ_FileLoader::AddMesh(OBJ_Mesh const& mesh) {
	OBJ_Mesh* newmesh = new OBJ_Mesh(mesh);
	m_MeshList.push_back(newmesh);
}

void OBJ_FileLoader::WriteFile(char const* fname) const {
	std::ofstream fout(fname, std::ios_base::out);
	if( fout.fail() ) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_FAILED,
			                         "Output OBJ file could not be opened" ) );
	}
	//again we need counters since obj doesn't reset indices between meshes:
	int vert_counter=0, normal_counter=0, texture_counter=0;
	int vert_base=0, normal_base=0, texture_base=0;
	int current_smooth_group = 0;
	
	fout.precision(6);
	fout << std::fixed;
	//file header:
	fout << "# OBJ File created by PS2IconSys Viewer" << std::endl
		 << "#  http://www.ghulbus-inc.de/" << std::endl
		 << "#" << std::endl;

	for(int i=0; i<GetNMeshes(); i++) {
		OBJ_Mesh const* mesh = GetMesh(i);
		//object header:
		fout << "# object " << mesh->GetName() << " to come" << std::endl
			 << "#" << std::endl;
		
		//vertex data:
		for(int j=0; j<mesh->GetNVertices(); j++) {
			vert_counter++;
			fout << "v  " << *(mesh->GetVertexX(j)) << " " << *(mesh->GetVertexY(j)) << " "
				<< *(mesh->GetVertexZ(j)) << std::endl;
		}
		fout << "# " << mesh->GetNVertices() << " vertices" << std::endl << std::endl;
		//texture vertex data:
		for(int j=0; j<mesh->GetNTexture(); j++) {
			texture_counter++;
			fout << "vt  " << *(mesh->GetTextureX(j)) << " " << *(mesh->GetTextureY(j)) << " "
				<< *(mesh->GetTextureZ(j)) << std::endl;
		}
		fout << "# " << mesh->GetNTexture() << " texture vertices" << std::endl << std::endl;
		//vertex normal data:
		for(int j=0; j<mesh->GetNNormals(); j++) {
			normal_counter++;
			fout << "vn  " << *(mesh->GetNormalX(j)) << " " << *(mesh->GetNormalY(j)) << " "
				<< *(mesh->GetNormalZ(j)) << std::endl;
		}
		fout << "# " << mesh->GetNNormals() << " vertex normals" << std::endl << std::endl;

		//face list:
		fout << "g " << mesh->GetName() << std::endl;
		current_smooth_group = mesh->GetFace(0)->smoothing_group;
		fout << "s " << current_smooth_group << std::endl;
		for(int j=0; j<mesh->GetNFaces(); j++) {
			OBJ_Mesh::Face const* current_face = mesh->GetFace(j);
			if(current_face->smoothing_group != current_smooth_group) {
				current_smooth_group = current_face->smoothing_group;
				fout << "s " << current_smooth_group << std::endl;
			}
			fout << "f " << (current_face->vert1 + vert_base + 1) << "/" 
				<< (current_face->texture1 + texture_base + 1) << "/" 
				<< (current_face->normal1 + normal_base + 1)   << " " 
				<< (current_face->vert2 + vert_base + 1)       << "/" 
				<< (current_face->texture2 + texture_base + 1) << "/" 
				<< (current_face->normal2 + normal_base + 1)   << " " 
				<< (current_face->vert3 + vert_base + 1)       << "/" 
				<< (current_face->texture3 + texture_base + 1) << "/" 
				<< (current_face->normal3 + normal_base + 1)   << std::endl;
		}
		fout << "# " << mesh->GetNFaces() << " faces" << std::endl << std::endl;

		//adjust base counters:
		vert_base = vert_counter;
		normal_base = normal_counter;
		texture_base = texture_counter;
		fout << "g" << std::endl;
	}
}

void OBJ_FileLoader::ReadFile(std::ifstream& f) 
{
	///@todo: only triangular polys supported; all vertices are assumed to have vertex, texture and normal data present;
	std::string buffer;
	if(m_MeshList.size() > 0) { 
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_INVALIDCONTEXT,
			                         "The mesh list is not empty" ) );
	}
	char group_name[256];								///< buffer holding the current group name
	bool new_group_was_opened = false;					///< flag for mesh object maintenance
	OBJ_Mesh* mesh = new OBJ_Mesh("");					///< buffer mesh object
	double tmp[3];
	OBJ_Mesh::Face tmp_face;
	tmp_face.smoothing_group = -1;
	int vert_counter=0, normal_counter=0, texture_counter=0;
	int vert_base=0, normal_base=0, texture_base=0;
	
	while( std::getline(f, buffer) ) {
		//for each line of the file do:
		//first character determines what kind of data to expect:
		switch(buffer[0]) {
			case '#':
				//commentary
				// do nothing
				break;
			case 'v':
				//vertex data
				if(new_group_was_opened) {
					//if new group was opened since last vertex was read
					//the following data belongs to a new mesh
					OBJ_Mesh* newmesh = new OBJ_Mesh("");
					mesh->SetName(group_name);
					m_MeshList.push_back(mesh);
					mesh = newmesh;
					vert_base = vert_counter;			//save current indices
					normal_base = normal_counter;
					texture_base = texture_counter;
					new_group_was_opened = false;
				}
				//read and store vertex data:
				switch(buffer[1]) {
					case ' ':
						//geometry vertex
						sscanf(buffer.c_str(), "v %lf %lf %lf", &(tmp[0]), &(tmp[1]), &(tmp[2]));
						mesh->AddGeometry(tmp, 3);
						vert_counter++;
						break;
					case 't':
						//texture vertex
						sscanf(buffer.c_str(), "vt %lf %lf %lf", &(tmp[0]), &(tmp[1]), &(tmp[2]));
						mesh->AddTextureData(tmp, 3);
						texture_counter++;
						break;
					case 'n':
						//normal vertex
						sscanf(buffer.c_str(), "vn %lf %lf %lf", &(tmp[0]), &(tmp[1]), &(tmp[2]));
						mesh->AddNormals(tmp, 3);
						normal_counter++;
						break;
					case 'p':
						//parameter space vertex
						/// @todo
						break;
					default:
						break;
				}
				break;
			case 'f':
				//face
				//read and store face data:
				sscanf(buffer.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
					    &(tmp_face.vert1), &(tmp_face.texture1), &(tmp_face.normal1),
						&(tmp_face.vert2), &(tmp_face.texture2), &(tmp_face.normal2),
						&(tmp_face.vert3), &(tmp_face.texture3), &(tmp_face.normal3) );
				//since obj's index counters are not reset between groups, we need to adjust indices manually:
				tmp_face.vert1 -= vert_base; tmp_face.vert2 -= vert_base; tmp_face.vert3 -= vert_base;
				tmp_face.normal1 -= normal_base; tmp_face.normal2 -= normal_base; tmp_face.normal3 -= normal_base; 
				tmp_face.texture1 -= texture_base; tmp_face.texture2 -= texture_base; tmp_face.texture3 -= texture_base;
				//beware! object file indices are 1-based; readjust:
				tmp_face.vert1--;    tmp_face.vert2--;    tmp_face.vert3--;
				tmp_face.texture1--; tmp_face.texture2--; tmp_face.texture3--;
				tmp_face.normal1--;  tmp_face.normal2--;  tmp_face.normal3--;
				mesh->AddFaceData(&tmp_face, 1);
				break;
			case 'g':
				//group
				if(sscanf(buffer.c_str(), "g %s", group_name) > 0) {
					//a new (named) group was opened;
					//in 3dsmax obj files this should only happen before a new faces block
					new_group_was_opened = true;
				}
				break;
			case 's':
				switch(buffer[1]) {
					case ' ':
						//smoothing group
						sscanf(buffer.c_str(), "s %d", &(tmp_face.smoothing_group));
						break;
					default:
						break;
				}
			default:
				break;
		}
	}
	if(mesh->GetNFaces() > 0) {
		mesh->SetName(group_name);
		m_MeshList.push_back(mesh);
	} else {
		delete mesh;
	}
}

OBJ_Mesh::OBJ_Mesh(char const* name)
	:m_name(NULL)
{
	SetName(name);
}
OBJ_Mesh::~OBJ_Mesh() {
	if(m_name) { delete[] m_name;  m_name = NULL; }
}
OBJ_Mesh::OBJ_Mesh(OBJ_Mesh const& rhs)
	:m_name(NULL)
{
	//copy construct:
	this->SetName(rhs.m_name);
	this->SetGeometry(rhs.m_geometry);
	this->SetNormals(rhs.m_normals);
	this->SetTextureData(rhs.m_texcoords);
	this->SetFaceData(rhs.m_faces);
}

void OBJ_Mesh::SetName(char const* name) {
	if(m_name) { delete[] m_name;  m_name = NULL; }
	size_t size = strlen(name) + 1;
	m_name = new char[size];
	strcpy(m_name, name);
}

/** Helper function for the Set*vector methods
 */
template<typename T>
inline void CopyVector(std::vector<T>* dest, std::vector<T> const* src) {
	dest->clear();
	for(typename std::vector<T>::const_iterator i = src->begin(); i != src->end(); ++i) {
		dest->push_back(*i);
	}
}

void OBJ_Mesh::SetGeometry(std::vector<double> const& data) {
	CopyVector(&m_geometry, &data);
}
void OBJ_Mesh::SetNormals(std::vector<double> const& data) {
	CopyVector(&m_normals, &data);
}
void OBJ_Mesh::SetTextureData(std::vector<double> const& data) {
	CopyVector(&m_texcoords, &data);
}
void OBJ_Mesh::SetFaceData(std::vector<Face> const& data) {
	CopyVector(&m_faces, &data);
}
void OBJ_Mesh::SetFaceData(Face const* data, int n_data) {
	FillVector(&m_faces, data, n_data);
}
void OBJ_Mesh::AddFaceData(Face const* data, int n_data) {
	AppendToVector(&m_faces, data, n_data);
}
void OBJ_Mesh::ClearGeometry() {
	m_geometry.clear();
}
void OBJ_Mesh::ClearNormals() {
	m_normals.clear();
}
void OBJ_Mesh::ClearTextureData() {
	m_texcoords.clear();
}
void OBJ_Mesh::ClearFaceData() {
	m_faces.clear();
}
char const* OBJ_Mesh::GetName() const {
	return (m_name?m_name:"");
}
int OBJ_Mesh::GetNVertices() const {
	return static_cast<int>(m_geometry.size() / 3);
}
int OBJ_Mesh::GetNFaces() const {
	return static_cast<int>(m_faces.size());
}
int OBJ_Mesh::GetNNormals() const {
	return static_cast<int>(m_normals.size() / 3);
}
int OBJ_Mesh::GetNTexture() const {
	return static_cast<int>(m_texcoords.size() / 3);
}
double const* OBJ_Mesh::GetVertexX(int index) const {
	if(index >= static_cast<int>(m_geometry.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_geometry[index*3]);
}
double const* OBJ_Mesh::GetVertexY(int index) const {
	if(index >= static_cast<int>(m_geometry.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_geometry[index*3 + 1]);
}
double const* OBJ_Mesh::GetVertexZ(int index) const {
	if(index >= static_cast<int>(m_geometry.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_geometry[index*3 + 2]);
}
double const* OBJ_Mesh::GetNormalX(int index) const {
	if(index >= static_cast<int>(m_normals.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_normals[index*3]);
}
double const* OBJ_Mesh::GetNormalY(int index) const {
	if(index >= static_cast<int>(m_normals.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_normals[index*3 + 1]);
}
double const* OBJ_Mesh::GetNormalZ(int index) const {
	if(index >= static_cast<int>(m_normals.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_normals[index*3 + 2]);
}
double const* OBJ_Mesh::GetTextureX(int index) const {
	if(index >= static_cast<int>(m_texcoords.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_texcoords[index*3]);
}
double const* OBJ_Mesh::GetTextureY(int index) const {
	if(index >= static_cast<int>(m_texcoords.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_texcoords[index*3 + 1]);
}
double const* OBJ_Mesh::GetTextureZ(int index) const {
	if(index >= static_cast<int>(m_texcoords.size() / 3)) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_texcoords[index*3 + 2]);
}
OBJ_Mesh::Face const* OBJ_Mesh::GetFace(int index) const {
	if(index >= static_cast<int>(m_faces.size()) ) {
		throw( Ghulbus::gbException( Ghulbus::gbException::GB_ILLEGALPARAMETER ) );
	}
	return &(m_faces[index]);
}
