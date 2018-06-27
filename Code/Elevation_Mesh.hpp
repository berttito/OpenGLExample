/*
Author: Alberto Quesada Ibáñez
Date: 8/06/2018
Info:
*/

/**
*  @class: Elevation_Mesh
*  @brief: Clase elevation mesh, crea una malla de elevación a partir de una textura.
*  Texturiza la malla de elevación.
*  Calcula sus normales para poder utilizarlas a la hora de darle una iluminación determinada.
*/

#ifndef ELEVATION_MESH_HEADER
#define ELEVATION_MESH_HEADER

	#include <memory>
	#include <glm\vec3.hpp>
    #include <SFML/OpenGL.hpp>

	using glm::vec3;
	using namespace std;

    namespace example
    {
		class   Color_Buffer_Rgba8888;
		typedef Color_Buffer_Rgba8888 Texture;

        class Elevation_Mesh
        {

        private:

			// Struct punto con dos coordenadas

			struct Point2f
			{
				float x, y;
			};

			// Struct punto con tres coordenadas

			struct Point3f
			{
				float x, y, z;
			};

			// Color con tres valores

			struct Color
			{
				uint8_t x, y, z;
			};
			
            // Índices para indexar el array vbo_ids:

            enum
            {
                COORDINATES_VBO,
                UVS_VBO,
				COLORS_VBO,
				NORMALS_VBO,
				INDICES_IBO,
				TEXTURE_COORDS_VBO,
                VBO_COUNT
            };

        private:

			// Ids de los VBOs que se usan

            GLuint  vbo_ids[VBO_COUNT]; 

			// Número de índices

			GLsizei number_of_indices;

			// Id del VAO del cubo

			GLuint vao_id;            

			// Número de filas 

			int num_rows;

			// Número de columnas

			int num_cols;


		private:

			// Booleano para comprobar si carga la textura

			bool   has_texture;

			// Id de la textura de elevación

			GLuint textureElevation_id;

			// Id de la textura 

			GLuint texture_id;

        public:

			// Constructor

			Elevation_Mesh(int cols, int rows, float height, float width, float depth, float elevation, const char * textureElevation_path = 0, const char * textureImage_path = 0);

			// Destructor

           ~Elevation_Mesh();

		   // Render de la malla 

            void render ();

			// Método para el cálculo de la normal de un triángulo

			vec3 calculateNormal(const vec3 & a, const vec3 & b, const vec3 & c);	

			// Método para el cálculo de todas las normales de la malla

			void calculateNormals(vector< vec3  > & positions, int cols, int rows, float x_step, float z_step, vector< vec3  > & normals);

			// Método para la carga de las texturas

			auto_ptr< Texture > load_texture(const char * texture_file_path);

        };

    }

#endif
