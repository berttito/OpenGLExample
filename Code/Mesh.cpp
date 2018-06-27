#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Mesh.hpp"
#include <cmath>
#include <cassert>
#include <Vector.hpp>
#include <Scaling.hpp>
#include <Rotation.hpp>
#include <Projection.hpp>
#include <Translation.hpp>

using namespace toolkit;
using namespace tinyobj;

namespace example

	//constructor de mesh que recibe el path, los componentes del color, la posicion y la escala
{
	Mesh::Mesh(const std::string & _path, float _Rcomp, float _Gcomp, float _Bcomp, float _xPos, float _yPos, float _scale)
	{
		attrib_t  attrib;
		vector< shape_t > shapes; 
		vector< material_t > materials; 		

		posX = _xPos;
		posY = _yPos;
		scale = _scale;

		bool isCharged = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, _path.c_str());

		if (!isCharged)
		{
			exit(1);
		}	

		//se guarda el size de los vertices

		const size_t vertex = attrib.vertices.size();

		//se recorren

		for (size_t i = 0; i < vertex; i += 3)
		{
			//por cada tres vertices se crea un point4 y se le pushea a original vertices
			
			original_vertices.push_back( Point4f ({ attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2], 1 }));
			
		}

		//se recorren los indices de la mesh
	
		for (auto const& index : shapes[0].mesh.indices)
		{
			//se pushean los vertex_index a original indices

			original_indices.push_back(index.vertex_index);			
		}

		//se hace el resize de original colors, display_vertices y transformed_vertices

		display_vertices.resize(original_vertices.size());
		transformed_vertices.resize(original_vertices.size());
		original_colors.resize(original_vertices.size());

		//se setean los componentes RGB a original colors
		
		for (auto & color : original_colors)
		{
			color.set(_Rcomp, _Gcomp, _Bcomp);
		}
	}



	bool Mesh::is_frontface(const Vertex * const projected_vertices, const int * const indices)
	{
		const Vertex & v0 = projected_vertices[indices[0]];
		const Vertex & v1 = projected_vertices[indices[1]];
		const Vertex & v2 = projected_vertices[indices[2]];

		// Se asumen coordenadas proyectadas y polígonos definidos en sentido horario.
		// Se comprueba a qué lado de la línea que pasa por v0 y v1 queda el punto v2:

		return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
	}

	void Mesh:: Update(Projection3f & projection)
	{
		// Se actualizan los parámetros de transformatión (sólo se modifica el ángulo):

		static float angle = 0.f;

		angle += 0.025f;

		// Se crean las matrices de transformación:

		Scaling3f     scaling(scale);
		Rotation3f    rotation_x;
		Rotation3f    rotation_y;
		Translation3f translation(posX, posY, -10);

		rotation_x.set< Rotation3f::AROUND_THE_X_AXIS >(0.50f);
		rotation_y.set< Rotation3f::AROUND_THE_Y_AXIS >(angle);

		// Creación de la matriz de transformación unificada:

		Transformation3f transformation = projection * translation * rotation_x * rotation_y * scaling;

		// Se transforman todos los vértices usando la matriz de transformación resultante:

		for (size_t index = 0, number_of_vertices = original_vertices.size(); index < number_of_vertices; index++)
		{
			// Se multiplican todos los vértices originales con la matriz de transformación y
			// se guarda el resultado en otro vertex buffer:

			Vertex & vertex = transformed_vertices[index] = Matrix44f(transformation) * Matrix41f(original_vertices[index]);

			// La matriz de proyección en perspectiva hace que el último componente del vector
			// transformado no tenga valor 1.0, por lo que hay que normalizarlo dividiendo:

			float divisor = 1.f / vertex[3];

			vertex[0] *= divisor;
			vertex[1] *= divisor;
			vertex[2] *= divisor;
			vertex[3] = 1.f;
		}

		// LLAMAR AL MÉTODO UPDATE() DE LOS NODOS HIJOS PASÁNDOLES transformation
	}

	void Mesh ::Paint(Rasterizer< Color_Buffer > & rasterizer)
	{
		// Se convierten las coordenadas transformadas y proyectadas a coordenadas
		// de recorte (-1 a +1) en coordenadas de pantalla con el origen centrado.
		// La coordenada Z se escala a un valor suficientemente grande dentro del
		// rango de int (que es lo que espera fill_convex_polygon_z_buffer).

		float width  = (float)rasterizer.get_color_buffer().get_width  ();
		float height = (float)rasterizer.get_color_buffer().get_height ();

		Scaling3f        scaling = Scaling3f(width / 2.f, height / 2.f, 100000000.f);
		Translation3f    translation = Translation3f(width / 2.f, height / 2.f, 0.f);
		Transformation3f transformation = translation * scaling;

		for (size_t index = 0, number_of_vertices = transformed_vertices.size(); index < number_of_vertices; index++)
		{
			display_vertices[index] = Point4i(Matrix44f(transformation) * Matrix41f(transformed_vertices[index]));
		}

		for (int * indices = original_indices.data(), *end = indices + original_indices.size(); indices < end; indices += 3)
		{
			if (is_frontface(transformed_vertices.data(), indices))
			{
				// Se establece el color del polígono a partir del color de su primer vértice:

				rasterizer.set_color(original_colors[*indices]);

				// Se rellena el polígono:

				rasterizer.fill_convex_polygon_z_buffer(display_vertices.data(), indices, indices + 3);
			}
		}

		// llamar al método paint de cada nodo hijo
	}


}


