/*
Author: Alberto Quesada Ib��ez
Date: 8/06/2018
Info:
*/

/**
*  @class: View
*  @brief: Clase view, se encarga de cargar los shaders de la malla.
*  Crea una malla de elvaci�n, un skybox y una c�mara.
*  Carga los shaders de postprocesado.
*/

#ifndef VIEW_HEADER
#define VIEW_HEADER
	
	
    #include "Camera.hpp"
    #include "Skybox.hpp"
	#include <string>
	#include <GL/glew.h>            // Debe incluirse antes que gl.h	
	#include "Elevation_Mesh.hpp"
	#include <glm/glm.hpp>                          // vec3, vec4, ivec4, mat4
	#include <glm/gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
	#include <glm/gtc/type_ptr.hpp> 
	#include <vector>
	#include <Point.hpp>
	#include "Rasterizer.hpp"
	#include <Projection.hpp>
	#include <list>    
	#include "Mesh.hpp"

    namespace example
    {
		using std::vector;
		using std::string;
		using glm::vec3;
		using toolkit::Point4i;
		using toolkit::Point4f;
		using toolkit::Projection3f;

        class View
        {

		private:

			typedef Color_Buffer_Rgba8888 Color_Buffer;
			typedef Color_Buffer::Color   Color;

        private:

			// Width y height del frame_buffer

			static const GLsizei framebuffer_width = 512;
			static const GLsizei framebuffer_height = 512;

			// Shaders de la malla

			static const string   vertex_shader_code;
			static const string fragment_shader_code;

			// Shaders del efecto sepia de postprocesado

			static const string   effect_vertex_shader_code;
			static const string effect_fragment_shader_code;


			// Id de la model_view_matrix

			GLint  model_view_matrix_id;

			// Id de la projection_matrix

			GLint  projection_matrix_id;

			// Id de la normal_matrix

			GLint      normal_matrix_id;

			// Id del frame_buffer

			GLuint framebuffer_id;

			// Id del depth_buffer

			GLuint depthbuffer_id;

			// Id  de la textura que ser� el buffer de color vinculado al framebuffer

			GLuint out_texture_id;

			// �ngulo de rotaci�n de la malla			

			float  angle;			

			// Width y height de la ventana

			int    width;
			int    height;

			// Se crea la c�mara

            Camera camera;

			// Se crea el skybox

            Skybox skybox;

			// Se crea la elevation mesh

			Elevation_Mesh elevation_mesh;

			// Id del shader

			GLuint program_id;

			// Id del shader del postprocesado

			GLuint effect_program_id;

			// Tri�ngulos de postprocesado

			GLuint triangle_vbo0;
			GLuint triangle_vbo1;

			// �ngulos de rotaci�n

            float  angle_around_x;
            float  angle_around_y;
            float  angle_delta_x;
            float  angle_delta_y;

			// Coordenada de zoom 

			float z_coord;

			// Speed de movimiento de la c�mara

			float speed;

			// Vector de posici�n de la c�mara

			vec3 camera_pos;

			// Booleano para la detecci�n de presi�n del rat�n

            bool   pointer_pressed;
			
			// Guardamos las posiciones del rat�n

            int    last_pointer_x;
            int    last_pointer_y;

        public:

			// Constructor

            View(int width, int height);

			// Update 

            void update   ();

			// Render

            void render   ();

			// Resize de la pantalla

            void resize   (int width, int height);

			// Hacer zoom

			void do_zoom();

			// Quitar zoom

			void quit_zoom();

			// M�todos para la rotaci�n de la c�mara 
           
			void on_drag(int pointer_x, int pointer_y);
			void on_click(int pointer_x, int pointer_y, bool down);
			void set_pointer(bool value, int pointer_x, int pointer_y);

			// Movimiento de la c�mara hacia delante y atr�s

			void move_camera_front();	
			void move_camera_back();
			

		private:

			// Crear y renderizar el frame_buffer

			void   build_framebuffer();
			void   render_framebuffer();

			// Compilaci�n de shaders

			GLuint compile_shaders(const string & vertex_shader_code, const string & fragment_shader_code);

			// M�todos para los errores 

			void   show_compilation_error(GLuint  shader_id);
			void   show_linkage_error(GLuint program_id);

			// Configuraci�n de la luz

			void   configure_light(GLuint program_id);

        };

    }

#endif
