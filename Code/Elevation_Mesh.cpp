/*
Author: Alberto Quesada Ibáñez
Date: 8/06/2018
Info:
*/

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>            // Debe incluirse antes que gl.h
#include <SFML/OpenGL.hpp>
#include "Elevation_Mesh.hpp"
#include "Color_Buffer_Rgba8888.hpp"

extern "C"
{
#include <targa.h>
}

using std::vector;
using namespace glm;
using namespace std;

namespace example
{
	Elevation_Mesh::Elevation_Mesh(int cols, int rows,float height, float width, float depth, float elevation, const char * textureElevation_path, const char * textureImage_path) : has_texture(false)
	{
		// Inicializamos los valores

		num_cols = cols;
		num_rows = rows;
		unsigned number_of_vertices = cols      *  rows;
		number_of_indices = (cols - 1) * (rows - 1) * 2 * 3;

		// Iniciamos los vectores con los números de vértices

		vector< vec3  > positions(number_of_vertices);
		vector< vec3  > normals(number_of_vertices);
		vector< Point2f  > uvs(number_of_vertices);
		vector< unsigned > indices;

		indices.reserve(number_of_indices);

		//CARGAR TEXTURA DE ELEVACION
		if (textureElevation_path != 0)
		{
			// Cargamos las texturas

			auto_ptr< Texture > texture = load_texture(textureElevation_path);
			auto_ptr <Texture> texture2 = load_texture(textureImage_path);

			has_texture = texture2.get () != nullptr;

			// Se generan los datos de las posiciones:

			//Distancia que hay de columna a columna

			float x_step = width / (cols - 1);

			//Distancia de fila a fila

			float z_step = depth / (rows - 1);

			//Distancia que hay de columna a columna en la textura

			float u_step = texture->get_width() / (cols - 1);

			//Distancia que hay entre fila y fila en la textura

			float v_step = texture->get_width() / (rows - 1);

			//Posicion real en el mapa de la malla

			float z = -width / 2.f;

			//Coordenadas de textura

			float v = 0;

			for (int row = 0, i = 0; row < rows; ++row)		// row = z's
			{
				//Posicion real en el mapa de la malla

				float x = -width / 2.f;

				//Coordenadas de textura

				float u = 0;

				for (int col = 0; col < cols; ++col, ++i)		// col = x's
				{
					float h = 0.f;

					// SAMPLEAR LA TEXTURA USANDO col(x) y row(y) Y GUARDAR LA ALTURA EN h

					h = texture->get_pixel(u, v).data.component.r * elevation / 255 + height;

					//Posicion real donde va a estar la malla

					positions[i] = vec3{ x, h, z };

					//Tileado de la malla que va de 0 a 1 todo el rato

					uvs[i] = Point2f{ (float)(row % 2) , (float)(col % 2) };
					x += x_step;
					u += u_step;
				}

				v += v_step;
				z += z_step;
			}

			// Se generan los datos de los índices:

			for (int row = 0; row < rows - 1; ++row)				// r = z's
			{
				int i = row * cols;

				for (int col = 0; col < cols - 1; ++col, ++i)		// c = x's
				{
					// Se añade el primer triángulo:

					indices.push_back(i + 1);
					indices.push_back(i);
					indices.push_back(i + cols);

					// Se añade el segundo triángulo:

					indices.push_back(i + cols);
					indices.push_back(i + cols + 1);
					indices.push_back(i + 1);
				}
			}

			// Calculamos las normales de la malla

			calculateNormals(positions, cols, rows, x_step, z_step, normals);

			// Si tiene textura bindeamos

			if (has_texture) 
			{
				//texture
				glEnable(GL_TEXTURE_2D);
				glGenTextures(2, &texture_id);
				glBindTexture(GL_TEXTURE_2D, texture_id);

				glTexImage2D
				(
					GL_TEXTURE_2D,
					0,
					GL_RGBA,
					texture2->get_width(),
					texture2->get_height(),
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					texture2->colors()
				);

				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			}
			
		}

		
		// Se generan índices para los VBOs del cubo:

		glGenBuffers(VBO_COUNT, vbo_ids);
		glGenVertexArrays(1, &vao_id);

		// Se activa el VAO del cubo para configurarlo:

		glBindVertexArray(vao_id);

		// Se suben a un VBO los datos de coordenadas:

		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Point3f), positions.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Se suben a un VBO los datos de normales y se vinculan al VAO:

		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMALS_VBO]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Se suben a un VBO los datos de text_coords:

		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[TEXTURE_COORDS_VBO]);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(Point2f), uvs.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Se suben a un IBO los datos de índices:

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_IBO]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);	
	}


	void Elevation_Mesh::calculateNormals(vector< vec3  > & positions, int cols, int rows, float x_step, float z_step, vector< vec3  > & normals)
	{
		//CALCULATE NORMALS

		unsigned vertex_index = 0;

		auto offset = [&cols](unsigned c, unsigned r) { return r * cols + c; };

		for (unsigned row = 0; row < rows; ++row)
		{
			for (unsigned col = 0; col < cols; ++col)
			{
				// a es el punto central y los demás están alrededor:

				//   g f
				// b a e
				// c d  
				vec3 a, b, c, d, e, f, g;

				//Punto central
				a = positions[offset(col, row)];

				if (col > 0) b = positions[offset(col - 1, row)]; else b = vec3(a.x - x_step, a.y, a.z);

				if (col > 0 && row < rows - 1) c = positions[offset(col - 1, row + 1)]; else c = vec3(a.x - x_step, a.y, a.z + z_step);

				if (row < rows - 1) d = positions[offset(col, row + 1)]; else d = vec3(a.x, a.y, a.z + z_step);

				if (col < cols - 1) e = positions[offset(col + 1, row)]; else b = vec3(a.x + x_step, a.y, a.z);

				if (col < cols - 1 && row > 0) f = positions[offset(col + 1, row - 1)]; else f = vec3(a.x + x_step, a.y, a.z - z_step);

				if (row > 0) g = positions[offset(col, row - 1)]; else g = vec3(a.x, a.y, a.z - z_step);

				// Calculamos las normales por cada triángulo

				vec3 n0 = calculateNormal(g, b, a);
				vec3 n1 = calculateNormal(a, b, c);
				vec3 n2 = calculateNormal(a, c, d);
				vec3 n3 = calculateNormal(a, d, e);
				vec3 n4 = calculateNormal(a, f, e);
				vec3 n5 = calculateNormal(a, f, g);

				// Vamos incorporando las normales

				normals[vertex_index++] = glm::normalize((n0 + n1 + n2 + n3 + n4 + n5) / 6.f);
			}
		}
	}

	Elevation_Mesh::~Elevation_Mesh()
	{
		if (has_texture)
		{
			glDeleteTextures(1, &textureElevation_id);
			glDeleteTextures(2, &texture_id);
		}

		// Se liberan los VBOs y el VAO usados:

		glDeleteVertexArrays(1, &vao_id);
		glDeleteBuffers(VBO_COUNT, vbo_ids);
	}
	
	vec3 Elevation_Mesh::calculateNormal(const vec3 & a, const vec3 & b, const vec3 & c)
	{
		// Calculamos el cross

		return glm::cross (b - a, c - a);
	}

	void Elevation_Mesh::render()
	{
		if (has_texture) 
		{
			// Se selecciona la textura

			glBindTexture(GL_TEXTURE_2D, texture_id);       
		}
	
		// Se selecciona el VAO que contiene los datos del objeto y se dibujan sus elementos:

		glBindVertexArray(vao_id);
		glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	std::auto_ptr< Texture > Elevation_Mesh::load_texture(const char * texture_file_path)
	{
		std::auto_ptr< Texture > texture;
		tga_image                loaded_image;

		if (tga_read(&loaded_image, texture_file_path) == TGA_NOERR)
		{
			// Si se ha podido cargar la imagen desde el archivo, se crea un búfer para una textura:

			texture.reset(new Texture(loaded_image.width, loaded_image.height));

			// Se convierte el formato de píxel de la imagen cargada a RGBA8888:

			tga_convert_depth(&loaded_image, texture->bits_per_color());
			tga_swap_red_blue(&loaded_image);

			// Se copian los pixels del búfer de la imagen cargada al búfer de la textura:

			Texture::Color * loaded_image_pixels = reinterpret_cast< Texture::Color * >(loaded_image.image_data);
			Texture::Color * loaded_image_pixels_end = loaded_image_pixels + loaded_image.width * loaded_image.height;
			Texture::Color * image_buffer_pixels = texture->colors();

			while (loaded_image_pixels <  loaded_image_pixels_end)
			{
				*image_buffer_pixels++ = *loaded_image_pixels++;
			}

			tga_free_buffers(&loaded_image);
		}

		return (texture);
	}
}
