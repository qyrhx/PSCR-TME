#pragma once

#include "Scene.h"
#include "Image.h"
#include "./Job.h"

namespace pr {

class PixelJob : public Job {
  const Scene &scene;
  Image &img;
  int x, y;

 public:
  PixelJob(const Scene &scene_, Image &img_, int x_, int y_)
      : scene{scene_}, img{img_}, x{x_}, y{y_} {}

  void operator()() {
    auto &screenPoint = scene.getScreenPoints()[y][x];

    Ray ray(scene.getCameraPos(), screenPoint);
    int targetSphere = scene.findClosestInter(ray);
    if (targetSphere == -1)
      return;  // keep background color
    const Sphere &obj = scene.getObject(targetSphere);
    // pixel prend la couleur de l'objet
    Color finalcolor = scene.computeColor(obj, ray);
    // mettre a jour la couleur du pixel dans l'image finale.
    img.pixel(x, y) = finalcolor;
  }
};

}  // namespace pr
