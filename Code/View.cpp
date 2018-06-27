/*
Author: Alberto Quesada Ibáñez
Date: 8/06/2018
Info:
*/

#include "View.hpp"
#include <iostream>
#include <cassert>
#include <glm/glm.hpp>                          // vec3, vec4, ivec4, mat4
#include <glm/gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp>                 // value_ptr
#include <SFML/OpenGL.hpp>

namespace example
{
	using namespace std;
	using glm::vec3;
	using glm::mat4;

	// Vertex shader de la malla

	const string View::vertex_shader_code =

		"#version 330\n"
		""
		"struct Light"
		"{"
		"    vec4 position;"
		"    vec3 color;"
		"};"
		""
		"uniform Light light;"
		"uniform float ambient_intensity;"
		"uniform float diffuse_intensity;"
		""
		"uniform mat4 model_view_matrix;"
		"uniform mat4 projection_matrix;"
		"uniform mat4     normal_matrix;"
		""
		"layout (location = 0) in vec3 vertex_coordinates;"
		"layout (location = 1) in vec3 vertex_normal;"
		"layout (location = 2) in vec2 vertex_texture_uv;"
		""
		"out vec2 texture_uv;"
		"out float light_intensity;"
		""
		"void main()"
		"{"
		"    vec4  normal   = normal_matrix     * vec4(vertex_normal,     1.0);"
		"    vec4  position = model_view_matrix * vec4(vertex_coordinates, 1.0);"
		""
		"    vec4  light_direction = light.position - position;"
		""
		"    light_intensity = diffuse_intensity * max (dot (normalize (normal.xyz), normalize (light_direction.xyz)), 0.0);"
		""
		"    texture_uv  = vertex_texture_uv;"
		"	 gl_Position = projection_matrix * model_view_matrix * vec4(vertex_coordinates, 1.0); "
		"}";

	// Fragment shader de la malla

	const string View::fragment_shader_code =

		"#version 330\n"
		""
		"uniform sampler2D sampler2d;"
		""
		"in  vec2  texture_uv;"
		"in  float light_intensity;"
		"out vec4  fragment_color;"
		""
		"void main()"
		"{"
		"    fragment_color = vec4((texture (sampler2d, texture_uv.st).rgb + vec3(0.5, 0.5, 0.5)) * light_intensity, 1.0);"
		"}";

	// Vertex shader del post procesado

	const string View::effect_vertex_shader_code =

		"#version 330\n"
		""
		"layout (location = 0) in vec3 vertex_coordinates;"
		"layout (location = 1) in vec2 vertex_texture_uv;"
		""
		"out vec2 texture_uv;"
		""
		"void main()"
		"{"
		"   gl_Position = vec4(vertex_coordinates, 1.0);"
		"   texture_uv  = vertex_texture_uv;"
		"}";

	// Fragment shader del post procesado

	const string View::effect_fragment_shader_code =

		"#version 330\n"
		""
		"uniform sampler2D sampler2d;"
		""
		"in  vec2 texture_uv;"
		"out vec4 fragment_color;"
		""
		"void main()"
		"{"
		"    vec3 color = texture (sampler2d, texture_uv.st).rgb;"
		"    float i = (color.r + color.g + color.b) * 0.3333333333;"
		"    fragment_color = vec4(vec3(i, i, i) * vec3(1.0, 0.75, 0.5), 1.0);"
		"}";

    View::View(int width, int height)
    :
        skybox("../../../../assets/sky-cube-map-"),
		angle(0),
		elevation_mesh(50, 50, -0.4, 2.f, 2.f, 0.8, "..\\..\\..\\..\\assets\\HeightMap.tga", "..\\..\\..\\..\\assets\\cesped.tga"),
		camera_pos(0, 0, 0)
    {
		// Se crea la textura y se dibuja algo en ella:
			
		build_framebuffer();

		// Se establece la configuración básica:

        glEnable (GL_DEPTH_TEST);
        glEnable (GL_CULL_FACE);
		glClearColor (0.2f, 0.2f, 0.2f, 1.f);
		
		// Se compilan y se activan los shaders:

		program_id = compile_shaders(vertex_shader_code, fragment_shader_code);
		effect_program_id = compile_shaders(effect_vertex_shader_code, effect_fragment_shader_code);

		glUseProgram(program_id);

		model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
		projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");
		normal_matrix_id = glGetUniformLocation(program_id, "normal_matrix");

		configure_light(program_id);

		resize(width, height);

		// Inicializamos valores de zoom, rotación y speed de la caámara

        angle_around_x  = angle_delta_x = 0.0;
        angle_around_y  = angle_delta_y = 0.0;        
        pointer_pressed = false;
		z_coord = 60;
		speed = 0.05f;
		
    }

