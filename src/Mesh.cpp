#include <crucible/Mesh.hpp>
#include <glad/glad.h>

#include <sstream>

Mesh::Mesh() {

}

Mesh::Mesh(std::vector<vec3> positions, std::vector<unsigned int> indices) {
    this->positions = positions;
    this->indices = indices;
}

Mesh::Mesh(std::vector<vec3> positions, std::vector<vec3> normals, std::vector<unsigned int> indices) {
    this->positions = positions;
    this->normals = normals;
    this->indices = indices;
}

Mesh::Mesh(std::vector<vec3> positions, std::vector<vec3> normals, std::vector<vec2> uvs, std::vector<unsigned int> indices) {
    this->positions = positions;
    this->normals = normals;
    this->uvs = uvs;
    this->indices = indices;
}

Mesh::Mesh(std::vector<vec3> positions, std::vector<vec3> normals, std::vector<vec2> uvs, std::vector<vec3> colors, std::vector<unsigned int> indices) {
    this->positions = positions;
    this->normals = normals;
    this->uvs = uvs;
    this->tangents = colors;
    this->indices = indices;
}

void Mesh::generate() {


    if (!VBO) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (uvs.size() > 0)
        {
            data.push_back(uvs[i].x);
            data.push_back(uvs[i].y);
        }
        if (tangents.size() > 0)
        {
            data.push_back(tangents[i].x);
            data.push_back(tangents[i].y);
            data.push_back(tangents[i].z);
        }
    }

    length = positions.size();

	if (data.size() > 0) {

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		if (indices.size() > 0)
		{
		    if (!EBO)
			    glGenBuffers(1, &EBO);


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			length = indices.size();
		}

		int stride = 3 * sizeof(float);
		if (uvs.size() > 0) stride += 2 * sizeof(float);
		if (normals.size() > 0) stride += 3 * sizeof(float);
		if (tangents.size() > 0) stride += 3 * sizeof(float);


		long offset = 0;
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
		offset += 3 * sizeof(float);

		if (normals.size() > 0)
		{
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
			offset += 3 * sizeof(float);
		}
		if (uvs.size() > 0)
		{
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
			offset += 2 * sizeof(float);
		}
		if (tangents.size() > 0) {
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
			offset += 3 * sizeof(float);
		}
	}
}

void Mesh::clear() {
    positions.clear();
    uvs.clear();
    normals.clear();
    tangents.clear();
    indices.clear();
}

void Mesh::render() {
    glBindVertexArray(VAO);

    if (EBO) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glDrawElements(renderMode, length, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(renderMode, 0, length);
    }

    glBindVertexArray(0);
}

// https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
template<typename Out>
static void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())
            *(result++) = item;
    }
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
// ------------------------------------------------------------------------------------------------


void Mesh::fromstring(std::string data) {
    clear();

    std::vector<std::string> components = split(data, '/');

    for (int i = 0; i < components.size(); i++) {
        //std::cout << components[i] << std::endl;

        std::vector<std::string> rawContents = split(components[i], ',');

        std::string stype = rawContents[0];
        int type = -1; // store type as integer for faster looping

        std::vector<std::string> contents(rawContents.begin() + 1, rawContents.end()); // since the first element is the identifier, let's get rid of it to avoid off-by-one-errors


        if (!stype.compare("p")) {
            //std::cout << "positions" << std::endl;
            type = 0;
        }
        if (!stype.compare("n")) {
            //std::cout << "normals" << std::endl;
            type = 1;
        }
        if (!stype.compare("u")) {
            //std::cout << "uvs" << std::endl;
            type = 2;
        }
        if (!stype.compare("t")) {
            //std::cout << "tangents" << std::endl;
            type = 3;
        }
        if (!stype.compare("i")) {
            //std::cout << "indices" << std::endl;
            type = 4;
        }


        for (int j = 0; j < contents.size(); j++) {
            std::vector<std::string> dimensions = split(contents[j], '_');

            if (type == 0) {
                //positions
                positions.push_back({stof(dimensions[0]), stof(dimensions[1]), stof(dimensions[2])});
                //std::cout << "inserting " << stof(dimensions[0]) << ", " << stof(dimensions[1]) << ", " << stof(dimensions[2]) << std::endl;
            }

            if (type == 1) {
                //normals
                normals.push_back({stof(dimensions[0]), stof(dimensions[1]), stof(dimensions[2])});
            }

            if (type == 2) {
                //uvs
                uvs.push_back({stof(dimensions[0]), stof(dimensions[1])});
            }

            if (type == 3) {
                //tangents
                tangents.push_back({stof(dimensions[0]), stof(dimensions[1]), stof(dimensions[2])});
            }

            if (type == 4) {
                //indices
                indices.push_back(stoi(contents[j]));
            }
        }
    }
}

std::string Mesh::tostring() {
    std::stringstream buf;

    buf << "p";
    for (int i = 0; i < positions.size(); i++) {

        vec3 pos = positions[i];
        buf << "," << pos.x << "_" << pos.y << "_" << pos.z;
    }
    buf << "/";


    if (normals.size() > 0) {
        buf << "n";
        for (int i = 0; i < normals.size(); i++) {

            vec3 normal = normals[i];
            buf << "," << normal.x << "_" << normal.y << "_" << normal.z;
        }
        buf << "/";
    }

    if (uvs.size() > 0) {
        buf << "u";
        for (int i = 0; i < uvs.size(); i++) {

            vec2 uv = uvs[i];
            buf << "," << uv.x << "_" << uv.y;
        }
        buf << "/";
    }

    if (tangents.size() > 0) {
        buf << "t";
        for (int i = 0; i < tangents.size(); i++) {

            vec3 tangent = tangents[i];
            buf << "," << tangent.x << "_" << tangent.y << "_" << tangent.z;
        }
        buf << "/";
    }


    if (indices.size() > 0) {
        buf << "i";
        for (int i = 0; i  < indices.size(); i++) {

            buf << "," << indices[i];
        }
    }

    return buf.str();
}

void Mesh::destroy() {
    clear();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
