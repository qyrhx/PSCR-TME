#include "./Job.h"
#include "./Scene.h"
#include "./Image.h"

namespace pr {

class LineJob : public Job {
  const Scene &scene;
  Image &img;
  int x;

 public:
  LineJob(const Scene &scene_, Image &img_, int x_) : scene{scene_}, img{img_}, x{x_} {}

  void operator()() {
    for (int y = 0; y < scene.getWidth(); ++y) {
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

}  // namespace pr
