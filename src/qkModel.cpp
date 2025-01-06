#define CGLTF_IMPLEMENTATION
#include "quack/qkModel.h"

#include <cgltf.h>
#include <stdexcept>

qkModel::qkModel(const char* filename)
{
	cgltf_options options = {};
	cgltf_data*	  data	  = NULL;
	cgltf_result  result  = cgltf_parse_file(&options, filename, &data);

	if (result != cgltf_result_success)
	{
		throw std::runtime_error("Failed to load GLTF file");
	}

	result = cgltf_load_buffers(&options, data, filename);
	if (result != cgltf_result_success)
	{
		cgltf_free(data);
		throw std::runtime_error("Failed to load GLTF buffers");
	}

	for (size_t i = 0; i < data->meshes_count; ++i)
	{
		cgltf_mesh* mesh = &data->meshes[i];
		for (size_t j = 0; j < mesh->primitives_count; ++j)
		{
			cgltf_primitive* primitive = &mesh->primitives[j];

			for (size_t k = 0; k < primitive->attributes_count; ++k)
			{
				cgltf_attribute* attr = &primitive->attributes[k];
				if (attr->type == cgltf_attribute_type_position)
				{
					cgltf_accessor* accessor = attr->data;
					for (size_t l = 0; l < accessor->count; ++l)
					{
						cgltf_float position[3];
						cgltf_accessor_read_float(accessor, l, position, 3);
						vertices.push_back({{position[0], position[1], position[2]}});
					}
				}
				else if (attr->type == cgltf_attribute_type_texcoord)
				{
					cgltf_accessor* accessor	 = attr->data;
					size_t			vertexOffset = vertices.size() - accessor->count;

					for (size_t l = 0; l < accessor->count; ++l)
					{
						cgltf_float texcoord[2];
						cgltf_accessor_read_float(accessor, l, texcoord, 2);
						vertices[vertexOffset + l].texU = texcoord[0];
						vertices[vertexOffset + l].texV = texcoord[1];
					}
				}
			}

			if (primitive->indices)
			{
				cgltf_accessor* accessor = primitive->indices;
				for (size_t k = 0; k < accessor->count; k += 3)
				{
					cgltf_uint v1, v2, v3;
					cgltf_accessor_read_uint(accessor, k, &v1, 1);
					cgltf_accessor_read_uint(accessor, k + 1, &v2, 1);
					cgltf_accessor_read_uint(accessor, k + 2, &v3, 1);
					triangles.push_back({(int)v1, (int)v2, (int)v3});
				}
			}
			else
			{
				// No indices - assume vertices are arranged in triangles
				for (size_t k = 0; k < primitive->attributes[0].data->count; k += 3)
				{
					triangles.push_back({(int)k, (int)(k + 1), (int)(k + 2)});
				}
			}
		}
	}

	m_gltfData = data;
}

qkModel::~qkModel()
{
	if (m_gltfData)
	{
		cgltf_free((cgltf_data*)m_gltfData);
	}
}