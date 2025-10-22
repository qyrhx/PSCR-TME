#pragma once

#include <thread>

#include "Scene.h"
#include "Image.h"
#include "Ray.h"

namespace pr {

// Classe pour rendre une scène dans une image
class Renderer {
 public:
  // Rend la scène dans l'image
  void render(const Scene &scene, Image &img) {
    // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
    // on tire un rayon de l'observateur vers chacun de ces points
    const Scene::screen_t &screen = scene.getScreenPoints();

    // pour chaque pixel, calculer sa couleur
    for (int x = 0; x < scene.getWidth(); x++) {
      for (int y = 0; y < scene.getHeight(); y++) {
        // le point de l'ecran par lequel passe ce rayon
        auto &screenPoint = screen[y][x];
        // le rayon a inspecter
        Ray ray(scene.getCameraPos(), screenPoint);

        int targetSphere = scene.findClosestInter(ray);

        if (targetSphere == -1) {
          // keep background color
          continue;
        } else {
          const Sphere &obj = scene.getObject(targetSphere);
          // pixel prend la couleur de l'objet
          Color finalcolor = scene.computeColor(obj, ray);
          // mettre a jour la couleur du pixel dans l'image finale.
          img.pixel(x, y) = finalcolor;
        }
      }
    }
  }

  void renderThreadPerPixel(const Scene &scene, Image &img) {
    std::vector<std::thread> slaves;
    slaves.reserve(scene.getHeight() * scene.getWidth());

    for (int y = 0; y < scene.getHeight(); ++y) {
      for (int x = 0; x < scene.getWidth(); ++x) {
        auto &screenPoint = scene.getScreenPoints()[y][x];

        auto job = [&, x, y] {
          Ray ray(scene.getCameraPos(), screenPoint);
          int targetSphere = scene.findClosestInter(ray);
          if (targetSphere == -1)
            return;  // keep background color
          const Sphere &obj = scene.getObject(targetSphere);
          // pixel prend la couleur de l'objet
          Color finalcolor = scene.computeColor(obj, ray);
          // mettre a jour la couleur du pixel dans l'image finale.
          img.pixel(x, y) = finalcolor;
        };

        slaves.emplace_back(job);
      };
    }
    for (auto &slave : slaves)
      slave.join();
  }

  void renderThreadPerRow(const Scene &scene, Image &img) {
    std::vector<std::thread> slaves;
    slaves.reserve(scene.getHeight());

    for (int y = 0; y < scene.getHeight(); ++y) {
      auto job = [&scene, &img, y] {
        for (int x = 0; x < scene.getWidth(); ++x) {
          auto &screenPoint = scene.getScreenPoints()[y][x];
          Ray ray(scene.getCameraPos(), screenPoint);
          int targetSphere = scene.findClosestInter(ray);

          Color finalcolor = Color{255, 255, 255};
          if (targetSphere == -1)
            continue;  // keep background color

          const Sphere &obj = scene.getObject(targetSphere);
          // pixel prend la couleur de l'objet
          finalcolor = scene.computeColor(obj, ray);
          // mettre a jour la couleur du pixel dans l'image finale.
          img.pixel(x, y) = finalcolor;
        }
      };
      slaves.emplace_back(job);
    }

    for (auto &slave : slaves)
      slave.join();
  }

  void renderThreadManual(const Scene &scene, Image &img, int nbthread) {
    std::vector<std::thread> slaves;
    slaves.reserve(nbthread);

    int offset = scene.getHeight() / nbthread;
    std::cout << "Img rows: " << scene.getHeight() << ", offset: " << offset << '\n';
    for (int i = 0; i < nbthread; ++i) {
      int begin = i * offset;
      int end   = begin + offset;
      if (i == nbthread-1)
        end = scene.getHeight();

      std::cout << "Begin Index: " << begin << ", End: " << end << '\n';

      auto job = [&scene, &img, begin, end] {
        for (int y = begin; y < end; ++y) {
          for (int x = 0; x < scene.getWidth(); ++x) {
            auto &screenPoint = scene.getScreenPoints()[y][x];
            Ray ray(scene.getCameraPos(), screenPoint);
            int targetSphere = scene.findClosestInter(ray);

            Color finalcolor = Color{255, 255, 255};
            if (targetSphere == -1)
              continue;  // keep background color

            const Sphere &obj = scene.getObject(targetSphere);
            // pixel prend la couleur de l'objet
            finalcolor = scene.computeColor(obj, ray);
            // mettre a jour la couleur du pixel dans l'image finale.
            img.pixel(x, y) = finalcolor;
          }
        }
      };
      slaves.emplace_back(job);
    }

    for (auto &slave : slaves)
      slave.join();
  }
};

}  // namespace pr
