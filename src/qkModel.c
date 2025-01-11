#define CGLTF_IMPLEMENTATION
#include "quack/qkModel.h"

#include <cgltf.h>
#include <stdlib.h>
#include <string.h>

int qkModelLoad(const char* filename, qkModel* out)
{
	cgltf_options options = {0};
	cgltf_data*	  data	  = NULL;
	cgltf_result  result  = cgltf_parse_file(&options, filename, &data);

	if (result != cgltf_result_success)
	{
		return -1;	// -1: Failed to parse GLTF file
	}

	result = cgltf_load_buffers(&options, data, filename);
	if (result != cgltf_result_success)
	{
		cgltf_free(data);
		return -2;	// Failed to load GLTF buffers
	}

	size_t totalVertices  = 0;
	size_t totalTriangles = 0;

	for (size_t i = 0; i < data->meshes_count; i++)
	{
		cgltf_mesh* mesh = &data->meshes[i];
		for (size_t j = 0; j < mesh->primitives_count; j++)
		{
			cgltf_primitive* primitive = &mesh->primitives[j];
			for (size_t k = 0; k < primitive->attributes_count; k++)
			{
				cgltf_attribute* attr = &primitive->attributes[k];
				if (attr->type == cgltf_attribute_type_position)
				{
					totalVertices += attr->data->count;
				}
			}
			if (primitive->indices)
			{
				totalTriangles += primitive->indices->count / 3;
			}
			else
			{
				totalTriangles += primitive->attributes[0].data->count / 3;
			}
		}
	}

	// Allocate memory
	out->pVertices = malloc(totalVertices * sizeof(qkVertex));
	if (!out->pVertices)
	{
		cgltf_free(data);
		return -3;	// -3: Failed to allocate vertices
	}

	out->pTriangles = malloc(totalTriangles * sizeof(qkTriangle));
	if (!out->pTriangles)
	{
		free(out->pVertices);
		cgltf_free(data);
		return -4;	// Failed to allocate triangles
	}

	out->vertexCount   = totalVertices;
	out->triangleCount = totalTriangles;
	out->pGltfData	   = data;

	size_t vertexOffset	  = 0;
	size_t triangleOffset = 0;

	for (size_t i = 0; i < data->meshes_count; i++)
	{
		cgltf_mesh* mesh = &data->meshes[i];
		for (size_t j = 0; j < mesh->primitives_count; j++)
		{
			cgltf_primitive* primitive			  = &mesh->primitives[j];
			size_t			 primitiveVertexCount = 0;

			for (size_t k = 0; k < primitive->attributes_count; k++)
			{
				cgltf_attribute* attr = &primitive->attributes[k];
				if (attr->type == cgltf_attribute_type_position)
				{
					cgltf_accessor* accessor = attr->data;
					primitiveVertexCount	 = accessor->count;
					for (size_t l = 0; l < accessor->count; l++)
					{
						cgltf_float pos[3];
						cgltf_accessor_read_float(accessor, l, pos, 3);
						out->pVertices[vertexOffset + l].position = qkVec3Create(pos[0], pos[1], pos[2]);
					}
				}
			}

			for (size_t k = 0; k < primitive->attributes_count; k++)
			{
				cgltf_attribute* attr = &primitive->attributes[k];
				if (attr->type == cgltf_attribute_type_texcoord)
				{
					cgltf_accessor* accessor = attr->data;
					for (size_t l = 0; l < accessor->count; l++)
					{
						cgltf_float texcoord[2];
						cgltf_accessor_read_float(accessor, l, texcoord, 2);
						out->pVertices[vertexOffset + l].texU = texcoord[0];
						out->pVertices[vertexOffset + l].texV = texcoord[1];
					}
				}
			}

			if (primitive->indices)
			{
				cgltf_accessor* accessor = primitive->indices;
				for (size_t k = 0; k < accessor->count; k += 3)
				{
					cgltf_uint indices[3];
					cgltf_accessor_read_uint(accessor, k, &indices[0], 1);
					cgltf_accessor_read_uint(accessor, k + 1, &indices[1], 1);
					cgltf_accessor_read_uint(accessor, k + 2, &indices[2], 1);

					out->pTriangles[triangleOffset + k / 3].v1 = vertexOffset + indices[0];
					out->pTriangles[triangleOffset + k / 3].v2 = vertexOffset + indices[1];
					out->pTriangles[triangleOffset + k / 3].v3 = vertexOffset + indices[2];
				}
				triangleOffset += accessor->count / 3;
			}
			else
			{
				for (size_t k = 0; k < primitiveVertexCount; k += 3)
				{
					out->pTriangles[triangleOffset + k / 3].v1 = vertexOffset + k;
					out->pTriangles[triangleOffset + k / 3].v2 = vertexOffset + k + 1;
					out->pTriangles[triangleOffset + k / 3].v3 = vertexOffset + k + 2;
				}
				triangleOffset += primitiveVertexCount / 3;
			}

			vertexOffset += primitiveVertexCount;
		}
	}

	return 0;
}

void qkModelDestroy(qkModel* model)
{
	if (!model)
		return;

	free(model->pVertices);
	free(model->pTriangles);
	if (model->pGltfData)
	{
		cgltf_free(model->pGltfData);
	}

	memset(model, 0, sizeof(qkModel));
}