#include "wavefront_file.h"

// std includes
#include <fstream>
#include <regex>
#include <sstream>

// mirage includes
#include "../../config.h"
#include "../../macros.h"
#include "../../util/strutil.h"

namespace mirage
{

	// ---------------------------------------------------------------------------
	// WavefrontFile
	// ---------------------------------------------------------------------------

	WavefrontFile::WavefrontFile(const std::string & filePath) :
		m_objFilePath(filePath),
		m_mtlFilePath("NULL"),
		m_points(),
		m_normals(),
		m_texcoords(),
		m_meshes(),
		m_materials()
	{
		try
		{
			loadObj(m_objFilePath);
		}
		catch (const std::exception)
		{
			MERR("WavefrontFile::WavefrontFile - Error loading WavefrontFile!"
				<< " .obj FilePath: " << m_objFilePath << ", .mtl FilePath: " << m_mtlFilePath);
		}
	}


	void WavefrontFile::loadObj(const std::string & filePath)
	{
		// Read the file into memory
		std::ifstream file(filePath);

		// Throw if the file didn't open
		if (file.is_open() == false)
			throw std::exception();

		// State variables
		std::string currentMesh = "root";
		std::string currentMaterial = "root";
		glm::vec3 current_v3;
		glm::vec2 current_v2;

		// Intialize currentMesh & currentMaterial
		m_meshes[currentMesh] = WavefrontMesh();
		m_materials[currentMaterial] = WavefrontMaterial();

		// Iterate through every line
		std::string l;
		while (std::getline(file, l))
		{
			// Trim any leading/trailing/extra ws
			l.erase(std::unique(l.begin(), l.end(), istwospace), l.end());

			// Line sstream
			std::istringstream l_stream(l);

			// Get the type of the current line
			std::string type;
			l_stream >> type;

			switch (cstr2int(type.c_str()))
			{
			case cstr2int("mtllib"):
				l_stream >> m_mtlFilePath;
				loadMtl(m_mtlFilePath);
				break;
			case cstr2int("v"):
				l_stream >> current_v3.x >> current_v3.y >> current_v3.z;
				m_points.push_back(current_v3);
				break;
			case cstr2int("vn"):
				l_stream >> current_v3.x >> current_v3.y >> current_v3.z;
				m_normals.push_back(current_v3);
				break;
			case cstr2int("vt"):
				l_stream >> current_v2.x >> current_v2.y;
				m_texcoords.push_back(current_v2);
				break;
			case cstr2int("o"):
			case cstr2int("g"):
				l_stream >> currentMesh;
				m_meshes[currentMesh] = WavefrontMesh();
				break;
			case cstr2int("usemtl"):
			{
				// Switch to the new material
				l_stream >> currentMaterial;

				// Get the current mesh that is being processed
				// and split the mesh into two if it had faces, this handles multiple materials + non-consistent mesh objects
				WavefrontMesh & mesh = m_meshes[currentMesh];
				if (mesh.faces.empty() == false)
				{
					currentMesh = currentMesh + currentMaterial;
					m_meshes[currentMesh] = WavefrontMesh();
				}
			}
			break;
			case cstr2int("f"):
			{
				// Initially create the face with current material
				WavefrontFace face(currentMaterial);

				// Determine face format, this is v1/t1/n1
				if (l.find("//") == std::string::npos)
				{
					// Count the number of values on this line
					int n = std::count_if(l.begin(), l.end(), [](unsigned char c) { return c == '/'; });

					// Replace slashes with whitespace & create sstream
					std::replace(l.begin(), l.end(), '/', ' ');
					std::istringstream lf(l.substr(2));

					// v1/t1/n1
					if (n == 6)
					{
						lf >> face.points[0];
						lf >> face.texcoords[0];
						lf >> face.normals[0];

						lf >> face.points[1];
						lf >> face.texcoords[1];
						lf >> face.normals[1];

						lf >> face.points[2];
						lf >> face.texcoords[2];
						lf >> face.normals[2];
					}
					// v1/t1
					else if (n == 3)
					{
						lf >> face.points[0];
						lf >> face.texcoords[0];

						lf >> face.points[1];
						lf >> face.texcoords[1];

						lf >> face.points[2];
						lf >> face.texcoords[2];
					}
				}
				// This is v1//n1
				else
				{

				}

				// Get the pointer to current mesh in map
				WavefrontMesh * mesh = &m_meshes[currentMesh];

				// Create the mesh if it doesn't exist
				if (mesh == nullptr)
					m_meshes[currentMesh] = WavefrontMesh();

				// Insert the face
				m_meshes[currentMesh].faces.push_back(face);
			}
			break;

			}

		}

	}

	void WavefrontFile::loadMtl(const std::string & filePath)
	{
		MLOG(filePath);
	}

}