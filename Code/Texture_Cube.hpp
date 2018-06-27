
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on may of 2016                                            *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TEXTURE_CUBE_HEADER
#define TEXTURE_CUBE_HEADER

    #include <memory>
    #include <string>
    #include <GL/glew.h>
    #include <SFML/OpenGL.hpp>
    #include "Color_Buffer_Rgba8888.hpp"

    namespace example
    {

        class Texture_Cube
        {
        private:

            typedef Color_Buffer_Rgba8888 Buffer;

        private:

            GLuint texture_id;
            bool   texture_is_loaded;

        public:

            Texture_Cube(const std::string & texture_base_path);
           ~Texture_Cube();

        private:

            Texture_Cube(const Texture_Cube & );
            Texture_Cube & operator = (const Texture_Cube & );

        private:

            std::shared_ptr< Buffer > load_texture (const std::string & texture_path);

        public:

            bool is_ok () const
            {
                return texture_is_loaded;
            }

            bool bind () const
            {
                return texture_is_loaded ? glBindTexture (GL_TEXTURE_CUBE_MAP, texture_id), true : false;
            }

        };

    }

#endif
