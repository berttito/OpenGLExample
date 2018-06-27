#pragma once

#include <vector>
#include <Point.hpp>
#include "Rasterizer.hpp"
#include <Projection.hpp>
#include "Color_Buffer_Rgb565.hpp"
#include "Color_Buffer_Rgba8888.hpp"

using std::vector;
using toolkit::Point4i;
using toolkit::Point4f;
using toolkit::Projection3f;

namespace example
{
	class Mesh {

		typedef Color_Buffer_Rgba8888 Color_Buffer;
		typedef Color_Buffer::Color   Color;

	private:

		typedef Point4f               Vertex;
		typedef vector< Vertex >      Vertex_Buffer;
		typedef vector< int    >      Index_Buffer;
		typedef vector< Color  >      Vertex_Colors;

		Vertex_Buffer     original_vertices;
		Index_Buffer      original_indices;
		Vertex_Colors     original_colors;

		Vertex_Buffer     transformed_vertices;
		vector< Point4i > display_vertices;

		//Posiciones y escala para setearlas al crear cada mesh

		float posX;
		float posY;
		float scale;	

		std::string error;

	public:

		Mesh(const std::string & _path,float _Rcomp,float _Gcomp, float _Bcomp,float _xPos, float _yPos, float _scale);

	public:

		//Metodos update y paint que tienen las mesh
		
		void Update(Projection3f & projection);
		void Paint(Rasterizer< Color_Buffer > & rasterizer);

	private:

		bool is_frontface(const Vertex * const projected_vertices, const int * const indices);


	

	};



}