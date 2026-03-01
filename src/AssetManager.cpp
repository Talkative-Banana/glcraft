#include "AssetManager.h"

FILE *AssetManager::LoadModel(const char *path) {
  FILE *file = fopen(path, "r");
  return file;
}

uint64_t AssetManager::loadMeshObject(const char *modelpath,
                                      unsigned int shaderProgram, double scale,
                                      double angle, glm::dvec3 pos,
                                      glm::dvec3 axis) {
  std::vector<int> vertex_indices, uv_indices, normal_indices;
  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  FILE *file = LoadModel(modelpath);
  if (file == NULL) {
    printf("File not found\n");
    return 0;
  }

  while (true) {

    char head[128];
    // read the first word of the line
    int res = fscanf(file, "%s", head);
    if (res == EOF)
      break; // EOF = End Of File. Quit the loop.

    if (strcmp(head, "v") == 0) {
      glm::vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      // printf("%f %f %f\n", vertex.x, vertex.y, vertex.z );
      temp_vertices.push_back(vertex);
    }

    else if (strcmp(head, "vt") == 0) {
      glm::vec2 uv;
      fscanf(file, "%f %f\n", &uv.x, &uv.y);
      // printf("%f %f\n", uv.x, uv.y );
      temp_uvs.push_back(uv);
    }

    else if (strcmp(head, "vn") == 0) {
      glm::vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      // printf("%f %f %f\n", normal.x, normal.y, normal.z );
      temp_normals.push_back(normal);
    }

    else if (strcmp(head, "f") == 0) {
      int vertexIndex[3], uvIndex[3], normalIndex[3];
      long int pos = ftell(file);
      // Try to read faces in the format of v/t/n
      int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                           &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                           &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                           &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

      if (matches == 9) {
        vertex_indices.push_back(vertexIndex[0]);
        vertex_indices.push_back(vertexIndex[1]);
        vertex_indices.push_back(vertexIndex[2]);
        uv_indices.push_back(uvIndex[0]);
        uv_indices.push_back(uvIndex[1]);
        uv_indices.push_back(uvIndex[2]);
        normal_indices.push_back(normalIndex[0]);
        normal_indices.push_back(normalIndex[1]);
        normal_indices.push_back(normalIndex[2]);
      } else {
        // Reset file pointer to retry parsing in different format
        fseek(file, pos, SEEK_SET);

        // Try to read faces in the format of v//n
        matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0],
                         &normalIndex[0], &vertexIndex[1], &normalIndex[1],
                         &vertexIndex[2], &normalIndex[2]);

        if (matches == 6) {
          vertex_indices.push_back(vertexIndex[0]);
          normal_indices.push_back(normalIndex[0]);
          vertex_indices.push_back(vertexIndex[1]);
          normal_indices.push_back(normalIndex[1]);
          vertex_indices.push_back(vertexIndex[2]);
          normal_indices.push_back(normalIndex[2]);
        } else {
          // Handle other formats or errors appropriately
          fprintf(stderr, "Unsupported face format or error occurred\n");
        }
      }
    }
  }
  fclose(file);

  std::vector<float> shape_vertices(vertex_indices.size() * 3),
      vertex_normals(normal_indices.size() * 3),
      uv_normals(uv_indices.size() * 3);

  // vertex_indices => consecutive three triangle coords
  for (size_t i = 0; i < vertex_indices.size(); i++) {
    int vertex_index = vertex_indices[i];
    shape_vertices[i * 3] = temp_vertices[vertex_index - 1][0];
    shape_vertices[i * 3 + 1] = temp_vertices[vertex_index - 1][1];
    shape_vertices[i * 3 + 2] = temp_vertices[vertex_index - 1][2];
  }

  // generated normals for the triangle mesh
  for (int i = 0; i < vertex_indices.size(); i += 3) {
    glm::vec3 v1 =
        glm::vec3(shape_vertices[(i + 1) * 3] - shape_vertices[i * 3],
                  shape_vertices[(i + 1) * 3 + 1] - shape_vertices[i * 3 + 1],
                  shape_vertices[(i + 1) * 3 + 2] - shape_vertices[i * 3 + 2]);
    glm::vec3 v2 = glm::vec3(
        shape_vertices[(i + 2) * 3] - shape_vertices[(i + 1) * 3],
        shape_vertices[(i + 2) * 3 + 1] - shape_vertices[(i + 1) * 3 + 1],
        shape_vertices[(i + 2) * 3 + 2] - shape_vertices[(i + 1) * 3 + 2]);

    glm::vec3 n = cross(v1, v2);

    vertex_normals[i * 3] = n.x;
    vertex_normals[(i + 1) * 3] = n.x;
    vertex_normals[(i + 2) * 3] = n.x;
    vertex_normals[i * 3 + 1] = n.y;
    vertex_normals[(i + 1) * 3 + 1] = n.y;
    vertex_normals[(i + 2) * 3 + 1] = n.y;
    vertex_normals[i * 3 + 2] = n.z;
    vertex_normals[(i + 1) * 3 + 2] = n.z;
    vertex_normals[(i + 2) * 3 + 2] = n.z;
  }

  std::vector<AssetVertex> vertices;

  for (int i = 0; i < vertex_indices.size(); i++) {
    vertices.push_back({{shape_vertices[i * 3], shape_vertices[i * 3 + 1],
                         shape_vertices[i * 3 + 2]},
                        {vertex_normals[i * 3], vertex_normals[i * 3 + 1],
                         vertex_normals[i * 3 + 2]}});
  }

  static glm::uint64 handle = 1;
  std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(
      std::move(vertices), shaderProgram, scale, angle, pos, axis);
  m_assets.emplace(++handle, mesh);

  return handle;
}