    void View::update ()
    {
		// Ángulo de rotación de la malla

		angle += 0.5f;

		// Actualizamos los ángulos de la cámara

        angle_around_x += angle_delta_x;
        angle_around_y += angle_delta_y;

        if (angle_around_x < -1.5)
        {
            angle_around_x = -1.5;
        }
        else
        if (angle_around_x > +1.5)
        {
            angle_around_x = +1.5;
        }

        glm::mat4 camera_rotation;

        camera_rotation = glm::rotate (camera_rotation, glm::degrees (angle_around_y), vec3(0.f, 1.f, 0.f));
        camera_rotation = glm::rotate (camera_rotation, glm::degrees (angle_around_x), vec3(1.f, 0.f, 0.f));

		// Seteamos siempre location y target

		camera.set_location(0, 0, 0);
		camera.set_target(0, 0, -1);

		// Rotamos y movemos siempre en función de la posición y rotación

		camera.rotate(camera_rotation);
		camera.move(camera_pos);		
    }

    void View::render ()
    {
		// Buffer del postprocesado

		glViewport(0, 0, framebuffer_width, framebuffer_height);

		// Se activa el framebuffer de la textura

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);  

		// Renderizamos el skybox

		skybox.render(camera);

		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(program_id);

		// Controlamos la posición y rotación de la malla en función de su model_view_matrix

		mat4 model_view_matrix;

		model_view_matrix = camera.get_model_view();
		model_view_matrix = glm::translate(model_view_matrix, vec3(0.f, 0.f, -3.f));
		model_view_matrix = glm::rotate(model_view_matrix, angle, vec3(0.f, 1.f, 0.f));
		
		glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));

		// Matriz de normales

		mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

		glUniformMatrix4fv(normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));

		// Renderizamos la malla

		elevation_mesh.render();

		// Renderizamos el buffer de postprocesado

		render_framebuffer();		
    }

    void View::resize (int new_width, int new_height)
    {
		// Resize de la window

		width = new_width;
        height = new_height;

        camera.set_ratio (float(width) / height);

		mat4 projection_matrix = glm::perspective(60.f, GLfloat(width) / height, 0.1f, 50.f);

		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glViewport (0, 0, width, height);
    }

	void View::move_camera_front()
	{
		// Movemos la cámara hacia delante

		vec3 forward = vec3(camera.get_target() - camera.get_location());

		cout << "x: " << forward.x << endl;
		cout << "y: " << forward.y << endl;
		cout << "z: " << forward.z << endl;

		camera_pos += glm::normalize(forward * speed);	
	}

	void View::move_camera_back()
	{
		// Movemos la cámara hacia atrás

		vec3 forward = vec3(camera.get_target() - camera.get_location());

		cout << "x: " << forward.x << endl;
		cout << "y: " << forward.y << endl;
		cout << "z: " << forward.z << endl;

		camera_pos += glm::normalize(forward * -speed);
	}	

	void View::do_zoom() 
	{		
		// Hacemos zoom cambiando el fov de la cámara

		z_coord--;

		if (z_coord < 1)
		{
			z_coord = 1;
		}

		camera.set_fov(z_coord);

		mat4 projection_matrix = glm::perspective(camera.get_fov(), GLfloat(width) / height, 0.1f, 50.f);

		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));		

		cout << "z: " << camera.get_fov() << endl;
	}

	void View::quit_zoom() 
	{		
		// Quitamos zoom cambiando el fov de la cámara

		z_coord++;

		if (z_coord >= 90)
		{
			z_coord = 90;
		}
		
		camera.set_fov(z_coord);

		mat4 projection_matrix = glm::perspective(camera.get_fov(), GLfloat(width) / height, 0.1f, 50.f);

		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));		

		cout << "z: " << camera.get_fov() << endl;
	}

		
	void View::on_drag(int pointer_x, int pointer_y)
	{
		// Al arrastrar modificamos los ángulos para la rotación

		if (pointer_pressed)
		{
			angle_delta_x = 0.025f * float(last_pointer_y - pointer_y) / float(height);
			angle_delta_y = 0.025f * float(last_pointer_x - pointer_x) / float(width);
		}
	}

	void View::on_click(int pointer_x, int pointer_y, bool down)
	{
		// Al hacer click guardamos la posición

		if ((pointer_pressed = down) == true)
		{
			last_pointer_x = pointer_x;
			last_pointer_y = pointer_y;
		}
		else
		{
			angle_delta_x = angle_delta_y = 0.0;
		}
	}

	void View::set_pointer(bool value, int pointer_x, int pointer_y)
	{
		last_pointer_x = pointer_x;
		last_pointer_y = pointer_y;
		cout << "set_pointer: " << pointer_pressed << endl;
		pointer_pressed = value;
	}   

	void View::build_framebuffer()
	{
		// Se crea un framebuffer en el que poder renderizar:
		{
			glGenFramebuffers(1, &framebuffer_id);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
		}

		// Se crea una textura que será el buffer de color vinculado al framebuffer:
		{
			glGenTextures(1, &out_texture_id);
			glBindTexture(GL_TEXTURE_2D, out_texture_id);

			// El buffer de color tendrá formato RGB:

			glTexImage2D
			(
				GL_TEXTURE_2D,
				0,
				GL_RGB,
				framebuffer_width,
				framebuffer_height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				0
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		// Se crea un Z-Buffer para usarlo en combinación con el framebuffer:
		{
			glGenRenderbuffers(1, &depthbuffer_id);
			glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_id);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer_id);
		}

		// Se configura el framebuffer:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out_texture_id, 0);

			const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;

			glDrawBuffers(1, &draw_buffer);
		}

		// Se comprueba que el framebuffer está listo:

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// Se desvincula el framebuffer:

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Se crea el quad para hacer el render del framebuffer:

		static const GLfloat triangle_positions[] =
		{
			+1.0f, -1.0f, 0.0f,
			+1.0f, +1.0f, 0.0f,
			-1.0f, +1.0f, 0.0f,

			-1.0f, +1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			+1.0f, -1.0f, 0.0f,
		};

		static const GLfloat triangle_texture_uv[] =
		{
			+1.0f,  0.0f,
			+1.0f, +1.0f,
			 0.0f, +1.0f,

			0.0f, +1.0f,
			0.0f,  0.0f,
			1.0f,  0.0f
		};

		glGenBuffers(1, &triangle_vbo0);
		glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_positions), triangle_positions, GL_STATIC_DRAW);

		glGenBuffers(1, &triangle_vbo1);
		glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_texture_uv), triangle_texture_uv, GL_STATIC_DRAW);

	}

	void View::render_framebuffer()
	{
		glViewport(0, 0, width, height);

		// Se activa el framebuffer de la ventana:

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(effect_program_id);

		// Se activa la textura del framebuffer:

		glBindTexture(GL_TEXTURE_2D, out_texture_id);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}



	GLuint View::compile_shaders(const string & vertex_shader_code, const string & fragment_shader_code)
	{
		GLint succeeded = GL_FALSE;

		// Se crean objetos para los shaders:

		GLuint   vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

		// Se carga el código de los shaders:

		const char *   vertex_shaders_code[] = { vertex_shader_code.c_str() };
		const char * fragment_shaders_code[] = { fragment_shader_code.c_str() };
		const GLint    vertex_shaders_size[] = { (int)vertex_shader_code.size() };
		const GLint  fragment_shaders_size[] = { (int)fragment_shader_code.size() };

		glShaderSource(vertex_shader_id, 1, vertex_shaders_code, vertex_shaders_size);
		glShaderSource(fragment_shader_id, 1, fragment_shaders_code, fragment_shaders_size);

		// Se compilan los shaders:

		glCompileShader(vertex_shader_id);
		glCompileShader(fragment_shader_id);

		// Se comprueba que si la compilación ha tenido éxito:

		glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &succeeded);
		if (!succeeded) show_compilation_error(vertex_shader_id);

		glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &succeeded);
		if (!succeeded) show_compilation_error(fragment_shader_id);

		// Se crea un objeto para un programa:

		GLuint program_id = glCreateProgram();

		// Se cargan los shaders compilados en el programa:

		glAttachShader(program_id, vertex_shader_id);
		glAttachShader(program_id, fragment_shader_id);

		// Se linkan los shaders:

		glLinkProgram(program_id);

		// Se comprueba si el linkage ha tenido éxito:

		glGetProgramiv(program_id, GL_LINK_STATUS, &succeeded);
		if (!succeeded) show_linkage_error(program_id);

		// Se liberan los shaders compilados una vez se han linkado:

		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);

		return (program_id);
	}

	void View::show_compilation_error(GLuint shader_id)
	{
		string info_log;
		GLint  info_log_length;

		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

		info_log.resize(info_log_length);

		glGetShaderInfoLog(shader_id, info_log_length, NULL, &info_log.front());

		cerr << info_log.c_str() << endl;

#ifdef _MSC_VER
		OutputDebugStringA(info_log.c_str());
#endif

		assert(false);
	}

	void View::show_linkage_error(GLuint program_id)
	{
		string info_log;
		GLint  info_log_length;

		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);

		info_log.resize(info_log_length);

		glGetProgramInfoLog(program_id, info_log_length, NULL, &info_log.front());

		cerr << info_log.c_str() << endl;

#ifdef _MSC_VER
		OutputDebugStringA(info_log.c_str());
#endif

		assert(false);
	}

	void View::configure_light(GLuint program_id)
	{
		// Configuración de los parámetros de la luz

		GLint light_position = glGetUniformLocation(program_id, "light.position");
		GLint light_color = glGetUniformLocation(program_id, "light.color");
		GLint ambient_intensity = glGetUniformLocation(program_id, "ambient_intensity");
		GLint diffuse_intensity = glGetUniformLocation(program_id, "diffuse_intensity");

		glUniform4f(light_position, 10.f, 10.f, 10.f, 1.f);
		glUniform3f(light_color, 1.f, 1.f, 1.f);
		glUniform1f(ambient_intensity, 0.2f);
		glUniform1f(diffuse_intensity, 0.8f);
	}
}
