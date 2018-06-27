
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on may of 2016                                            *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <vector>
#include "Texture_Cube.hpp"

extern "C"
{
    #include <targa.h>
}

namespace example
{

    typedef Color_Buffer_Rgba8888 Buffer;

    Texture_Cube::Texture_Cube(const std::string & texture_base_path)
    {
        texture_is_loaded = false;

        // Se intentan cargar los mapas de bits de todas las caras:

        std::vector< std::shared_ptr< Buffer > > texture_sides(6);

        for (size_t texture_index = 0; texture_index < 6; texture_index++)
        {
            texture_sides[texture_index] = load_texture (texture_base_path + char('0' + texture_index) + ".tga");

            if (!texture_sides[texture_index])
            {
                return;
            }
        }

        // Se crea un objeto de textura:

        glEnable        (GL_TEXTURE_CUBE_MAP);

        glGenTextures   (1, &texture_id);

        glActiveTexture (GL_TEXTURE0);
        glBindTexture   (GL_TEXTURE_CUBE_MAP, texture_id);

        // Se configura la textura: escalado suavizado, clamping de coordenadas (s,t) hasta el borde:

        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);

        // Se envían los mapas de bits a la GPU:

        static const GLenum texture_target[] =
        {
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        };

        for (size_t texture_index = 0; texture_index < 6; texture_index++)
        {
            Buffer & texture = *texture_sides[texture_index];

            glTexImage2D
            (
                texture_target[texture_index], 
                0, 
                GL_RGBA, 
                texture.get_width  (),
                texture.get_height (),
                0, 
                GL_RGBA, 
                GL_UNSIGNED_BYTE, 
                texture.colors ()
            );
        }

        texture_is_loaded = true;
    }

    std::shared_ptr< Buffer > Texture_Cube::load_texture (const std::string & texture_path)
    {
        std::shared_ptr< Buffer > buffer;

        // Se carga el archivo TGA:

        tga_image loaded_image;

        if (tga_read (&loaded_image, texture_path.c_str ()) == TGA_NOERR)
        {
            // Se crea un búfer con el formato de píxel adecuado:

            buffer.reset (new Buffer(loaded_image.width, loaded_image.height));

            // Se convierte el búfer de la imagen cargada al formato de píxel adecuado:

            tga_convert_depth (&loaded_image, buffer->bits_per_color ());
            tga_swap_red_blue (&loaded_image);

            Buffer::Color * loaded_image_pixels     = reinterpret_cast< Buffer::Color * >(loaded_image.image_data);
            Buffer::Color * loaded_image_pixels_end = loaded_image_pixels + loaded_image.width * loaded_image.height;
            Buffer::Color * texture_buffer_pixels   = buffer->colors ();

            while (loaded_image_pixels <  loaded_image_pixels_end)
            {
                *texture_buffer_pixels++ = *loaded_image_pixels++;
            }

            tga_free_buffers (&loaded_image);
        }

        return buffer;
    }

    Texture_Cube::~Texture_Cube()
    {
        if (texture_is_loaded)
        {
            glDeleteTextures (1, &texture_id);
        }
    }

}
