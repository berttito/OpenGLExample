
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on march of 2014                                          *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "View.hpp"
#include <GL/glew.h>            // Debe incluirse antes que gl.h
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>

using namespace sf;
using namespace example;
using namespace std;

int main ()
{
    Window window(VideoMode(800, 600), "OpenGL Examples: Skybox", Style::Default, ContextSettings(32));

    // Una vez se ha creado el contexto de OpenGL ya se puede inicializar Glew:

    GLenum glew_initialization =  glewInit ();

    assert(glew_initialization == GLEW_OK);

    // Glew se inicializa antes de crear view porque view ya usa extensiones de OpenGL:

    View view (800, 600);

    window.setVerticalSyncEnabled (true);

    bool running = true;

    do
    {
        Event event;

        while (window.pollEvent (event))
        {
            switch (event.type)
            {
                case Event::Closed:
                {
                    running = false;
                    break;
                }

                case Event::Resized:
                {
                    Vector2u window_size = window.getSize ();

                    view.resize (window_size.x, window_size.y);

                    break;
                }

                case Event::KeyPressed:
                {
					if (event.key.code == sf::Keyboard::O) 
					{						
						view.do_zoom();
					}

					if (event.key.code == sf::Keyboard::P)
					{
						view.quit_zoom();
					}

					if (event.key.code == sf::Keyboard::W)
					{
						view.move_camera_front();
					}

					if (event.key.code == sf::Keyboard::S)
					{
						view.move_camera_back();
					}

				

					break;
                }

				case Event::KeyReleased:
				{
					break;
				}

                case Event::MouseButtonPressed:
                {
                    view.on_click (event.mouseButton.x, event.mouseButton.y, true);
                    break;
                }

                case Event::MouseButtonReleased:
                {
                    view.on_click (event.mouseButton.x, event.mouseButton.y, false);
                    break;
                }

                case Event::MouseMoved:
                {						
                    view.on_drag (event.mouseMove.x, event.mouseMove.y);
                    break;
                }

				
				/*case Event::MouseEntered:
				{
					view.on_click(event.mouseButton.x, event.mouseButton.y, true);
					break;
				}

				case Event::MouseLeft:
				{
					view.on_enter(event.mouseButton.x, event.mouseButton.y, false);
					break;
				}*/
            }
        }

        view.update ();
        view.render ();

        window.display ();
    }
    while (running);

    return (EXIT_SUCCESS);
}
