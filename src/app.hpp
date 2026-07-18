#ifndef APP_HPP
#define APP_HPP

#include <core/window.hpp>
#include <renderer/renderer.hpp>
#include <core/scene.hpp>

class App
{
private:
  int HEIGHT = 720;
  int WIDTH = 1280;
  Window window;
  Scene scene;
  Renderer renderer;

public:
  App()
      : window(WIDTH, HEIGHT),
        renderer(window, scene) {}

  void run()
  {
    while (!window.shouldClose())
    {
      window.pollEvents();
    }
  }
};

#endif // APP_HPP
