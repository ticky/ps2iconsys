/**
 * @file src/obj_loader.impl.hpp
 *
 * @brief Template members of OBJ_FileLoader and OBJ_Mesh
 *
 * @version 1.0
 * @author Andreas Weis
 *
 */
template<typename T>
void OBJ_Mesh::GetMeshGeometryUnindexed(T* mesh_geometry, T* mesh_normals, T* mesh_texture, T scale) const {
	int index = 0;		//index of the current triangle
	for(std::vector<Face>::const_iterator iter = m_faces.begin(); iter != m_faces.end(); ++iter, ++index) {
		if(mesh_geometry) {
			mesh_geometry[(index*9)]     = static_cast<T>( m_geometry[ (*iter).vert1 * 3 ])    * scale;
			mesh_geometry[(index*9) + 1] = static_cast<T>( m_geometry[ (*iter).vert1 * 3 + 1]) * scale;
			mesh_geometry[(index*9) + 2] = static_cast<T>( m_geometry[ (*iter).vert1 * 3 + 2]) * scale;
			mesh_geometry[(index*9) + 3] = static_cast<T>( m_geometry[ (*iter).vert2 * 3 ])    * scale;
			mesh_geometry[(index*9) + 4] = static_cast<T>( m_geometry[ (*iter).vert2 * 3 + 1]) * scale;
			mesh_geometry[(index*9) + 5] = static_cast<T>( m_geometry[ (*iter).vert2 * 3 + 2]) * scale;
			mesh_geometry[(index*9) + 6] = static_cast<T>( m_geometry[ (*iter).vert3 * 3 ])    * scale;
			mesh_geometry[(index*9) + 7] = static_cast<T>( m_geometry[ (*iter).vert3 * 3 + 1]) * scale;
			mesh_geometry[(index*9) + 8] = static_cast<T>( m_geometry[ (*iter).vert3 * 3 + 2]) * scale;
		}
		if(mesh_normals) {
			mesh_normals[(index*9)]      = static_cast<T>( m_normals[ (*iter).normal1 * 3 ] );
			mesh_normals[(index*9) + 1]  = static_cast<T>( m_normals[ (*iter).normal1 * 3 + 1] );
			mesh_normals[(index*9) + 2]  = static_cast<T>( m_normals[ (*iter).normal1 * 3 + 2] );
			mesh_normals[(index*9) + 3]  = static_cast<T>( m_normals[ (*iter).normal2 * 3] );
			mesh_normals[(index*9) + 4]  = static_cast<T>( m_normals[ (*iter).normal2 * 3 + 1] );
			mesh_normals[(index*9) + 5]  = static_cast<T>( m_normals[ (*iter).normal2 * 3 + 2] );
			mesh_normals[(index*9) + 6]  = static_cast<T>( m_normals[ (*iter).normal3 * 3] );
			mesh_normals[(index*9) + 7]  = static_cast<T>( m_normals[ (*iter).normal3 * 3 + 1] );
			mesh_normals[(index*9) + 8]  = static_cast<T>( m_normals[ (*iter).normal3 * 3 + 2] );
		}
		if(mesh_texture) {
			mesh_texture[(index*9)]      = static_cast<T>( m_texcoords[ (*iter).texture1 * 3 ] );
			mesh_texture[(index*9) + 1]  = static_cast<T>( m_texcoords[ (*iter).texture1 * 3 + 1] );
			mesh_texture[(index*9) + 2]  = static_cast<T>( m_texcoords[ (*iter).texture1 * 3 + 2] );
			mesh_texture[(index*9) + 3]  = static_cast<T>( m_texcoords[ (*iter).texture2 * 3] );
			mesh_texture[(index*9) + 4]  = static_cast<T>( m_texcoords[ (*iter).texture2 * 3 + 1] );
			mesh_texture[(index*9) + 5]  = static_cast<T>( m_texcoords[ (*iter).texture2 * 3 + 2] );
			mesh_texture[(index*9) + 6]  = static_cast<T>( m_texcoords[ (*iter).texture3 * 3] );
			mesh_texture[(index*9) + 7]  = static_cast<T>( m_texcoords[ (*iter).texture3 * 3 + 1] );
			mesh_texture[(index*9) + 8]  = static_cast<T>( m_texcoords[ (*iter).texture3 * 3 + 2] );
		}
	}
}

template<typename T>
void OBJ_Mesh::GetMeshGeometry(T* mesh_geometry, T* mesh_normals, T* mesh_texture, OBJ_Mesh::Face* mesh_faces, T scale) const {
	int i = 0;
	if(mesh_geometry) {
		for(std::vector<double>::const_iterator iter = m_geometry.begin(); iter != m_geometry.end(); ++iter, ++i) {
			mesh_geometry[i] = static_cast<T>( (*iter) * scale );
		}
	}
	if(mesh_normals) {
		for(std::vector<double>::const_iterator iter = m_normals.begin(); iter != m_normals.end(); ++iter, ++i) {
			mesh_normals[i] = static_cast<T>( *iter );
		}
	}
	if(mesh_texture) {
		for(std::vector<double>::const_iterator iter = m_texcoords.begin(); iter != m_texcoords.end(); ++iter, ++i) {
			mesh_texture[i] = static_cast<T>( *iter );
		}
	}
	if(mesh_faces) {
		for(std::vector<Face>::const_iterator iter = m_faces.begin(); iter != m_faces.end(); ++iter, ++i) {
			mesh_faces[i].vert1           = (*iter).vert1;
			mesh_faces[i].vert2           = (*iter).vert2;
			mesh_faces[i].vert3           = (*iter).vert3;
			mesh_faces[i].normal1         = (*iter).normal1;
			mesh_faces[i].normal2         = (*iter).normal2;
			mesh_faces[i].normal3         = (*iter).normal3;
			mesh_faces[i].texture1        = (*iter).texture1;
			mesh_faces[i].texture2        = (*iter).texture2;
			mesh_faces[i].texture3        = (*iter).texture3;
			mesh_faces[i].smoothing_group = (*iter).smoothing_group;
		}
	}
}
/** Helper function for the Set*field methods
 */
template<typename T_1, typename T_2>
inline void FillVector(std::vector<T_1>* dest, T_2 const* data, int n_data) {
	dest->clear();
	for(int i=0; i<n_data; i++) {
		dest->push_back( static_cast<T_1>(data[i]) );
	}
}
/** Helper function for the Add*field methods
 */
template<typename T_1, typename T_2>
inline void AppendToVector(std::vector<T_1>* dest, T_2 const* data, int n_data) {
	for(int i=0; i<n_data; i++) {
		dest->push_back( static_cast<T_1>(data[i]) );
	}
}

template<typename T>
void OBJ_Mesh::SetGeometry(T const* data, int n_data) {
	FillVector(&m_geometry, data, n_data);
}
template<typename T>
void OBJ_Mesh::AddGeometry(T const* data, int n_data) {
	AppendToVector(&m_geometry, data, n_data);
}
template<typename T>
void OBJ_Mesh::SetNormals(T const* data, int n_data) {
	FillVector(&m_normals, data, n_data);
}
template<typename T>
void OBJ_Mesh::AddNormals(T const* data, int n_data) {
	AppendToVector(&m_normals, data, n_data);
}
template<typename T>
void OBJ_Mesh::SetTextureData(T const* data, int n_data) {
	FillVector(&m_texcoords, data, n_data);
}
template<typename T>
void OBJ_Mesh::AddTextureData(T const* data, int n_data) {
	AppendToVector(&m_texcoords, data, n_data);
}
